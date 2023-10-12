#include "Common.hlsl"

struct Vertex
{
    float3 position_;
    float3 normal_;
};

StructuredBuffer<Vertex> vertices : register(t1);
ByteAddressBuffer Indices : register(t2);

struct SceneConstant
{
    row_major float4x4 inv_view_projection_;
    float4 camera_position_;
    float4 light_position_;
};

ConstantBuffer<SceneConstant> scene_constant : register(b0);



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


[shader("closesthit")]void CubeHit(inout HitInfo payload,
                                       Attributes attrib)
{
    float3 hit_position = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

    uint index_size_in_bytes = 2u;
    uint indices_per_triangle = 3u;
    uint triangle_index_stride = index_size_in_bytes * indices_per_triangle;
    uint offset_index = PrimitiveIndex() * triangle_index_stride;

    uint3 index = Load3x16BitIndices(offset_index);
    //uint3 index = Indices.Load3(offset_index);

    
    float3 vertex_normal[3] =
    {
        vertices[index[0]].normal_,
        vertices[index[1]].normal_,
        vertices[index[2]].normal_,
    };

    float3 triangle_normal = vertex_normal[0] +
    attrib.bary.x * (vertex_normal[1] - vertex_normal[0]) +
    attrib.bary.y * (vertex_normal[2] - vertex_normal[0]);

    //float3 light_direction = float3(1.0, -1.0, 1.0);
    //float3 light_direction = normalize(float3(1.0f, -1.0f, 1.0f));
    //float3 pixel_to_light = normalize(scene_constant.light_position_ - hit_position);

    float fNDotL = max(0.0f, dot(-scene_constant.light_position_, triangle_normal));
    //float fNDotL = max(0.0f, dot(pixel_to_light, triangle_normal));

    float3 color = float3(1, 1, 1) * fNDotL;


    payload.colorAndDistance = float4(color, RayTCurrent());


    float3 col = abs(index.xyz - 36.0f);
    col /= 36.0f;
payload.colorAndDistance = float4(col, RayTCurrent());

  // payload.colorAndDistance = float4(triangle_normal, RayTCurrent()); 
   

    //float3 barycentrics =
    //  float3(1.f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);

    //uint vertId = 3 * PrimitiveIndex();
    //float3 hitColor = abs(vertices[vertId + 0].normal_) * barycentrics.x +
    //                abs(vertices[vertId + 1].normal_) * barycentrics.y +
    //                abs(vertices[vertId + 2].normal_) * barycentrics.z;
    //payload.colorAndDistance = float4(hitColor, RayTCurrent());
    //payload.colorAndDistance = float4(1.0f, attrib.bary, RayTCurrent());
}
