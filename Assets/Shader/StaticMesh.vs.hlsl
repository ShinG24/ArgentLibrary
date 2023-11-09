
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


struct Vertex
{
    float3 position_ : POSITION;
    float3 normal_ : NORMAL;
    float3 tangent_ : TANGENT;
    float3 binormal_ : BINORMAL;
    float2 texcoord_ : TEXCOORD;
};


VertexShaderOutput main( Vertex input )
{
    VertexShaderOutput vs_out;
    vs_out.position_ = mul(float4(input.position_, 1.0), mul(object_constant.world_, scene_constant.view_projection_matrix_));
    vs_out.normal_ = mul(float4(input.normal_, 0.0f), object_constant.world_);
    vs_out.tangent_ = mul(float4(input.tangent_, 0.0f), object_constant.world_);
    vs_out.binormal_ = mul(float4(input.binormal_, 0.0f), object_constant.world_);
    vs_out.texcoord_ = input.texcoord_;
    return vs_out;
}