#pragma once
#include <cmath>

// ─────────────────────────────────────────────
// Vec2 — 2D vector
// ─────────────────────────────────────────────
struct Vec2 {
    float x = 0, y = 0;
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s)       const { return {x * s,   y * s};   }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    float length() const { return std::sqrt(x * x + y * y); }
    Vec2  normalized() const {
        float l = length();
        return l > 0.0001f ? Vec2{x / l, y / l} : Vec2{0, 0};
    }
};

// ─────────────────────────────────────────────
// AABB — Axis-Aligned Bounding Box
// ─────────────────────────────────────────────
struct AABB {
    float x, y, w, h;
    bool overlaps(const AABB& o) const {
        return x < o.x + o.w && x + w > o.x &&
               y < o.y + o.h && y + h > o.y;
    }
};

// ─────────────────────────────────────────────
// PhysicsBody
// ─────────────────────────────────────────────
struct PhysicsBody {
    Vec2  pos;
    Vec2  vel;
    Vec2  acc;
    float mass = 1.0f;

    void integrate(float dt) {
        vel += acc * dt;
        pos += vel * dt;
        acc  = {0, 0}; // reset acceleration each frame
    }

    void applyForce(Vec2 force) {
        acc += force * (1.0f / mass);
    }

    AABB bounds(float w, float h) const {
        return {pos.x - w * 0.5f, pos.y - h * 0.5f, w, h};
    }
};

// ─────────────────────────────────────────────
// Utilities
// ─────────────────────────────────────────────
inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

inline float distSq(float ax, float ay, float bx, float by) {
    float dx = ax - bx, dy = ay - by;
    return dx * dx + dy * dy;
}
