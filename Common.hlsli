// Hit information, aka ray payload
// This sample only carries a shading color and hit distance.
// Note that the payload should be kept as small as possible,
// and that its size must be declared in the corresponding
// D3D12_RAYTRACING_SHADER_CONFIG pipeline subobjet.

#ifdef __cplusplus
#define uint    UINT
#endif

struct RayPayload
{
    float4 colorAndDistance;
    uint recursion_depth_;
};


#define _MAX_RECURSION_DEPTH_ 10
//static const uint kMaxReflection = 2;

// Attributes output by the raytracing when hitting a surface,
// here the barycentric coordinates
struct Attributes
{
    float2 bary;
};

struct Ray
{
    float3 origin_;
    float3 direction_;
};

#ifndef __cplusplus

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> output_texture : register(u0);

struct SceneConstant
{
    row_major float4x4 inv_view_projection_;
    float4 camera_position_;
    float4 light_position_;
};

ConstantBuffer<SceneConstant> scene_constant : register(b0);


static const uint kInstanceMask = ~0;

float3 CalcHitPosition()
{
    return WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
}

float3 CalcReflectedRayDirection(float3 normal)
{
    return reflect(WorldRayDirection(), normal);
}

float4 TraceRadianceRay(Ray ray, uint current_recursion_depth)
{
	if(current_recursion_depth >= _MAX_RECURSION_DEPTH_)
	{
        return float4(0, 0, 0, 0);
    }

    RayDesc ray_desc;
    ray_desc.Origin = ray.origin_;
    ray_desc.Direction = ray.direction_;
    ray_desc.TMin = 0.0f;
    ray_desc.TMax = 1000.0f;

    RayPayload payload;
	payload.colorAndDistance = float4(0, 0, 0, 0);
    payload.recursion_depth_ = current_recursion_depth + 1;

    TraceRay(scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
    kInstanceMask, 0, 1, 0, ray_desc, payload);

    return payload.colorAndDistance;
}

#endif