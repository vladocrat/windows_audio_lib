#pragma once


namespace slk
{
namespace dsp
{
namespace literals
{

constexpr float operator""_Hz(size_t freq) {
    return static_cast<float>(freq);
}

constexpr float operator""_Hz(long double freq) {
    return static_cast<float>(freq);
}

constexpr float operator""_kHz(size_t freq) {
    return static_cast<float>(freq * 1000.0);
}

constexpr float operator""_kHz(long double freq) {
    return static_cast<float>(freq * 1000.0);
}

constexpr float operator""_MHz(size_t freq) {
    return static_cast<float>(freq * 1000000.0);
}

constexpr float operator""_MHz(long double freq) {
    return static_cast<float>(freq * 1000000.0);
}

constexpr float operator""_sec(size_t time) {
    return static_cast<float>(time);
}

constexpr float operator""_sec(long double time) {
    return static_cast<float>(time);
}

constexpr float operator""_ms(size_t time) {
    return static_cast<float>(time / 1000.0);
}

constexpr float operator""_ms(long double time) {
    return static_cast<float>(time / 1000.0);
}
}
}
}
