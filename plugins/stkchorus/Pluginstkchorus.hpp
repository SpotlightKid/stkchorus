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

#ifndef PLUGIN_STKCHORUS_H
#define PLUGIN_STKCHORUS_H

#include "DistrhoPlugin.hpp"
#include "LFO.hpp"
#include "LPF.hpp"
#include "CParamSmooth.hpp"


#define PARAM_SMOOTH_TIME 1000
typedef float AmpVal;

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class Pluginstkchorus : public Plugin {
public:
    enum Parameters {
        paramCutoff = 0,
        paramResonance,
        paramLFOWaveform,
        paramLFOFrequency,
        paramLFOModDepth,
        paramCount
    };

    Pluginstkchorus();

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getLabel() const noexcept override {
        return "stkchorus";
    }

    const char* getDescription() const override {
        return "A chorus audio effect plugin based on STK";
    }

    const char* getMaker() const noexcept override {
        return "chrisarndt.de";
    }

    const char* getHomePage() const override {
        return "https://chrisarndt.de/plugins/stkchorus";
    }

    const char* getLicense() const noexcept override {
        return "MIT";
    }

    uint32_t getVersion() const noexcept override {
        return d_version(0, 1, 0);
    }

    // Go to:
    //
    // http://service.steinberg.de/databases/plugin.nsf/plugIn
    //
    // Get a proper plugin UID and fill it in here!
    int64_t getUniqueId() const noexcept override {
        return d_cconst('s', 't', 'k', 'c');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(uint32_t index, Parameter& parameter) override;
    void initProgramName(uint32_t index, String& programName) override;

    // -------------------------------------------------------------------
    // Internal data

    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;
    void loadProgram(uint32_t index) override;

    // -------------------------------------------------------------------
    // Optional

    // Optional callback to inform the plugin about a sample rate change.
    void sampleRateChanged(double fs) override;

    // -------------------------------------------------------------------
    // Process

    void activate() override;
    void run(const AmpVal**, AmpVal** outputs, uint32_t frames) override;

    // -------------------------------------------------------------------

private:
    float fParams[paramCount];
    double fSampleRate;
    LFO *lfo;
    LowPassFilter *lpfL;
    LowPassFilter *lpfR;
    CParamSmooth* cutoff;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Pluginstkchorus)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif  // #ifndef PLUGIN_STKCHORUS_H
