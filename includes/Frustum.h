#pragma once
#include "Types.h"

using Plane = Vector4;

enum class PlaneDir
{
    Left,
    Right,
    Bottom,
    Top,
    Near,
    Far,
    Size
};

struct AABB
{
    Vector3 Min;
    Vector3 Max;
};

class Frustum
{
public:
    Frustum(const Matrix& viewProjM)
    {
        const Matrix m = glm::transpose(viewProjM);
        mPlanes[static_cast<uint32_t>(PlaneDir::Left)] = NormalizePlane(m[3] + m[0]);
        mPlanes[static_cast<uint32_t>(PlaneDir::Right)] = NormalizePlane(m[3] - m[0]);
        mPlanes[static_cast<uint32_t>(PlaneDir::Bottom)] = NormalizePlane(m[3] + m[1]);
        mPlanes[static_cast<uint32_t>(PlaneDir::Top)] = NormalizePlane(m[3] - m[1]);
        mPlanes[static_cast<uint32_t>(PlaneDir::Near)] = NormalizePlane(m[3] + m[2]);
        mPlanes[static_cast<uint32_t>(PlaneDir::Far)] = NormalizePlane(m[3] - m[2]);
    }
    bool IntersectsFrustum(const AABB& box);

private:
    static constexpr uint32_t PLANE_COUNT = static_cast<uint32_t>(PlaneDir::Size);
    Plane mPlanes[PLANE_COUNT];
    static Plane NormalizePlane(const Plane& plane)
    {
        const float normalLength = glm::length(Vector3(plane));
        // 길이는 성분 3개만 하기
        return plane / normalLength;
    }
    bool IsOutsidePlane(const Vector4& plane, const AABB& box);
};
