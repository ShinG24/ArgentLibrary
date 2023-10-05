
struct VertexShaderOut
{
    float4 position_ : SV_POSITION;
    float4 color_ : COLOR;
};

float4 main(VertexShaderOut pin) : SV_TARGET
{
    return float4(1, 1, 1, 1);
}