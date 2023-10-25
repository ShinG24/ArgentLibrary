#include "Common.hlsli"

struct Vertex
{
    float3 position_;
    float3 normal_;
    float3 tangent_;
    float3 binormal_;
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

float3 CalcWorldTangent(uint3 index, float2 barycentrics)
{
    float3 vertex_tangent[3] = { vertices[index.x].tangent_, vertices[index.y].tangent_, vertices[index.z].tangent_ };
    float3 tangent = vertex_tangent[0] + barycentrics.x * (vertex_tangent[1] - vertex_tangent[0]) + barycentrics.y * (vertex_tangent[2] - vertex_tangent[0]);
    tangent = normalize(mul(float4(tangent, 0), object_constant.world_).xyz);
    return tangent;
}
float3 CalcWorldBinormal(uint3 index, float2 barycentrics)
{
    float3 vertex_binormal[3] = { vertices[index.x].binormal_, vertices[index.y].binormal_, vertices[index.z].binormal_ };
    float3 binormal = vertex_binormal[0] + barycentrics.x * (vertex_binormal[1] - vertex_binormal[0]) + barycentrics.y * (vertex_binormal[2] - vertex_binormal[0]);
    binormal = normalize(mul(float4(binormal, 0), object_constant.world_).xyz);
    return binormal;
}

float2 CalcTexcoord(uint3 index, float2 barycentrics)
{
    float2 vertex_texcoord[3] = { vertices[index[0]].texcoord_.xy, vertices[index[1]].texcoord_.xy, vertices[index[2]].texcoord_.xy };
    float2 texcoord = vertex_texcoord[0] + barycentrics.x * (vertex_texcoord[1] - vertex_texcoord[0]) + barycentrics.y * (vertex_texcoord[2] - vertex_texcoord[0]);
    return texcoord;
}

float4 NormalLinearSampling(float2 uv, uint dimension)
{
    float4 color = 0;
    color += normal_texture[uv]; //Center

    float offset_pixel = 1.0f;

    color += normal_texture[clamp(float2(uv.x - offset_pixel, uv.y               ), 0, dimension)]; //Left
    color += normal_texture[clamp(float2(uv.x - offset_pixel, uv.y - offset_pixel), 0, dimension)]; //Left Top
    color += normal_texture[clamp(float2(uv.x               , uv.y - offset_pixel), 0, dimension)]; //Top
    color += normal_texture[clamp(float2(uv.x + offset_pixel, uv.y - offset_pixel), 0, dimension)]; //Right Top
    color += normal_texture[clamp(float2(uv.x + offset_pixel, uv.y               ), 0, dimension)]; //Right 
    color += normal_texture[clamp(float2(uv.x + offset_pixel, uv.y + offset_pixel), 0, dimension)]; //Right Bottom
    color += normal_texture[clamp(float2(uv.x               , uv.y + offset_pixel), 0, dimension)]; //Bottom
    color += normal_texture[clamp(float2(uv.x - offset_pixel, uv.y + offset_pixel), 0, dimension)]; //Left Bottom
    color /= 9.0f;
    return float4(color.rgb, 1.0f);
}


float3 CalcNormal(float3 surface_normal, float3 tangent, float3 binormal, float2 uv)
{
    float3 N = normalize(surface_normal);
    float3 T = normalize(tangent.xyz);
    float3 B = normalize(binormal.xyz);


    uint2 dimension;
    normal_texture.GetDimensions(dimension.x, dimension.y);
    float4 normal = NormalLinearSampling(uv, dimension.x);
    normal = (normal * 2.0f) - 1.f;

    N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
    return N;
}

float4 AlbedoLinearSampling(float2 uv, uint dimension)
{
    float4 color = 0;
    color += albedo_texture[uv];//Center

    float offset_pixel = 1.0f;
    
    color += albedo_texture[clamp(float2(uv.x - offset_pixel, uv.y               ), 0, dimension)];//Left
    color += albedo_texture[clamp(float2(uv.x - offset_pixel, uv.y - offset_pixel), 0, dimension)];//Left Top
    color += albedo_texture[clamp(float2(uv.x               , uv.y - offset_pixel), 0, dimension)];//Top
    color += albedo_texture[clamp(float2(uv.x + offset_pixel, uv.y - offset_pixel), 0, dimension)];//Right Top
    color += albedo_texture[clamp(float2(uv.x + offset_pixel, uv.y               ), 0, dimension)];//Right 
    color += albedo_texture[clamp(float2(uv.x + offset_pixel, uv.y + offset_pixel), 0, dimension)];//Right Bottom
    color += albedo_texture[clamp(float2(uv.x               , uv.y + offset_pixel), 0, dimension)];//Bottom
    color += albedo_texture[clamp(float2(uv.x - offset_pixel, uv.y + offset_pixel), 0, dimension)]; //Left Bottom
    color /= 9.0f;
    return float4(color.rgb, 1.0f);
}

float4 AlbedoSampling(uint3 index, float2 barycentrics)
{
    float2 texcoord0 = vertices[index.x].texcoord_;
    float2 texcoord1 = vertices[index.y].texcoord_;
    float2 texcoord2 = vertices[index.z].texcoord_;

    texcoord0 = frac(texcoord0);
    texcoord1 = frac(texcoord1);
    texcoord2 = frac(texcoord2);

    texcoord0 = abs(material_constant.texcoord_offset_.xy - texcoord0);
    texcoord1 = abs(material_constant.texcoord_offset_.xy - texcoord1);
    texcoord2 = abs(material_constant.texcoord_offset_.xy - texcoord2);

    uint2 dimension;
    albedo_texture.GetDimensions(dimension.x, dimension.y);

    float4 color0 = AlbedoLinearSampling(texcoord0 * dimension, dimension.x);
    float4 color1 = AlbedoLinearSampling(texcoord1 * dimension, dimension.x);
    float4 color2 = AlbedoLinearSampling(texcoord2 * dimension, dimension.x);
    return color0 * (1 - barycentrics.x - barycentrics.y) + color1 * barycentrics.x + color2 * barycentrics.y;
}

#define _USE_MATERIAL_CONSTANT_ 1

_CLOSEST_HIT_SHADER_
void StaticMeshClosestHit(inout RayPayload payload, in HitAttribute attr)
{
    uint3 index = Load3x32BitIndices();
	float3 world_normal = CalcWorldNormal(index, attr.barycentrics);
    float3 world_tangent = CalcWorldTangent(index, attr.barycentrics);
    float3 world_binormal = CalcWorldBinormal(index, attr.barycentrics);
    float2 texcoord = CalcTexcoord(index, attr.barycentrics);
    texcoord = frac(texcoord);
    texcoord = abs(material_constant.texcoord_offset_.xy - texcoord);

    uint width, height;
	albedo_texture.GetDimensions(width, height);
    float2 albedo_texcoord = float2(texcoord.x * width, texcoord.y * height);

    normal_texture.GetDimensions(width, height);
    float2 normal_texcoord = float2(texcoord.x * width, texcoord.y * height);

    float3 normal = CalcNormal(world_normal, world_tangent.xyz, world_binormal.xyz, normal_texcoord);


    float4 albedo_color = AlbedoSampling(index, attr.barycentrics) * material_constant.albedo_color_;
  

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

    payload.color_ = float4(color.rgb, 1.0f);
}



//uint index_size_in_bytes = 2u;
//uint indices_per_triangle = 3u;
//uint triangle_index_stride = index_size_in_bytes * indices_per_triangle;
//uint offset_index = PrimitiveIndex() * triangle_index_stride;