#include "Common.hlsli"


[shader("miss")] void Miss(inout RayPayload payload
                           : SV_RayPayload) {
  uint2 launchIndex = DispatchRaysIndex().xy;
  float2 dims = float2(DispatchRaysDimensions().xy);

    float3 position = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();

    float y = position.y - scene_constant.camera_position_.y;
    y = min(1000.0f, y);
    y /= 1000.0f;
    y = y * 0.5 + 0.5f;

    payload.colorAndDistance = float4(y * y, y * y, y, RayTCurrent());
}