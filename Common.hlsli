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
    uint num_reflect_;
};


#define _MAX_REFLECTION_ 10
//static const uint kMaxReflection = 2;

// Attributes output by the raytracing when hitting a surface,
// here the barycentric coordinates
struct Attributes
{
    float2 bary;
};