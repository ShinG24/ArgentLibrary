#include "Common.hlsli"

// Raytracing output texture, accessed as a UAV
RWTexture2D<float4> gOutput : register(u0);

// Raytracing acceleration structure, accessed as a SRV
RaytracingAccelerationStructure SceneBVH : register(t0);

struct SceneConstant
{
    row_major float4x4 inv_view_projection_;
    float4 camera_position_;
    float4 light_position_;
};

ConstantBuffer<SceneConstant> scene_constant : register(b0);


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


[shader("raygeneration")] void RayGen() {
  // Initialize the ray payload
  RayPayload payload;
 payload.colorAndDistance = float4(0, 0, 0, 0);
    payload.num_reflect_ = 0u;
    //payload.colorAndDistance = scene_constant.camera_position_;

  // Get the location within the dispatched 2D grid of work items
  // (often maps to pixels, so this could represent a pixel coordinate).
  uint2 launchIndex = DispatchRaysIndex().xy;
  float2 dims = float2(DispatchRaysDimensions().xy);
  float2 d = (((launchIndex.xy + 0.5f) / dims.xy) * 2.f - 1.f);
  // Define a ray, consisting of origin, direction, and the min-max distance
  // values
  RayDesc ray;

#if 0
  ray.Origin = float3(d.x, -d.y, 1);
  ray.Direction = float3(0, 0, -1);
#else

    float3 ray_origin;
    float3 ray_direction;
    GenerateCameraRay(DispatchRaysIndex().xy, ray_origin, ray_direction);
    ray.Origin = ray_origin;
    ray.Direction = ray_direction;

    //ray.Origin = float3(d.x, -d.y, -0.001);
    //ray.Direction = float3(0, 0, 1);
#endif

	ray.TMin = 0.001;
  ray.TMax = 10000.0;

  // Trace the ray
  TraceRay(
      // Parameter name: AccelerationStructure
      // Acceleration structure
      SceneBVH,

      // Parameter name: RayFlags
      // Flags can be used to specify the behavior upon hitting a surface
      RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
      //RAY_FLAG_NONE,

      // Parameter name: InstanceInclusionMask
      // Instance inclusion mask, which can be used to mask out some geometry to
      // this ray by and-ing the mask with a geometry mask. The 0xFF flag then
      // indicates no geometry will be masked
      0xFF,

      // Parameter name: RayContributionToHitGroupIndex
      // Depending on the type of ray, a given object can have several hit
      // groups attached (ie. what to do when hitting to compute regular
      // shading, and what to do when hitting to compute shadows). Those hit
      // groups are specified sequentially in the SBT, so the value below
      // indicates which offset (on 4 bits) to apply to the hit groups for this
      // ray. In this sample we only have one hit group per object, hence an
      // offset of 0.
      0,

      // Parameter name: MultiplierForGeometryContributionToHitGroupIndex
      // The offsets in the SBT can be computed from the object ID, its instance
      // ID, but also simply by the order the objects have been pushed in the
      // acceleration structure. This allows the application to group shaders in
      // the SBT in the same order as they are added in the AS, in which case
      // the value below represents the stride (4 bits representing the number
      // of hit groups) between two consecutive objects.
      1,

      // Parameter name: MissShaderIndex
      // Index of the miss shader to use in case several consecutive miss
      // shaders are present in the SBT. This allows to change the behavior of
      // the program when no geometry have been hit, for example one to return a
      // sky color for regular rendering, and another returning a full
      // visibility value for shadow rays. This sample has only one miss shader,
      // hence an index 0
      0,

      // Parameter name: Ray
      // Ray information to trace
      ray,

      // Parameter name: Payload
      // Payload associated to the ray, which will be used to communicate
      // between the hit/miss shaders and the raygen
      payload);
  gOutput[launchIndex] = float4(payload.colorAndDistance.rgb, 1.f);
}
