
struct VertexShaderOut
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 main(VertexShaderOut pin)
{
    return float4(1, 1, 1, 1);
}