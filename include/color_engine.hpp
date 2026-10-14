#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace techni {
enum class Style { Off, WarmCold, PureRandom, Gradient };
enum class Grouping { Standard, IsolatedGroup, Mayhem };
struct Color { float r, g, b, a = 1; };
inline constexpr std::array<Color, 4> warmPalette{{{1,0,0}, {1,0,1}, {1,.6f,0}, {1,0,.4f}}};
inline constexpr std::array<Color, 4> coldPalette{{{0,.501f,1}, {0,1,0}, {0,0,1}, {0,1,.8f}}};
inline float repeat(float t, float length = 1) { return t - std::floor(t / length) * length; }
inline Color lerp(Color a, Color b, float t) {
    t = std::clamp(t, 0.f, 1.f);
    return {a.r+(b.r-a.r)*t, a.g+(b.g-a.g)*t, a.b+(b.b-a.b)*t, a.a+(b.a-a.a)*t};
}
template <size_t N> Color palette(const std::array<Color,N>& colors, float time) {
    const float t = repeat(time, static_cast<float>(N));
    const auto i = static_cast<size_t>(t);
    return lerp(colors[i], colors[(i+1)%N], repeat(t));
}
inline Color hsv(float hue) {
    float h = repeat(hue)*6, x = 1-std::abs(std::fmod(h,2.f)-1);
    switch (static_cast<int>(h)) {
        case 0: return {1,x,0}; case 1: return {x,1,0}; case 2: return {0,1,x};
        case 3: return {0,x,1}; case 4: return {x,0,1}; default: return {1,0,x};
    }
}
// .NET Framework System.Random, as used by the original PC mod.
class Random {
    std::array<int32_t,56> seed{};
    int next = 0, nextp = 21;
public:
    explicit Random(int32_t value = 400) {
        int32_t mj = 161803398-std::abs(value), mk = 1;
        seed[55] = mj;
        for (int i=1; i<55; ++i) {
            int ii = (21*i)%55; seed[ii]=mk; mk=mj-mk;
            if (mk<0) mk+=2147483647; mj=seed[ii];
        }
        for (int k=0; k<4; ++k) for (int i=1; i<56; ++i) {
            seed[i]-=seed[1+(i+30)%55]; if(seed[i]<0) seed[i]+=2147483647;
        }
    }
    double sample() {
        if (++next>=56) next=1; if (++nextp>=56) nextp=1;
        int32_t result=seed[next]-seed[nextp];
        if(result==2147483647) --result; if(result<0) result+=2147483647;
        seed[next]=result; return result*(1.0/2147483647);
    }
};
struct Options {
    bool enabled=true, desync=false, disableBackground=false, useLeftNotes=false, useLeftSaber=false;
    Style lights=Style::Gradient, notes=Style::Off, walls=Style::Gradient, bombs=Style::PureRandom,
          sabers=Style::Off, leftNotes=Style::Off, leftSaber=Style::Off;
    Grouping grouping=Grouping::Mayhem;
    float frequency=.1f, boost=0;
    float leftOffset() const { return desync ? 0.f : .5f; } // Preserve the PC setting's actual behavior.
    Style noteStyle(bool left) const { return left && useLeftNotes ? leftNotes : notes; }
    Style saberStyle(bool left) const { return left && useLeftSaber ? leftSaber : sabers; }
};
inline Style validStyle(int v) { return v>=0 && v<=3 ? static_cast<Style>(v) : Style::Off; }
inline float finiteClamp(float v, float lo, float hi, float fallback) { return std::isfinite(v) ? std::clamp(v,lo,hi) : fallback; }
inline Color flat(bool warm, Style style, Random& rng) {
    if (style==Style::PureRandom) return hsv(static_cast<float>(rng.sample()));
    // PC GetRandomFromArray passes NextDouble directly to interpolation. Preserve this,
    // including its interpolation between the first two entries rather than discrete selection.
    return palette(warm ? warmPalette : coldPalette, static_cast<float>(rng.sample()));
}
inline Color gradient(const Options& o, float time, bool left, float seed=0) {
    return hsv(time*o.frequency+(left ? o.leftOffset() : 0)+seed);
}
inline Color globalGradient(const Options& o, float time) { return hsv(time*(o.frequency/2+.7f)); }
inline Color lightColor(Color source, Color hue, float boost) {
    const float intensity=std::max({source.r,source.g,source.b,0.f});
    return {hue.r*intensity,hue.g*intensity,hue.b*intensity,source.a*(1+boost)};
}
}
