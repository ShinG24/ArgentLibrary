

struct VertexShaderOut
{
    float4 position_ : SV_POSITION;
    float2 texcoord_ : TEXCOORD;
};

struct VertexShaderIn
{
    float2 position_ : POSITION;
    float2 texcoord_ : TEXCOORD;
};

VertexShaderOut main(uint id : SV_VERTEXID)
{
    const float2 position[4] = { float2(-1, 1), float2(1, 1), float2(-1, -1), float2(1, -1) };
    VertexShaderOut vout;
    vout.position_ = float4(position[id], 0.0f, 1.0f);
    //vout.position_ = float4(vin.position_, 0.5f, 1.0f);
    vout.texcoord_ =float2(0, 0);
    return vout;
}