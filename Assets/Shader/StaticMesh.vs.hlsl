
struct Vertex
{
    float3 position_ : POSITION;
    float3 normal_ : NORMAL;
    float3 tangent_ : TANGENT;
    float3 binormal_ : BINORMAL;
    float2 texcoord_ : TEXCOORD;
};

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

float4 main( Vertex input ) : SV_POSITION
{
    float4 out_pos = mul(float4(input.position_, 1.0), mul(object_constant.world_, scene_constant.view_projection_matrix_));
    return out_pos;
}