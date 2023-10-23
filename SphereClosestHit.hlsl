#include "Common.hlsli"

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
    albedo_color);

    reflection_color = reflectance_coefficient * float4(fresnel_r, 1) * reflection_color;

    

    float4 phong_color = CalcPhongLighting(albedo_color, attr.normal_,
    diffuse_coefficient, specular_coefficient, specular_power);
    float4 color = phong_color + reflection_color;
    payload.color_ = float4(color.rgb, 1.0f);
}