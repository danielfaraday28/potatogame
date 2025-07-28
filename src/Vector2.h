#pragma once

struct Vector2 {
    float x, y;
    
    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}
    
    Vector2 operator+(const Vector2& other) const;
    Vector2 operator-(const Vector2& other) const;
    Vector2 operator*(float scalar) const;
    Vector2& operator+=(const Vector2& other);
    
    float length() const;
    Vector2 normalized() const;
    float distance(const Vector2& other) const;
};