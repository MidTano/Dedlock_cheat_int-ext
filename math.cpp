#pragma once
#include "math.h"
#include "memory.h"
#include <windows.h>


Vector2::Vector2() : X(0), Y(0) {}
Vector2::Vector2(float x, float y) : X(x), Y(y) {}

Vector2 Vector2::operator+(const Vector2& other) const {
    return Vector2(X + other.X, Y + other.Y);
}

Vector2 Vector2::operator-(const Vector2& other) const {
    return Vector2(X - other.X, Y - other.Y);
}

Vector2& Vector2::operator+=(const Vector2& other) {
    X += other.X;
    Y += other.Y;
    return *this;
}

Vector2& Vector2::operator-=(const Vector2& other) {
    X -= other.X;
    Y -= other.Y;
    return *this;
}

Vector2 Vector2::operator*(float scalar) const {
    return Vector2(X * scalar, Y * scalar);
}

Vector2 Vector2::operator/(float scalar) const {
    return Vector2(X / scalar, Y / scalar);
}

Vector3::Vector3() : X(0), Y(0), Z(0) {}
Vector3::Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}

Vector3 Vector3::operator+(const Vector3& other) const {
    return Vector3(X + other.X, Y + other.Y, Z + other.Z);
}

Vector3 Vector3::operator-(const Vector3& other) const {
    return Vector3(X - other.X, Y - other.Y, Z - other.Z);
}

Vector3& Vector3::operator+=(const Vector3& other) {
    X += other.X;
    Y += other.Y;
    Z += other.Z;
    return *this;
}

Vector3& Vector3::operator-=(const Vector3& other) {
    X -= other.X;
    Y -= other.Y;
    Z -= other.Z;
    return *this;
}

Vector3 Vector3::operator*(float scalar) const {
    return Vector3(X * scalar, Y * scalar, Z * scalar);
}

Vector3 Vector3::operator/(float scalar) const {
    return Vector3(X / scalar, Y / scalar, Z / scalar);
}


float Vector3::Length() const {
    return sqrtf(X * X + Y * Y + Z * Z);
}


float* ViewMatrix::operator[](int index) {
    return matrix[index];
}

const float* ViewMatrix::operator[](int index) const {
    return matrix[index];
}


std::string read_str(uintptr_t address) {
    std::vector<char> buffer(256);
    size_t offset = 0;
    char ch;
    while (true) {
        if (!ReadProcessMemory(memory::processHandle, reinterpret_cast<LPCVOID>(address + offset), &ch, sizeof(ch), NULL) || ch == '\0') {
            break;
        }
        if (offset >= buffer.size()) {
            buffer.resize(buffer.size() * 2);
        }
        buffer[offset++] = ch;
    }
    return std::string(buffer.data(), offset);
}


ViewMatrix get_view_matrix() {
    return memory::memRead<ViewMatrix>(memory::baseAddress + offsets::dwViewMatrix);
}


float get_distance(const Vector3& from, const Vector3& to) {
    return sqrt(pow(to.X - from.X, 2) + pow(to.Y - from.Y, 2) + pow(to.Z - from.Z, 2));
}


Vector3 WorldToScreen(const ViewMatrix& vm, const Vector3& pos) {
    float _x = vm[0][0] * pos.X + vm[0][1] * pos.Y + vm[0][2] * pos.Z + vm[0][3];
    float _y = vm[1][0] * pos.X + vm[1][1] * pos.Y + vm[1][2] * pos.Z + vm[1][3];
    float _w = vm[3][0] * pos.X + vm[3][1] * pos.Y + vm[3][2] * pos.Z + vm[3][3];

    if (_w < 0.01f) return Vector3(); 

    float inv_w = 1.0f / _w;
    _x *= inv_w;
    _y *= inv_w;

    float x = GetSystemMetrics(SM_CXSCREEN) * 0.5f + _x * GetSystemMetrics(SM_CXSCREEN) * 0.5f;
    float y = GetSystemMetrics(SM_CYSCREEN) * 0.5f - _y * GetSystemMetrics(SM_CYSCREEN) * 0.5f;

    return Vector3(x, y, _w);
}
