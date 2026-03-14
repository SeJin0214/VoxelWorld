#pragma once
#include <SimpleMath.h>
#include <cstdint>

using namespace DirectX::SimpleMath;

struct IVector3 
{
    int32_t x;
    int32_t y;
    int32_t z;

    // static_assert 사용하기 위함
    constexpr IVector3(const int32_t _x = 0, const int32_t _y = 0, const int32_t _z = 0) : x(_x), y(_y), z(_z) {}
    IVector3(const float _x, const float _y, const float _z)
        : x(static_cast<int32_t>(_x))
        , y(static_cast<int32_t>(_y))
        , z(static_cast<int32_t>(_z))
    {

    }
    IVector3(const Vector3 v)
        : x(static_cast<int32_t>(v.x))
        , y(static_cast<int32_t>(v.y))
        , z(static_cast<int32_t>(v.z))
    {

    }




    // 맵의 Key로 쓰기 위한 비교 연산자
    
    bool operator!=(const IVector3& other) const { return x != other.x || y != other.y || z != other.z; }
    constexpr bool operator==(const IVector3& other) const { return x == other.x && y == other.y && z == other.z; }


    IVector3 operator+(const IVector3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    IVector3 operator-(const IVector3& v) const { return { x - v.x, y - v.y, z - v.z }; }
};