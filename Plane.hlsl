#include "Common.hlsli"


[shader("closesthit")]void CLHPlane(inout RayPayload payload,
                                       Attributes attrib)
{
    float4 color = float4(0.8f, 0.8f, 0.8f, RayTCurrent());

	// Trace the ray
    Ray ray;
    ray.origin_ = CalcHitPosition();
    ray.direction_ = CalcReflectedRayDirection(float3(0, 1, 0));

    float4 reflected_color = TraceRadianceRay(ray, payload.recursion_depth_);
    payload.colorAndDistance = reflected_color * 0.8 + color;
    payload.colorAndDistance.w = 1.0f;
}


//float3 barycentrics =
    //  float3(1.f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);

    //uint vertId = 3 * PrimitiveIndex();
    //float3 hitColor = BTriVertex[vertId + 0].color * barycentrics.x +
    //                BTriVertex[vertId + 1].color * barycentrics.y +
    //                BTriVertex[vertId + 2].color * barycentrics.z;
