/**
 * By alexirae@gmail.com
 *
 * http://www.musicdsp.org/showone.php?id=257
 *
 * This is a very simple class that I'm using in my plugins for smoothing
 * parameter changes that directly affect audio stream.
 * It's a 1-pole LPF, very easy on CPU.
 * You can specify the speed response of the parameter in ms.
 * and sampling rate.
 *
 */
class CParamSmooth {
public:
    CParamSmooth(float ms, float fs) {
        smoothingTimeMs = ms;
        setSampleRate(fs);
    }

    ~CParamSmooth() {
    }

    /*
     * Set the sample rate
     * @param fs new sample rate in Hz
     */
    inline void setSampleRate(float fs) {
        if (sampleRate != fs) {
            sampleRate = (fs > 0.0) ? fs : 44100.0f;
            init();
        }
    }

    inline void init() {
        const float c_twoPi = 6.283185307179586476925286766559f;

        a = exp(-c_twoPi / (smoothingTimeMs * 0.001f * sampleRate));
        b = 1.0f - a;
        z = 0.0f;
    }

    inline float process(float in) {
        return z = (in * b) + (z * a);
    }

private:
    float sampleRate;
    float smoothingTimeMs;
    float a;
    float b;
    float z;
};
