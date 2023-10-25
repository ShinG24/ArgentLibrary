#include "Common.hlsli"

ConstantBuffer<ObjectConstant> object_constant : register(b0, space1);
ConstantBuffer<Material> material_constant : register(b1, space1);

_CLOSEST_HIT_SHADER_
void PlaneClosestHit(inout RayPayload payload, in HitAttribute attr)
{
    float4 albedo_color = material_constant.albedo_color_;
    float reflectance_coefficient = material_constant.reflectance_coefficient_;
    float diffuse_coefficient = material_constant.diffuse_coefficient_;
    float specular_coefficient = material_constant.specular_coefficient_;
    float specular_power = material_constant.specular_power_;

    //float4 albedo_color = float4(0.8f, 0.8f, 0.8f, RayTCurrent());
    float3 surface_normal = float3(0, 1, 0);
    surface_normal = mul(float4(surface_normal, 0), object_constant.world_).xyz;

	// Trace the ray
    Ray ray;
    ray.origin_ = CalcHitWorldPosition();
    ray.direction_ = CalcReflectedRayDirection(surface_normal);

    float4 reflection_color = TraceRadianceRay(ray, payload.recursion_depth_);

#if 0
	payload.color_ = reflected_color * 0.8 + color;
    payload.color_.w = 1.0f;
#else

    
    float3 fresnel_r = FresnelReflectanceSchlick(WorldRayDirection(), surface_normal, albedo_color.xyz);
    reflection_color = reflectance_coefficient * float4(fresnel_r, 1) * reflection_color;

    
    float4 phong_color = CalcPhongLighting(albedo_color, surface_normal,
						diffuse_coefficient, specular_coefficient, specular_power);
    float4 color = phong_color + reflection_color;

    payload.color_ = float4(color.rgb, 1.0f);
#endif

}


//float3 barycentrics =
    //  float3(1.f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);

    //uint vertId = 3 * PrimitiveIndex();
    //float3 hitColor = BTriVertex[vertId + 0].color * barycentrics.x +
    //                BTriVertex[vertId + 1].color * barycentrics.y +
    //                BTriVertex[vertId + 2].color * barycentrics.z;
