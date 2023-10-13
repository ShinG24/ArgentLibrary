#include "Common.hlsli"

//struct STriVertex
//{
//    float3 vertex;
//    float4 color;
//};

//StructuredBuffer<STriVertex> BTriVertex : register(t0);
RaytracingAccelerationStructure SceneBVH : register(t0);
RWTexture2D<float4> gOutput : register(u0);

[shader("closesthit")]void CLHPlane(inout RayPayload payload,
                                       Attributes attrib)
{
    //float3 barycentrics =
    //  float3(1.f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);

    //uint vertId = 3 * PrimitiveIndex();
    //float3 hitColor = BTriVertex[vertId + 0].color * barycentrics.x +
    //                BTriVertex[vertId + 1].color * barycentrics.y +
    //                BTriVertex[vertId + 2].color * barycentrics.z;



    float4 color = float4(0.8f, 0.8f, 0.8f, RayTCurrent());
    payload.colorAndDistance = float4(0.8f, 0.8f, 0.8f, RayTCurrent());
	//payload.colorAndDistance = float4(0, 1, 0, RayTCurrent());

    //payload.colorAndDistance = float4(hitColor, RayTCurrent());

    gOutput[DispatchRaysIndex().xy] = float4(payload.colorAndDistance.xyz, 1.0f);

    RayDesc ray;
    ray.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    ray.Direction = float3(0.0f, 1.0f, 0.0f);
    ray.TMin = 0.0001f;
    ray.TMax = 1000.0f;

        // Trace the ray

    if (payload.num_reflect_ < _MAX_REFLECTION_)
    {
        payload.num_reflect_ += 1;
       // payload.num_reflect_ += 1;
        TraceRay(SceneBVH, RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
	      0xFF, 0, 1, 0, ray, payload);
    }

    payload.colorAndDistance += payload.colorAndDistance * 0.3 + color * 0.5f;
    payload.colorAndDistance.w = 1.0f;
}
