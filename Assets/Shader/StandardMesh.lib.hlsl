#include "Common.hlsli"


struct StandardMaterial
{
	float metallic_;
	float roughness_;
    float2 texcoord_offset_;
};

Texture2D albedo_texture : register(t0, space1);
Texture2D normal_texture : register(t1, space1);

StructuredBuffer<float3> sb_position	: register(t2, space1);
StructuredBuffer<float3> sb_normal		: register(t3, space1);
StructuredBuffer<float3> sb_tangent		: register(t4, space1);
StructuredBuffer<float3> sb_binormal	: register(t5, space1);
StructuredBuffer<float2> sb_texcoord	: register(t6, space1);
ByteAddressBuffer Indices				: register(t7, space1);

ConstantBuffer<ObjectConstant> object_constant : register(b0, space1);
ConstantBuffer<StandardMaterial> material_constant : register(b1, space1);

uint3 Load3x32BitIndices()
{
	uint offset_index = PrimitiveIndex() * 4 * 3;
	return Indices.Load3(offset_index);
}

float3 CalcWorldNormal(uint3 index, float2 barycentrics)
{
	float3 vertex_normal[3] = { sb_normal[index.x].xyz, sb_normal[index.y].xyz, sb_normal[index.z].xyz, };
	float3 triangle_normal = vertex_normal[0] * (1 - barycentrics.x - barycentrics.y) + vertex_normal[1] * barycentrics.x + vertex_normal[2] * barycentrics.y;
	float3 ret = mul(float4(triangle_normal, 0.0f), object_constant.world_).xyz;
	return normalize(ret);
}

float3 CalcWorldTangent(uint3 index, float2 barycentrics)
{
	float3 vertex_tangent[3] = { sb_tangent[index.x], sb_tangent[index.y], sb_tangent[index.z] };
	float3 tangent = vertex_tangent[0] + barycentrics.x * (vertex_tangent[1] - vertex_tangent[0]) + barycentrics.y * (vertex_tangent[2] - vertex_tangent[0]);
	tangent = normalize(mul(float4(tangent, 0), object_constant.world_).xyz);
	return tangent;
}

float3 CalcWorldBinormal(uint3 index, float2 barycentrics)
{
	float3 vertex_binormal[3] = { sb_binormal[index.x], sb_binormal[index.y], sb_binormal[index.z] };
	float3 binormal = vertex_binormal[0] + barycentrics.x * (vertex_binormal[1] - vertex_binormal[0]) + barycentrics.y * (vertex_binormal[2] - vertex_binormal[0]);
	binormal = normalize(mul(float4(binormal, 0), object_constant.world_).xyz);
	return binormal;
}

/**
 * \brief Texcoordの計算
 * \param index 頂点バッファに対するインデックス
 * \param barycentrics 重心
 * \return Texcoord
 */
float2 CalcTexcoord(uint3 index, float2 barycentrics)
{
	float2 vertex_texcoord[3] = { sb_texcoord[index[0]].xy, sb_texcoord[index[1]].xy, sb_texcoord[index[2]].xy };
	float2 texcoord = vertex_texcoord[0] + barycentrics.x * (vertex_texcoord[1] - vertex_texcoord[0]) + barycentrics.y * (vertex_texcoord[2] - vertex_texcoord[0]);
	//1を超えていた場合用に小数点以下だけを抽出する
	texcoord = frac(texcoord);
	return texcoord;
}

float4 NormalLinearSampling(float2 uv, uint dimension)
{
	float4 color = 0;
	color += normal_texture[uv]; //Center

	float offset_pixel = 1.0f;

	color += normal_texture[clamp(float2(uv.x - offset_pixel, uv.y), 0, dimension)]; //Left
	color += normal_texture[clamp(float2(uv.x - offset_pixel, uv.y - offset_pixel), 0, dimension)]; //Left Top
	color += normal_texture[clamp(float2(uv.x, uv.y - offset_pixel), 0, dimension)]; //Top
	color += normal_texture[clamp(float2(uv.x + offset_pixel, uv.y - offset_pixel), 0, dimension)]; //Right Top
	color += normal_texture[clamp(float2(uv.x + offset_pixel, uv.y), 0, dimension)]; //Right 
	color += normal_texture[clamp(float2(uv.x + offset_pixel, uv.y + offset_pixel), 0, dimension)]; //Right Bottom
	color += normal_texture[clamp(float2(uv.x, uv.y + offset_pixel), 0, dimension)]; //Bottom
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
	color += albedo_texture[uv]; //Center

	float offset_pixel = 1.0f;
    
	color += albedo_texture[clamp(float2(uv.x - offset_pixel, uv.y), 0, dimension)]; //Left
	color += albedo_texture[clamp(float2(uv.x - offset_pixel, uv.y - offset_pixel), 0, dimension)]; //Left Top
	color += albedo_texture[clamp(float2(uv.x, uv.y - offset_pixel), 0, dimension)]; //Top
	color += albedo_texture[clamp(float2(uv.x + offset_pixel, uv.y - offset_pixel), 0, dimension)]; //Right Top
	color += albedo_texture[clamp(float2(uv.x + offset_pixel, uv.y), 0, dimension)]; //Right 
	color += albedo_texture[clamp(float2(uv.x + offset_pixel, uv.y + offset_pixel), 0, dimension)]; //Right Bottom
	color += albedo_texture[clamp(float2(uv.x, uv.y + offset_pixel), 0, dimension)]; //Bottom
	color += albedo_texture[clamp(float2(uv.x - offset_pixel, uv.y + offset_pixel), 0, dimension)]; //Left Bottom
	color /= 9.0f;
	return float4(color.rgb, 1.0f);
}

