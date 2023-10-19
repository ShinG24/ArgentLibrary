#include "Common.hlsli"

void Swap(inout float a, inout float b)
{
    float tmp = a;
    a = b;
    b = tmp;
}

bool IsInRange(in float val, in float min, in float max)
{
    return (val >= min && val <= max);
}

bool IsCulled(in Ray ray, in float3 hit_surface_normal)
{
    float ray_direction_normal_dot = dot(ray.direction_, hit_surface_normal);
    bool is_culled =
		((RayFlags() & RAY_FLAG_CULL_BACK_FACING_TRIANGLES) && (ray_direction_normal_dot > 0))
		||
		((RayFlags() & RAY_FLAG_CULL_FRONT_FACING_TRIANGLES) && (ray_direction_normal_dot < 0));

    return is_culled;
}

bool IsAValidHit(in Ray ray, in float t_hit, in float3 hit_surface_normal)
{
    return IsInRange(t_hit, RayTMin(), RayTCurrent()) && !IsCulled(ray, hit_surface_normal);
}

float3 CalculateNormalForAraySphereHit(in Ray ray, in float t_hit, float3 center)
{
    float3 hit_position = ray.origin_ + t_hit * ray.direction_;
    return normalize(hit_position - center);
}

bool SolveQuadraticEqn(float a, float b, float c, out float x0, out float x1)
{
    float d = b * b - 4 * a * c;
    if (d < 0)
        return false;
    else if (d == 0)
        x0 = x1 = -0.5 * b / a;
    else
    {
        float q = (b > 0) ?
			-0.5 * (b + sqrt(d)) :
			-0.5 * (b - sqrt(d));
        x0 = q / a;
        x1 = c / q;
    }

    if (x0 > x1)
        Swap(x0, x1);

    return true;
}

bool SolveRaySphereIntersectionEquation(in Ray ray, out float t_min, out  float t_max, in float3 center, in float radius)
{
    float3 L = ray.origin_ - center;
    float a = dot(ray.direction_, ray.direction_);
    float b = 2 * dot(ray.direction_, L);
    float c = dot(L, L) - radius * radius;
    return SolveQuadraticEqn(a, b, c, t_min, t_max);
}


bool IntersectRaySphere(in Ray ray, out float t_hit, out float t_max, out SphereHitAttribute attr,
						in float3 center, in float radius)
{
    float t0, t1;

    //Calculate the Intersection point
    if (SolveRaySphereIntersectionEquation(ray, t0, t1, center, radius))
        return false;

    t_max = t1;

    if(t0 < RayTMin())
    {
        if (t1 < RayTMin())
            return false;

        attr.normal_ = CalculateNormalForAraySphereHit(ray, t1, center);
        if(IsAValidHit(ray, t1, attr.normal_))
        {
            t_hit = t1;
            return true;
        }
    }
    else
    {
        attr.normal_ = CalculateNormalForAraySphereHit(ray, t0, center);
        if(IsAValidHit(ray, t0, attr.normal_))
        {
            t_hit = t0;
            return true;
        }

        attr.normal_ = CalculateNormalForAraySphereHit(ray, t1, center);
        if(IsAValidHit(ray, t1, attr.normal_))
        {
            t_hit = t1;
            return true;
        }
    }
    return false;
}

bool IntersectRaySpheres(in Ray ray, out float t_hit, out SphereHitAttribute attr)
{
    float3 center = float3(0, 0, 0);
    float radius = 3.0f;
    bool hit_found = false;
    t_hit = RayTCurrent();

    float _t_hit;
    float _t_max;
    SphereHitAttribute _attr = (SphereHitAttribute)0;

    if(IntersectRaySphere(ray, _t_hit, _t_max, _attr, center, radius))
    {
	    if(_t_hit < t_hit)
	    {
            t_hit = _t_hit;
            attr = _attr;
            hit_found = true;
        }
    }
    return hit_found;
}

[shader("intersection")]
void SphereIntersection()
{
    Ray ray;
    ray.origin_ = ObjectRayOrigin();
    ray.direction_ = ObjectRayDirection();
    float t_hit;

    SphereHitAttribute attr = (SphereHitAttribute)0;

    if(IntersectRaySpheres(ray, t_hit, attr))
    {
        ReportHit(t_hit, 0, attr);
    }
}