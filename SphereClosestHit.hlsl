#include "Common.hlsli"

[shader("closesthit")]
void SphereClosestHit(inout RayPayload payload, in SphereHitAttribute attr)
{
    float4 albedo_color = float4(1, 1, 1, 1);

    Ray ray;
    ray.origin_ = CalcHitWorldPosition();
    ray.direction_ = CalcReflectedRayDirection(attr.normal_);
    float4 reflection_color = TraceRadianceRay(ray, payload.recursion_depth_);

    float reflectance_coefficient = 1.0f;
    float3 fresnel_r = FresnelReflectanceSchlick(WorldRayDirection(), attr.normal_,
    albedo_color);

    reflection_color = reflectance_coefficient * float4(fresnel_r, 1) * reflection_color;

    float diffuse_coefficient = 0.0f;
    float specular_coefficient = 1.0f;
    float specular_power = 50.0f;

    float4 phong_color = CalcPhongLighting(albedo_color, attr.normal_,
    diffuse_coefficient, specular_coefficient, specular_power);
    float4 color = phong_color + reflection_color;
    payload.colorAndDistance = float4(color.rgb, 1.0f);


    payload.colorAndDistance = float4(1, 1, 1, 1);
}