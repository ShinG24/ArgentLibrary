#include "Common.hlsli"


[shader("closesthit")]void CLHPlane(inout RayPayload payload,
                                       in HitAttribute attr)
{
    float4 albedo_color = float4(0.8f, 0.8f, 0.8f, RayTCurrent());
    float3 surface_normal = float3(0, 1, 0);
	// Trace the ray
    Ray ray;
    ray.origin_ = CalcHitWorldPosition();
    ray.direction_ = CalcReflectedRayDirection(surface_normal);

    float4 reflection_color = TraceRadianceRay(ray, payload.recursion_depth_);

#if 0
	payload.colorAndDistance = reflected_color * 0.8 + color;
    payload.colorAndDistance.w = 1.0f;
#else

    float reflectance_coefficient = 0.9f;
    float3 fresnel_r = FresnelReflectanceSchlick(WorldRayDirection(), surface_normal, albedo_color.xyz);
    reflection_color = reflectance_coefficient * float4(fresnel_r, 1) * reflection_color;

    float diffuse_coefficient = 1.0f;
    float specular_coefficient = 0.8f;
    float specular_power = 50.0f;
    float4 phong_color = CalcPhongLighting(albedo_color, surface_normal,
						diffuse_coefficient, specular_coefficient, specular_power);
    float4 color = phong_color + reflection_color;

    payload.colorAndDistance = float4(color.rgb, 1.0f);
#endif

}


//float3 barycentrics =
    //  float3(1.f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);

    //uint vertId = 3 * PrimitiveIndex();
    //float3 hitColor = BTriVertex[vertId + 0].color * barycentrics.x +
    //                BTriVertex[vertId + 1].color * barycentrics.y +
    //                BTriVertex[vertId + 2].color * barycentrics.z;
