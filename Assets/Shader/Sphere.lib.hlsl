#include "Common.hlsli"

ConstantBuffer<ObjectConstant> object_constant : register(b0, space1);
ConstantBuffer<Material> material_constant : register(b1, space1);


_CLOSEST_HIT_SHADER_
void SphereClosestHit(inout RayPayload payload, in SphereHitAttribute attr)
{
    float4 albedo_color = material_constant.albedo_color_;
    float reflectance_coefficient = material_constant.reflectance_coefficient_;
    float diffuse_coefficient = material_constant.diffuse_coefficient_;
    float specular_coefficient = material_constant.specular_coefficient_;
    float specular_power = material_constant.specular_power_;

    //float4 albedo_color = float4(1, 1, 1, 1);

    Ray ray;
    ray.origin_ = CalcHitWorldPosition();
    ray.direction_ = CalcReflectedRayDirection(attr.normal_);
    float4 reflection_color = TraceRadianceRay(ray, payload.recursion_depth_);

    float3 fresnel_r = FresnelReflectanceSchlick(WorldRayDirection(), attr.normal_,
    albedo_color.xyz);

    reflection_color = reflectance_coefficient * float4(fresnel_r, 1) * reflection_color;

    

    float4 phong_color = CalcPhongLighting(albedo_color, attr.normal_,
    diffuse_coefficient, specular_coefficient, specular_power);
    float4 color = phong_color + reflection_color;
    payload.color_ = float4(color.rgb, 1.0f);
}




//------------------Intersection-----------------------
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

bool SolveRaySphereIntersectionEquation(in Ray ray, out float t_min, out float t_max, in float3 center, in float radius)
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
    if (!SolveRaySphereIntersectionEquation(ray, t0, t1, center, radius))
        return false;

    t_max = t1;

    if (t0 < RayTMin())
    {
        if (t1 < RayTMin())
            return false;

        attr.normal_ = CalculateNormalForAraySphereHit(ray, t1, center);
        if (IsAValidHit(ray, t1, attr.normal_))
        {
            t_hit = t1;
            return true;
        }
    }
    else
    {
        attr.normal_ = CalculateNormalForAraySphereHit(ray, t0, center);
        if (IsAValidHit(ray, t0, attr.normal_))
        {
            t_hit = t0;
            return true;
        }

        attr.normal_ = CalculateNormalForAraySphereHit(ray, t1, center);
        if (IsAValidHit(ray, t1, attr.normal_))
        {
            t_hit = t1;
            return true;
        }
    }
    return false;
}

bool IntersectRaySpheres(in Ray ray, out float t_hit, out SphereHitAttribute attr)
{
    const int N = 1;
    //The position in Local Space
    float3 centeres[N] =
    {
        float3(0, 0, 0)
       // float3(250, 0, 0),
       // float3(-250, 0, 0),
    };
    float radius[N] =
    {
        50.0f
        //200.0f,
        //200.0f
    };

    bool hit_found = false;
    t_hit = RayTCurrent();

    float _t_hit;
    float _t_max;
    SphereHitAttribute _attr = (SphereHitAttribute) 0;

    for (int i = 0; i < N; ++i)
    {
        if (IntersectRaySphere(ray, _t_hit, _t_max, _attr, centeres[i], radius[i]))
        {
            if (_t_hit < t_hit)
            {
                t_hit = _t_hit;
                attr = _attr;
                hit_found = true;
            }
        }
    }
    return hit_found;
}

_INTERSECTION_SHADER_
void SphereIntersection()
{
    Ray ray;
    ray.origin_ = mul(float4(WorldRayOrigin(), 1), object_constant.inv_world_).xyz;
    ray.direction_ = mul(float4(WorldRayDirection(), 0), object_constant.inv_world_).xyz;
    //ray.origin_ = mul(float4(ObjectRayOrigin(), 1), object_constant.inv_world_);
    //ray.direction_ = mul(ObjectRayDirection(), (float3x3)object_constant.inv_world_);
    float t_hit;

    SphereHitAttribute attr = (SphereHitAttribute) 0;

    if (IntersectRaySpheres(ray, t_hit, attr))
    {
        attr.normal_ = mul(attr.normal_, (float3x3) object_constant.world_);
        attr.normal_ = normalize(attr.normal_);
        //attr.normal_ = normalize(mul((float3x3) ObjectToWorld3x4(), attr.normal_));

        ReportHit(t_hit, 1, attr);
    }
}
