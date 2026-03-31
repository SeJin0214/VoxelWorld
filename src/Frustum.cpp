#include "Frustum.h"

bool Frustum::IntersectsFrustum(const AABB& box)
{
    for (uint32_t i = 0; i < PLANE_COUNT; ++i)
    {
        if (IsOutsidePlane(mPlanes[i], box))
        {
            return false;
        }
    }
    return true;
}

bool Frustum::IsOutsidePlane(const Vector4& plane, const AABB& box)
{
    Vector3 p;
    p.x = (plane.x >= 0.0f) ? box.Max.x : box.Min.x;
    p.y = (plane.y >= 0.0f) ? box.Max.y : box.Min.y;
    p.z = (plane.z >= 0.0f) ? box.Max.z : box.Min.z;

    return glm::dot(Vector3(plane), p) + plane.w < 0.0f;
}
