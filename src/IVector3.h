#pragma once
#include <SimpleMath.h>
#include <cstdint>

using namespace DirectX::SimpleMath;

struct IVector3 
{
    int32_t x;
    int32_t y;
    int32_t z;

    // 생성자
    IVector3() : x(0), y(0), z(0) {}
    IVector3(const int32_t _x, const int32_t _y, const int32_t _z) : x(_x), y(_y), z(_z) {}
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
    bool operator==(const IVector3& other) const { return x == other.x && y == other.y && z == other.z; }


    IVector3 operator+(const IVector3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    IVector3 operator-(const IVector3& v) const { return { x - v.x, y - v.y, z - v.z }; }
};