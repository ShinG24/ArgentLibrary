#ifdef __cplusplus
#define uint    UINT
#endif

struct RayPayload
{
    float4 color_;
    uint recursion_depth_;
};


#define _MAX_RECURSION_DEPTH_ 3

#ifndef __cplusplus

#define HitAttribute   BuiltInTriangleIntersectionAttributes

#define _ClosestHitID_      "closesthit"
#define _RayGenerationID_   "raygeneration"
#define _MissID_            "miss"
#define _IntersectionID_    "intersection"


#define _RAY_GENERATION_SHADER_     [shader(_RayGenerationID_)]
#define _CLOSEST_HIT_SHADER_        [shader(_ClosestHitID_)]
#define _MISS_SHADER_               [shader(_MissID_)]
#define _INTERSECTION_SHADER_       [shader(_IntersectionID_)]
	
struct ObjectConstant
{
    row_major float4x4 world_;
    row_major float4x4 inv_world_;
};

struct Material
{
    float4 albedo_color_;
    float diffuse_coefficient_;
    float specular_coefficient_;
    float reflectance_coefficient_;
    float specular_power_;
};


struct SphereHitAttribute
{
	float3 normal_;
};

struct Ray
{
    float3 origin_;
    float3 direction_;
};

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> output_texture : register(u0);

struct SceneConstant
{
    row_major float4x4 view_projection_;
    row_major float4x4 inv_view_projection_;
    float4 camera_position_;
    float4 light_position_;
};

ConstantBuffer<SceneConstant> scene_constant : register(b0);


static const uint kInstanceMask = ~0;

float3 CalcHitWorldPosition()
{
    return WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
}

float3 CalcReflectedRayDirection(in float3 normal)
{
    return reflect(WorldRayDirection(), normal);
}

float4 TraceRadianceRay(in Ray ray, in uint current_recursion_depth)
{
	if(current_recursion_depth >= _MAX_RECURSION_DEPTH_)
	{
        return float4(0, 0, 0, 0);
    }

    RayDesc ray_desc;
    ray_desc.Origin = ray.origin_;
    ray_desc.Direction = ray.direction_;
    ray_desc.TMin = 0.0f;
    ray_desc.TMax = 1000.0f;

    RayPayload payload;
	payload.color_ = float4(0, 0, 0, 0);
    payload.recursion_depth_ = current_recursion_depth + 1;

#if 1
    TraceRay(scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
    kInstanceMask, 0, 1, 0, ray_desc, payload);
#else
    TraceRay(scene, RAY_FLAG_CULL_FRONT_FACING_TRIANGLES,
    kInstanceMask, 0, 1, 0, ray_desc, payload);
#endif

    return payload.color_;
}

float CalcDiffuseCoefficient(in float3 incident_light_ray, in float3 surface_normal)
{
    return saturate(dot(-incident_light_ray, surface_normal));
}

float4 CalcSpecularCoefficient(in float3 incident_light_ray, in float3 surface_normal, in float specular_power)
{
    float3 reflected_light_ray = normalize(reflect(incident_light_ray, surface_normal));
    return pow(saturate(dot(reflected_light_ray, normalize(-WorldRayDirection()))), specular_power);
}

float4 CalcPhongLighting(in float4 albedo_color, in float3 surface_normal,
						in float diffuse_coefficient, in float specular_coefficient,
						in float specular_power)
{
    float3 hit_position = CalcHitWorldPosition();
    float3 light_position = scene_constant.light_position_;
    float3 incident_light_ray = normalize(hit_position - light_position);

   // float3 incident_light_ray = normalize(scene_constant.light_position_.xyz);

    //White Light
    float4 light_color = float4(1, 1, 1, 1);
    float kd = CalcDiffuseCoefficient(incident_light_ray, surface_normal);
    float4 diffuse_color = diffuse_coefficient * kd * light_color * albedo_color;

    float4 specular_color = float4(0, 0, 0, 0);
    float4 ks = CalcSpecularCoefficient(incident_light_ray, surface_normal, specular_power);
    specular_color = specular_coefficient * ks * light_color;

    return diffuse_color + specular_color;
}

float3 FresnelReflectanceSchlick(in float3 ray, in float3 surface_normal, in float3 f0)
{
    float cosi = saturate(dot(-ray, surface_normal));
    return f0 + (1 - f0) * pow(1 - cosi, 5);
}

#endif