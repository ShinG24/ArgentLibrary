#include "Common.hlsli"


[shader("closesthit")] void ClosestHit(inout RayPayload payload,
                                       Attributes attrib)
{

  payload.colorAndDistance = float4((attrib.bary), 1.0f, RayTCurrent());

    float4 color = float4(0.8f, 0.2f, 0.1f, RayTCurrent());

    Ray ray;
    ray.origin_ = CalcHitPosition();
    ray.direction_ = CalcReflectedRayDirection(float3(1, 0, 0));


    float4 reflected_color = TraceRadianceRay(ray, payload.recursion_depth_);
    payload.colorAndDistance = reflected_color * 0.2 + color;
    payload.colorAndDistance.w = 1.0f;

}
