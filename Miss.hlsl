#include "Common.hlsli"


struct SceneConstant
{
    row_major float4x4 inv_view_projection_;
    float4 camera_position_;
    float4 light_position_;
};

ConstantBuffer<SceneConstant> scene_constant : register(b0);


[shader("miss")] void Miss(inout RayPayload payload
                           : SV_RayPayload) {
  uint2 launchIndex = DispatchRaysIndex().xy;
  float2 dims = float2(DispatchRaysDimensions().xy);

    float3 position = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();

    float y = position.y - scene_constant.camera_position_.y;
    y /= 10000.0f;
    y = y * 0.5 + 0.5f;

    payload.colorAndDistance = float4(y * y, y * y, y, RayTCurrent());

    //float4 col = payload.colorAndDistance;
    //payload.colorAndDistance = col;
    
    //float ramp = launchIndex.y / dims.y;

    //payload.colorAndDistance = float4(payload.colorAndDistance.xyz, RayTCurrent());
	//payload.colorAndDistance = float4(1, 0, 0, -1.0);
    //payload.colorAndDistance = float4(0.0f, 0.2f, 0.7f - 0.3f * ramp, -1.0f);
}