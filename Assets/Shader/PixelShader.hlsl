
struct VertexShaderOut
{
    float4 position_ : SV_POSITION;
    float2 texcoord_ : TEXCOORD;
};

float4 main(VertexShaderOut pin) : SV_TARGET
{
    return float4(1, 1, 1, 1);
}