float4 AlbedoSampling(uint3 index, float2 barycentrics)
{
	float2 texcoord0 = sb_texcoord[index.x];
	float2 texcoord1 = sb_texcoord[index.y];
	float2 texcoord2 = sb_texcoord[index.z];

	texcoord0 = frac(texcoord0);
	texcoord1 = frac(texcoord1);
	texcoord2 = frac(texcoord2);

	//texcoord0 = abs(material_constant.texcoord_offset_.xy - texcoord0);
	//texcoord1 = abs(material_constant.texcoord_offset_.xy - texcoord1);
	//texcoord2 = abs(material_constant.texcoord_offset_.xy - texcoord2);

	uint2 dimension;
	albedo_texture.GetDimensions(dimension.x, dimension.y);

	float4 color0 = AlbedoLinearSampling(texcoord0 * dimension, dimension.x);
	float4 color1 = AlbedoLinearSampling(texcoord1 * dimension, dimension.x);
	float4 color2 = AlbedoLinearSampling(texcoord2 * dimension, dimension.x);
	return color0 * (1 - barycentrics.x - barycentrics.y) + color1 * barycentrics.x + color2 * barycentrics.y;
}

float CalcDiffuseFromFresnel(float3 N, float3 L, float3 V)
{
    float3 H = normalize(L + V);
    float energy_bias = lerp(0.0f, 0.5f, material_constant.roughness_);
    float energy_factor = lerp(1.0f, 1.0f / 1.51, material_constant.roughness_);

    float dotLH = saturate(dot(L, H));

    float fd90 = energy_bias + 2.0f * dotLH * dotLH * material_constant.roughness_;

    float dotNL = saturate(dot(N, L));
    float FL = (1 + (fd90 - 1) * pow(1 - dotNL, 5));

    float dotNV = saturate(dot(N, V));
    float FV = (1 + (fd90 - 1) * pow(1 - dotNV, 5));

    return FL * FV * energy_factor;
}

float CookTorranceSpecular(float3 L, float3 V, float3 N, float metallic)
{
    float microfacet = 0.76f;
    float f0 = metallic;

    float3 H = normalize(L + V);

    float nDotH = saturate(dot(N, H));
    float vDotH = saturate(dot(V, H));
    float nDotL = saturate(dot(N, L));
    float nDotV = saturate(dot(N, V));

    float roughness = material_constant.roughness_;
    float D = roughness * roughness  * step(0, nDotH)/
		(3.1415 * pow(((nDotH * nDotH) * (roughness * roughness - 1) + 1), 2));

    float G = min(1.0f, min(2 * nDotH * nDotV / vDotH, 2 * nDotH * nDotL / vDotH));

    float F = f0 + (1 - f0) * pow(1 - nDotL, 5);

    float m = 3.1415 * nDotV * nDotH;

    return max(F * D * G / m, 0.0f);
}

_CLOSEST_HIT_SHADER_
void StaticMeshClosestHit(inout RayPayload payload, in HitAttribute attr)
{
	uint3 index = Load3x32BitIndices();
	float3 world_normal = CalcWorldNormal(index, attr.barycentrics);
	float3 world_tangent = CalcWorldTangent(index, attr.barycentrics);
	float3 world_binormal = CalcWorldBinormal(index, attr.barycentrics);
	float2 texcoord = CalcTexcoord(index, attr.barycentrics);


    uint2 dimension;
	normal_texture.GetDimensions(dimension.x, dimension.y);
	float2 normal_texcoord = float2(texcoord.x * dimension.x, texcoord.y * dimension.y);

	float3 normal = CalcNormal(world_normal, world_tangent.xyz, world_binormal.xyz, normal_texcoord);


	float4 albedo_color = AlbedoSampling(index, attr.barycentrics) ;
  
	
    //Do Raytracing
	Ray ray;
	ray.origin_ = CalcHitWorldPosition();
	ray.direction_ = CalcReflectedRayDirection(normal);
	float4 reflection_color = TraceRadianceRay(ray, payload.recursion_depth_);

    float reflection_coefficient = material_constant.metallic_;
    float diffuse_coefficient = 1.0f;
    float specular_coefficient = material_constant.roughness_;

    //Fresnel 
	float3 fresnel_r = FresnelReflectanceSchlick(WorldRayDirection(), normal, albedo_color.xyz);
    reflection_color = reflection_coefficient * float4(fresnel_r, 1) * reflection_color;

    //Phong Shading
	float4 phong_color = CalcPhongLighting(albedo_color, normal,
						diffuse_coefficient, specular_coefficient, 50.0f);
	float4 color = phong_color + reflection_color;

	payload.color_ = float4(color.rgb, 1.0f);
}