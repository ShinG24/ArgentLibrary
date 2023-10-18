#include "Common.hlsli"

struct Vertex
{
    float3 position_;
    float3 normal_;
};

#define _USE_LOCAL_ROOT_SIGNATURE 0

#if _USE_LOCAL_ROOT_SIGNATURE
StructuredBuffer<Vertex> vertices : register(t1);
ByteAddressBuffer Indices : register(t2);

#else

struct ObjectConstant
{
    row_major float4x4 world_;
};

StructuredBuffer<Vertex> vertices : register(t0, space1);
ByteAddressBuffer Indices : register(t1, space1);
StructuredBuffer<ObjectConstant> object_constant : register(t2, space1);

#endif


// Load three 16 bit indices from a byte addressed buffer.
uint3 Load3x16BitIndices(uint offsetBytes)
{
    uint3 indices;

    // ByteAdressBuffer loads must be aligned at a 4 byte boundary.
    // Since we need to read three 16 bit indices: { 0, 1, 2 } 
    // aligned at a 4 byte boundary as: { 0 1 } { 2 0 } { 1 2 } { 0 1 } ...
    // we will load 8 bytes (~ 4 indices { a b | c d }) to handle two possible index triplet layouts,
    // based on first index's offsetBytes being aligned at the 4 byte boundary or not:
    //  Aligned:     { 0 1 | 2 - }
    //  Not aligned: { - 0 | 1 2 }
    const uint dwordAlignedOffset = offsetBytes & ~3;
    const uint2 four16BitIndices = Indices.Load2(dwordAlignedOffset);
 
    // Aligned: { 0 1 | 2 - } => retrieve first three 16bit indices
    if (dwordAlignedOffset == offsetBytes)
    {
        indices.x = four16BitIndices.x & 0xffff;
        indices.y = (four16BitIndices.x >> 16) & 0xffff;
        indices.z = four16BitIndices.y & 0xffff;
    }
    else // Not aligned: { - 0 | 1 2 } => retrieve last three 16bit indices
    {
        indices.x = (four16BitIndices.x >> 16) & 0xffff;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = (four16BitIndices.y >> 16) & 0xffff;
    }

    return indices;
}


uint3 Load3x32BitIndices()
{
    uint offset_index = PrimitiveIndex() * 4 * 3;
    return Indices.Load3(offset_index);
}


[shader("closesthit")]void CubeHit(inout RayPayload payload,
                                       Attributes attrib)
{
	float3 hit_position = CalcHitPosition();

    uint index_size_in_bytes = 2u;
    uint indices_per_triangle = 3u;
    uint triangle_index_stride = index_size_in_bytes * indices_per_triangle;
    uint offset_index = PrimitiveIndex() * triangle_index_stride;


#if 0
    uint index = Load3x16BitIndices(offset_index);
#else
    uint3 index = Load3x32BitIndices();
#endif
    
    float3 vertex_normal[3] =
    {
        vertices[index[0]].normal_,
        vertices[index[1]].normal_,
        vertices[index[2]].normal_,
    };

    float3 triangle_normal = vertex_normal[0] +
    attrib.bary.x * (vertex_normal[1] - vertex_normal[0]) +
    attrib.bary.y * (vertex_normal[2] - vertex_normal[0]);

    triangle_normal = mul(float4(triangle_normal, 0.0f), object_constant[0].world_).xyz;

    float diffuse_factor = max(0.0f, dot(normalize(-scene_constant.light_position_), triangle_normal));

    float3 color = float3(0.4f, 0.8f, 1.0f) * diffuse_factor;

    Ray ray;
    ray.origin_ = hit_position;
    ray.direction_ = CalcReflectedRayDirection(triangle_normal);

    float4 reflected_color = TraceRadianceRay(ray, payload.recursion_depth_);
    payload.colorAndDistance = reflected_color * 0.5 + float4(color, 1.0f);
    payload.colorAndDistance.w = 1.0f;

}
