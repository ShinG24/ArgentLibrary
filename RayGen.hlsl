#include "Common.hlsli"

inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray.
    //Why not float4(screenPos, '1', 1). ????
    float4 world = mul(float4(screenPos, 0.1f, 1), scene_constant.inv_view_projection_);

    world.xyz /= world.w;
    origin = scene_constant.camera_position_.xyz;
    direction = normalize(world.xyz - origin);
}

_RAY_GENERATION_SHADER_
void RayGen()
{
	uint2 launchIndex = DispatchRaysIndex().xy;
    float3 origin, direction;
    GenerateCameraRay(launchIndex, origin, direction);
    Ray ray;
    ray.origin_ = origin;
    ray.direction_ = direction;

    float4 color = TraceRadianceRay(ray, 0u);
  
	output_texture[launchIndex] = float4(color.rgb, 1.f);
}
