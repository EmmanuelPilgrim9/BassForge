// BassForge — motor de realce psicoacústico de graves.
// Sin dependencias de JUCE: se puede probar y reutilizar por separado.
//
// Flujo por canal:
//   x ─┬─ LR4 pasa-bajos ─ low ─┬─ envolvente ─┐
//      │                        └─ normalizar ─┴─ Chebyshev (armónicos) ─ ×env ─ HP ─ LP ─ harm
//      └─ LR4 pasa-altos ─ high
//   salida = (high + fundamental·low + intensidad·harm) · ganancia
#pragma once
#include <cmath>
#include <algorithm>

namespace bassforge {

constexpr double kPi = 3.14159265358979323846;

// Biquad RBJ en forma directa transpuesta II.
struct Biquad {
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    double z1 = 0, z2 = 0;

    void reset() { z1 = z2 = 0; }

    void setLowpass(double fs, double fc, double q) { design(fs, fc, q, true); }
    void setHighpass(double fs, double fc, double q) { design(fs, fc, q, false); }

    inline double process(double x) {
        const double y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }

private:
    void design(double fs, double fc, double q, bool lowpass) {
        fc = std::clamp(fc, 5.0, fs * 0.45);
        const double w0 = 2.0 * kPi * fc / fs;
        const double cw = std::cos(w0), alpha = std::sin(w0) / (2.0 * q);
        const double a0 = 1.0 + alpha;
        if (lowpass) {
            b0 = (1.0 - cw) * 0.5; b1 = 1.0 - cw; b2 = b0;
        } else {
            b0 = (1.0 + cw) * 0.5; b1 = -(1.0 + cw); b2 = b0;
        }
        a1 = -2.0 * cw; a2 = 1.0 - alpha;
        b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;
    }
};

// Linkwitz-Riley de 4º orden = dos Butterworth de 2º orden (Q = 0.7071) en cascada.
// LP + HP suman a un pasa-todo: no hay hueco ni joroba en la frecuencia de cruce.
struct LR4 {
    Biquad s1, s2;
    void reset() { s1.reset(); s2.reset(); }
    void set(double fs, double fc, bool lowpass) {
        constexpr double q = 0.70710678118654752;
        if (lowpass) { s1.setLowpass(fs, fc, q); s2.setLowpass(fs, fc, q); }
        else         { s1.setHighpass(fs, fc, q); s2.setHighpass(fs, fc, q); }
    }
    inline double process(double x) { return s2.process(s1.process(x)); }
};

// Seguidor de envolvente de picos con ataque y relajación.
struct EnvelopeFollower {
    double env = 0, att = 0, rel = 0;
    void reset() { env = 0; }
    void set(double fs, double attackMs, double releaseMs) {
        att = std::exp(-1.0 / (fs * attackMs * 0.001));
        rel = std::exp(-1.0 / (fs * releaseMs * 0.001));
    }
    inline double process(double x) {
        const double a = std::fabs(x);
        const double c = a > env ? att : rel;
        env = a + c * (env - a);
        return env;
    }
};

struct Params {
    double frequency   = 70.0;  // Hz, corte de la banda grave (30–200)
    double intensity   = 0.5;   // 0–1, cantidad de armónicos
    double character   = 0.5;   // 0 = pares (cálido) … 1 = impares (agresivo)
    double fundamental = 1.0;   // 0–1, nivel de la fundamental original
    double outputDb    = 0.0;   // dB de salida
    bool   soloHarmonics = false;
};

class ChannelEngine {
public:
    void prepare(double sampleRate) {
        fs = sampleRate;
        // Ataque rápido (para que el transiente del kick tenga armónicos desde el inicio)
        // y relajación de ~1 periodo largo, para no modular la forma de onda.
        env.set(fs, 1.0, 40.0);
        lastFreq = -1.0;
        reset();
    }

    void reset() {
        lp.reset(); hp.reset(); env.reset();
        harmHp.reset(); harmLp1.reset(); harmLp2.reset();
    }

    void updateFilters(double freq) {
        if (freq == lastFreq) return;
        lastFreq = freq;
        lp.set(fs, freq, true);
        hp.set(fs, freq, false);
        // Los armónicos útiles viven entre ~1·fc y ~6·fc.
        harmHp.setHighpass(fs, freq * 1.1, 0.7071);
        harmLp1.setLowpass(fs, std::min(freq * 6.0, fs * 0.45), 0.7071);
        harmLp2.setLowpass(fs, std::min(freq * 6.0, fs * 0.45), 0.7071);
    }

    // character, intensity, fundamental y gain ya vienen suavizados desde fuera.
    inline double process(double x, double character, double intensity,
                          double fundamental, double gain, bool solo) {
        const double low  = lp.process(x);
        const double high = hp.process(x);

        // 1) Normalizar por la envolvente: el waveshaper siempre recibe ~±1,
        //    así el contenido armónico no depende del volumen.
        const double e = env.process(low);
        double n = low / (e + 1e-9);
        n = std::clamp(n, -1.0, 1.0);

        // 2) Polinomios de Chebyshev: con un seno de amplitud 1, T_k(x) da exactamente
        //    el armónico k. Pares (2,4) = calidez; impares (3,5) = mordida.
        const double n2 = n * n;
        const double t2 = 2.0 * n2 - 1.0;
        const double t3 = n * (4.0 * n2 - 3.0);
        const double t4 = 8.0 * n2 * n2 - 8.0 * n2 + 1.0;
        const double t5 = n * (16.0 * n2 * n2 - 20.0 * n2 + 5.0);
        const double even = 0.75 * t2 + 0.25 * t4;
        const double odd  = 0.75 * t3 + 0.25 * t5;
        double h = (1.0 - character) * even + character * odd;

        // 3) Devolver la dinámica original. Una compuerta suave en niveles muy bajos
        //    evita que el ruido de fondo se convierta en armónicos.
        const double gate = e / (e + 1e-4);
        h *= e * gate;

        // 4) Quitar DC y lo que cae bajo el corte; quitar armónicos altos ásperos.
        h = harmHp.process(h);
        h = harmLp2.process(harmLp1.process(h));

        const double harm = intensity * kHarmonicMakeup * h;
        const double dry  = solo ? 0.0 : (high + fundamental * low);
        return (dry + harm) * gain;
    }

private:
    static constexpr double kHarmonicMakeup = 2.0; // intensidad 100% ≈ +6 dB de armónicos
    double fs = 44100.0, lastFreq = -1.0;
    LR4 lp, hp;
    EnvelopeFollower env;
    Biquad harmHp, harmLp1, harmLp2;
};

// Suavizado lineal de parámetros para evitar clicks al mover knobs.
struct Smoothed {
    double current = 0, target = 0, step = 0;
    int remaining = 0, length = 1;
    void prepare(double fs, double seconds, double value) {
        length = std::max(1, (int)(fs * seconds));
        current = target = value; remaining = 0;
    }
    void setTarget(double v) {
        if (v == target) return;
        target = v; remaining = length;
        step = (target - current) / length;
    }
    inline double next() {
        if (remaining > 0) { current += step; --remaining; if (remaining == 0) current = target; }
        return current;
    }
};

inline double dbToGain(double db) { return std::pow(10.0, db / 20.0); }

} // namespace bassforge
