

struct VertexShaderOut
{
    float4 position_ : SV_POSITION;
    float4 color_ : COLOR;
};

struct VertexShaderIn
{
    float3 position_ : POSITION;
    float4 color_ : COLOR;
};

VertexShaderOut main(VertexShaderIn vin)
//VertexShaderOut main(uint id : SV_VERTEXID)
{
    const float2 position[4] = { float2(-1, 1), float2(1, 1), float2(-1, -1), float2(1, -1) };
    VertexShaderOut vout;
    //vout.position_ = float4(position[id], 0.0f, 1.0f);
    vout.position_ = float4(vin.position_.xy, 0.5f, 1.0f);
    vout.color_ = float4(1, 1, 1, 1);
    return vout;
}