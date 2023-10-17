#include "Common.hlsli"


RaytracingAccelerationStructure SceneBVH : register(t0);

[shader("closesthit")] void ClosestHit(inout RayPayload payload,
                                       Attributes attrib)
{

  payload.colorAndDistance = float4((attrib.bary), 1.0f, RayTCurrent());

    float4 color = float4(0.8f, 0.2f, 0.1f, RayTCurrent());
    payload.colorAndDistance = float4(0.8f, 0.8f, 0.8f, RayTCurrent());



    RayDesc ray;
    ray.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    ray.Direction = reflect(WorldRayDirection(), float3(1, 0, 0));
    ray.TMin = 0.f;
    ray.TMax = 1000.0f;

        // Trace the ray

    if (payload.num_reflect_ < 3)
    {
        payload.num_reflect_ += 1;
       // payload.num_reflect_ += 1;
        TraceRay(SceneBVH, RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
	      0xFF, 0, 1, 0, ray, payload);
    }

    float4 reflected_color = payload.colorAndDistance;
    payload.colorAndDistance = reflected_color * 0.2 + color;
    payload.colorAndDistance.w = 1.0f;

}
