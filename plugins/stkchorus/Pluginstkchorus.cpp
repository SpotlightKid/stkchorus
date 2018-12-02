/*
 * STK Chorus audio efffect based on DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2018 Christopher Arndt <info@chrisarndt.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <math.h>

#include "Pluginstkchorus.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

Pluginstkchorus::Pluginstkchorus()
    : Plugin(paramCount, 1, 0),  // paramCount params, 1 program(s), 0 states
    fSampleRate(getSampleRate())
{
    lfo = new LFO(fSampleRate);
    lpfL = new LowPassFilter(fSampleRate);
    lpfR = new LowPassFilter(fSampleRate);
    cutoff = new CParamSmooth(PARAM_SMOOTH_TIME, getSampleRate());
    loadProgram(0);
}

// -----------------------------------------------------------------------
// Init

void Pluginstkchorus::initParameter(uint32_t index, Parameter& parameter) {
    if (index >= paramCount)
        return;

    parameter.ranges.min = 0.0f;
    parameter.ranges.max = 1.0f;
    parameter.ranges.def = 0.1f;
    parameter.hints = kParameterIsAutomable;

    switch (index) {
        case paramCutoff:
            parameter.name = "Cutoff";
            parameter.symbol = "cutoff";
            parameter.unit = "Hz";
            parameter.ranges.min = 20.0f;
            parameter.ranges.max = 20000.0f;
            parameter.ranges.def = 15000.0f;
            parameter.hints |= kParameterIsInteger | kParameterIsLogarithmic;
            break;
        case paramResonance:
            parameter.name = "Resonance";
            parameter.symbol = "resonance";
            parameter.ranges.max = 1.0f;
            parameter.ranges.def = 0.0f;
            break;
        case paramLFOWaveform:
            parameter.name = "LFO Waveform";
            parameter.symbol = "lfowaveform";
            parameter.hints |= kParameterIsInteger;
            parameter.enumValues.count = 5;
            parameter.enumValues.restrictedMode = true;
            {
                ParameterEnumerationValue* const values = new ParameterEnumerationValue[5];
                parameter.enumValues.values = values;
                values[0].label = "Triangle";
                values[0].value = LFO::triangle;
                values[1].label = "Sine";
                values[1].value = LFO::sinus;
                values[2].label = "Sawtooth";
                values[2].value = LFO::sawtooth;
                values[3].label = "Square";
                values[3].value = LFO::square;
                values[4].label = "Exponential";
                values[4].value = LFO::exponent;
            }
            break;
        case paramLFOFrequency:
            parameter.name = "LFO Frequency";
            parameter.symbol = "lfofreq";
            parameter.unit = "Hz";
            parameter.ranges.min = 0.01f;
            parameter.ranges.max = 25.0f;
            parameter.ranges.def = 1.0f;
            parameter.hints |= kParameterIsLogarithmic;
            break;
        case paramLFOModDepth:
            parameter.name = "LFO Mod Depth";
            parameter.symbol = "lfomoddepth";
            parameter.unit = "ct";
            parameter.ranges.min = -10800.0f;
            parameter.ranges.max = 10800.0f;
            parameter.ranges.def = 0.0f;
            parameter.hints |= kParameterIsInteger;
            break;
    }
}

/**
  Set the name of the program @a index.
  This function will be called once, shortly after the plugin is created.
*/
void Pluginstkchorus::initProgramName(uint32_t index, String& programName) {
    switch (index) {
        case 0:
            programName = "Default";
            break;
    }
}

// -----------------------------------------------------------------------
// Internal data

/**
  Optional callback to inform the plugin about a sample rate change.
*/
void Pluginstkchorus::sampleRateChanged(double fs) {
    if (fSampleRate != fs) {
        fSampleRate = fs;
        lfo->setSampleRate(fs);
        lfo->setRate(fParams[paramLFOFrequency]);
        lpfL->setSampleRate(fs);
        lpfR->setSampleRate(fs);
        cutoff->setSampleRate(fs);
    }
}

/**
  Get the current value of a parameter.
*/
float Pluginstkchorus::getParameterValue(uint32_t index) const {
    return fParams[index];
}

/**
  Change a parameter value.
*/
void Pluginstkchorus::setParameterValue(uint32_t index, float value) {
    fParams[index] = value;

    switch (index) {
        case paramCutoff:
            lpfL->setCutoff(value);
            lpfR->setCutoff(value);
            break;
        case paramResonance:
            lpfL->setResonance(value);
            lpfR->setResonance(value);
            break;
        case paramLFOWaveform:
            lfo->setWaveform((LFO::waveform_t)value);
            break;
        case paramLFOFrequency:
            lfo->setRate(value);
            break;
    }
}

/**
  Load a program.
  The host may call this function from any context,
  including realtime processing.
*/
void Pluginstkchorus::loadProgram(uint32_t index) {
    switch (index) {
        case 0:
            setParameterValue(paramCutoff, 15000.0f);
            setParameterValue(paramResonance, 0.0f);
            setParameterValue(paramLFOWaveform, 0.0f);
            setParameterValue(paramLFOFrequency, 1.0f);
            setParameterValue(paramLFOModDepth, 0.0f);
            break;
    }
}

// -----------------------------------------------------------------------
// Process

void Pluginstkchorus::activate() {
    sampleRateChanged(getSampleRate());
}

void Pluginstkchorus::run(const AmpVal** inputs, AmpVal** outputs, uint32_t frames) {
    float cfreq, lfoval;

    // get the left and right audio inputs
    const AmpVal* const inpL = inputs[0];
    const AmpVal* const inpR = inputs[1];

    // get the left and right audio outputs
    AmpVal* const outL = outputs[0];
    AmpVal* const outR = outputs[1];

    float lfomod = fParams[paramLFOModDepth];

    // apply gain against all samples
    for (uint32_t i=0; i < frames; ++i) {
        lfoval = lfo->tick();
        if (lfomod != 0.0f) {
            cfreq = cutoff->process(fParams[paramCutoff]) * pow(2, lfoval * (lfomod / 1200.0));
            cfreq = cfreq > 20.0 ? cfreq : 20.0;
            cfreq = cfreq < 20000.0 ? cfreq : 20000.0;
            lpfL->setCutoff(cfreq);
            lpfR->setCutoff(cfreq);
        }
        outL[i] = lpfL->process(inpL[i]);
        outR[i] = lpfR->process(inpR[i]);
    }
}

// -----------------------------------------------------------------------

Plugin* createPlugin() {
    return new Pluginstkchorus();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
