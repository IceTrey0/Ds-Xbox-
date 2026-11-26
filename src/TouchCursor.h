#pragma once
// Right-stick cursor for the DS bottom screen (256x192).
// Header-only. Input: stick values in -1..1 with +Y = up
// (Windows.Gaming.Input RightThumbstickY is already +up).
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace mxb {

struct TouchOutput {
    bool down;      // stylus touching the screen
    uint16_t x, y;  // 0..255, 0..191 (valid when down)
};

class TouchCursor {
public:
    float speed = 200.0f;    // pixels/sec at full deflection
    float slowMult = 0.35f;  // multiplier while the precision button is held
    float deadzone = 0.18f;  // radial deadzone
    float curve = 1.6f;      // >1 = finer control near center

    // Call once per frame. dt in seconds.
    void Update(float sx, float sy, float dt, bool slow = false) {
        float mag = std::sqrt(sx * sx + sy * sy);
        if (mag > deadzone) {
            float t = std::min(1.0f, (mag - deadzone) / (1.0f - deadzone));
            t = std::pow(t, curve);
            float step = t * speed * dt * (slow ? slowMult : 1.0f);
            x_ += (sx / mag) * step;
            y_ -= (sy / mag) * step;  // screen Y grows downward
        }
        x_ = std::clamp(x_, 0.0f, 255.0f);
        y_ = std::clamp(y_, 0.0f, 191.0f);
    }

    TouchOutput Output(bool pressed) const {
        return {pressed, uint16_t(x_), uint16_t(y_)};
    }

    // For drawing the crosshair on the bottom screen.
    float X() const { return x_; }
    float Y() const { return y_; }

private:
    float x_ = 128.0f, y_ = 96.0f;
};

} // namespace mxb
