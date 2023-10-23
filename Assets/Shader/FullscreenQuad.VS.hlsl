
struct VertexShaderOut
{
    float4 position_ : SV_POSITION;
    float2 texcoord_ : TEXCOORD;
};

VertexShaderOut main(uint vertex_id : SV_VERTEXID)
{
    const float2 position[4] = { float2(-1, 1), float2(1, 1), float2(-1, -1), float2(1, -1) };
    const float2 texcoord[4] = { float2(0, 0), float2(1, 0), float2(0, 1), float2(1, 1)};
    VertexShaderOut vout;
    vout.position_ = float4(position[vertex_id], 0.0f, 0.0f);
    vout.texcoord_ = texcoord[vertex_id];
    return vout;
}