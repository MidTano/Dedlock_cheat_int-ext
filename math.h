#pragma once
#include <string>
#include <vector>
#include <cmath>
#include "memory.h"
#include <windows.h>

struct Vector2 {
    float X, Y;
    Vector2();
    Vector2(float x, float y);
    Vector2 operator+(const Vector2& other) const;
    Vector2 operator-(const Vector2& other) const;
    Vector2& operator+=(const Vector2& other);
    Vector2& operator-=(const Vector2& other);
    Vector2 operator*(float scalar) const;
    Vector2 operator/(float scalar) const;
};

struct Vector3 {
    float X, Y, Z;
    Vector3();
    Vector3(float x, float y, float z);
    Vector3 operator+(const Vector3& other) const;
    Vector3 operator-(const Vector3& other) const;
    Vector3& operator+=(const Vector3& other);
    Vector3& operator-=(const Vector3& other);
    Vector3 operator*(float scalar) const;
    Vector3 operator/(float scalar) const;


    float Length() const;
};

struct ViewMatrix {
    float matrix[4][4];
    float* operator[](int index);
    const float* operator[](int index) const;
};

std::string read_str(uintptr_t address);
ViewMatrix get_view_matrix();
float get_distance(const Vector3& from, const Vector3& to);
Vector3 WorldToScreen(const ViewMatrix& vm, const Vector3& pos);
