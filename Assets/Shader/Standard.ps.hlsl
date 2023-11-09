
#include "MeshCommon.hlsli"

struct SceneConstant
{
    float4 camera_position_;
    row_major float4x4 view_matrix_;
    row_major float4x4 projection_matrix_;
    row_major float4x4 view_projection_matrix_;
    row_major float4x4 inv_view_projection_matrix_;
    float4 light_direction_;
};

struct ObjectConstant
{
    row_major float4x4 world_;
};

ConstantBuffer<SceneConstant> scene_constant : register(b0);
ConstantBuffer<ObjectConstant> object_constant : register(b1);


Texture2D<float4> albedo_texture : register(t0);
Texture2D<float4> normal_texture : register(t1);

SamplerState linear_sampler : register(s0);

float4 main(VertexShaderOutput ps_input) : SV_TARGET
{
    float2 texcoord = float2(ps_input.texcoord_.x, 1.0f - ps_input.texcoord_.y);
    float4 color = albedo_texture.Sample(linear_sampler, texcoord);
    float4 normal = normal_texture.Sample(linear_sampler, texcoord);
    normal = CalcNormal(ps_input.normal_, ps_input.tangent_, ps_input.binormal_, normal);


    float4 ret = CalcPhongLighting(scene_constant.light_direction_, color, normal, 0.8f, 0.3f, 50.0f);
    return float4(ret.xyz, 1.0f);
}