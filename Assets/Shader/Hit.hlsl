#include "Common.hlsli"


_CLOSEST_HIT_SHADER_
void ClosestHit(inout RayPayload payload, in HitAttribute attr)
{
    float4 albedo_color = float4(1, 0, 0, 1);
    float3 surface_normal = float3(1, 0, 0);

    Ray ray;
    ray.origin_ = CalcHitWorldPosition();
    ray.direction_ = CalcReflectedRayDirection(surface_normal);
    float4 reflection_color = TraceRadianceRay(ray, payload.recursion_depth_);
#if 0 

    payload.color_ = reflected_color * 0.2 + color;
    payload.color_.w = 1.0f;
#else

    float reflectance_coefficient = 0.6f;
    float3 fresnel_r = FresnelReflectanceSchlick(WorldRayDirection(), surface_normal, albedo_color.xyz);
    reflection_color = reflectance_coefficient * float4(fresnel_r, 1) * reflection_color;

    float diffuse_coefficient = 1.0f;
    float specular_coefficient = 0.1f;
    float specular_power = 50.0f;
    float4 phong_color = CalcPhongLighting(albedo_color, surface_normal,
						diffuse_coefficient, specular_coefficient, specular_power);
    float4 color = phong_color/* + reflection_color*/;

    payload.color_ = float4(color.rgb, 1.0f);

#endif


}
