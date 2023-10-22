#include "Common.hlsli"

Texture2D skymap_texture[4] : register(t0, space1);

struct Constant
{
    int material_index_;
};
ConstantBuffer<Constant> material_index : register(b0, space1);

[shader("miss")] void Miss(inout RayPayload payload
                           : SV_RayPayload)
{
	uint2 launchIndex = DispatchRaysIndex().xy;
	float2 dims = float2(DispatchRaysDimensions().xy);

    float3 position = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
    float3 camera_position = scene_constant.camera_position_.xyz;

    float3 vec = normalize(position - camera_position);

    const float PI = 3.14159265358979;
    float2 uv;
    uv.x = (atan2(vec.z, vec.x) + PI) / (PI * 2.0f);
    uv.y = 1.0 - (asin(vec.y) + PI * 0.5) / PI;

    uint2 dimension;
    skymap_texture[material_index.material_index_].GetDimensions(dimension.x, dimension.y);
    float4 sky_color = skymap_texture[material_index.material_index_][uv * dimension];


    float y = position.y - scene_constant.camera_position_.y;
    y = min(1000.0f, y);
    y /= 1000.0f;
    y = y * 0.5 + 0.5f;

    payload.colorAndDistance = float4(y * y, y * y, y, RayTCurrent());

    payload.colorAndDistance = sky_color;
}