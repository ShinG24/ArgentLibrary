#include "Common.hlsli"

struct Vertex
{
    float3 position_;
    float3 normal_;
    float4 tangent_;
    float4 binormal_;
    float2 texcoord_;
};

Texture2D albedo_texture : register(t0, space1);
Texture2D normal_texture : register(t1, space1);
StructuredBuffer<Vertex> vertices : register(t2, space1);
ByteAddressBuffer Indices : register(t3, space1);
ConstantBuffer<ObjectConstant> object_constant : register(b0, space1);
ConstantBuffer<Material> material_constant : register(b1, space1);


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

float3 CalcWorldNormal(uint3 index, float2 barycentrics)
{
    float3 vertex_normal[3] = { vertices[index.x].normal_.xyz, vertices[index.y].normal_.xyz, vertices[index.z].normal_.xyz, };
    float3 triangle_normal = vertex_normal[0] * (1 - barycentrics.x - barycentrics.y) + vertex_normal[1] * barycentrics.x + vertex_normal[2] * barycentrics.y;
    //float3 triangle_normal = vertex_normal[0] + barycentrics.x * (vertex_normal[1] - vertex_normal[0]) + barycentrics.y * (vertex_normal[2] - vertex_normal[0]);
    float3 ret = mul(float4(triangle_normal, 0.0f), object_constant.world_).xyz;
    return normalize(ret);
}

float4 CalcWorldTangent(uint3 index, float2 barycentrics)
{
    float4 vertex_tangent[3] = { vertices[index.x].tangent_, vertices[index.y].tangent_, vertices[index.z].tangent_ };
    float4 tangent = vertex_tangent[0] + barycentrics.x * (vertex_tangent[1] - vertex_tangent[0]) + barycentrics.y * (vertex_tangent[2] - vertex_tangent[0]);
    float sigma = vertex_tangent[0].w;
    tangent.w = 0;
    tangent = normalize(mul(tangent, object_constant.world_));
    tangent.w = sigma;
    return tangent;
}
float4 CalcWorldBinormal(uint3 index, float2 barycentrics)
{
    float4 vertex_binormal[3] = { vertices[index.x].binormal_, vertices[index.y].binormal_, vertices[index.z].binormal_ };
    float4 binormal = vertex_binormal[0] + barycentrics.x * (vertex_binormal[1] - vertex_binormal[0]) + barycentrics.y * (vertex_binormal[2] - vertex_binormal[0]);
    float sigma = vertex_binormal[0].w;
    binormal.w = 0;
    binormal = normalize(mul(binormal, object_constant.world_));
    binormal.w = sigma;
    return binormal;
}

float2 CalcTexcoord(uint3 index, float2 barycentrics)
{
    float2 vertex_texcoord[3] = { vertices[index[0]].texcoord_.xy, vertices[index[1]].texcoord_.xy, vertices[index[2]].texcoord_.xy };
    float2 texcoord = vertex_texcoord[0] + barycentrics.x * (vertex_texcoord[1] - vertex_texcoord[0]) + barycentrics.y * (vertex_texcoord[2] - vertex_texcoord[0]);
    return texcoord;
}

float3 CalcNormal(float3 surface_normal, float3 tangent, float3 binormal, float2 uv)
{
    float3 N = normalize(surface_normal);
    float3 T = normalize(tangent.xyz);
    float3 B = normalize(binormal.xyz);


    float4 normal = normal_texture[uv];
    normal = (normal * 2.0f) - 1.f;

    N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
    return N;
}

float4 AlbedoLinearSampling(float2 uv, uint dimension)
{
    float4 color = 0;
    color += albedo_texture[uv];//Center
    
    color += albedo_texture[clamp(float2(uv.x - 1, uv.y    ), 0, dimension)];//Left
    color += albedo_texture[clamp(float2(uv.x - 1, uv.y - 1), 0, dimension)];//Left Top
    color += albedo_texture[clamp(float2(uv.x    , uv.y - 1), 0, dimension)];//Top
    color += albedo_texture[clamp(float2(uv.x + 1, uv.y - 1), 0, dimension)];//Right Top
    color += albedo_texture[clamp(float2(uv.x + 1, uv.y    ), 0, dimension)];//Right 
    color += albedo_texture[clamp(float2(uv.x + 1, uv.y + 1), 0, dimension)];//Right Bottom
    color += albedo_texture[clamp(float2(uv.x    , uv.y + 1), 0, dimension)];//Bottom
    color += albedo_texture[clamp(float2(uv.x - 1, uv.y + 1), 0, dimension)];//Left Bottom
    color /= 9.0f;
    return float4(color.rgb, 1.0f);
}

#define _USE_MATERIAL_CONSTANT_ 1

[shader("closesthit")]void CubeHit(inout RayPayload payload,
                                       in HitAttribute attr)
{
    uint3 index = Load3x32BitIndices();
	float3 world_normal = CalcWorldNormal(index, attr.barycentrics);
    float4 world_tangent = CalcWorldTangent(index, attr.barycentrics);
    float4 world_binormal = CalcWorldBinormal(index, attr.barycentrics);
    float2 texcoord = CalcTexcoord(index, attr.barycentrics);
    texcoord.y = 1 - texcoord.y;

    uint width, height;
	albedo_texture.GetDimensions(width, height);
    float2 albedo_texcoord = float2(texcoord.x * width, texcoord.y * height);

    normal_texture.GetDimensions(width, height);
    float2 normal_texcoord = float2(texcoord.x * width, texcoord.y * height);

    float3 normal = CalcNormal(world_normal, world_tangent.xyz, world_binormal.xyz, normal_texcoord);


    float4 albedo_color = AlbedoLinearSampling(albedo_texcoord, width);
    //float4 albedo_color = albedo_texture[albedo_texcoord.xy] * 4.0f;

    //Do Raytracing
    Ray ray;
    ray.origin_ = CalcHitWorldPosition();
    ray.direction_ = CalcReflectedRayDirection(normal);
    float4 reflection_color = TraceRadianceRay(ray, payload.recursion_depth_);

    //Fresnel 
    float3 fresnel_r = FresnelReflectanceSchlick(WorldRayDirection(), normal, albedo_color.xyz);
    reflection_color = material_constant.reflectance_coefficient_ * float4(fresnel_r, 1) * reflection_color;

    //Phong Shading
    float4 phong_color = CalcPhongLighting(albedo_color, normal,
						material_constant.diffuse_coefficient_, material_constant.specular_coefficient_, material_constant.specular_power_);
    float4 color = phong_color + reflection_color;

    payload.colorAndDistance = float4(color.rgb, 1.0f);
}



//uint index_size_in_bytes = 2u;
//uint indices_per_triangle = 3u;
//uint triangle_index_stride = index_size_in_bytes * indices_per_triangle;
//uint offset_index = PrimitiveIndex() * triangle_index_stride;