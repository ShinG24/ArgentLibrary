
struct VertexShaderOut
{
    float4 position_ : SV_POSITION;
    float2 texcoord : TEXCOORD;
};
struct Constant
{
    float alpha_;
};

Texture2D texture : register(t0);
ConstantBuffer<Constant> constant : register(b0);
SamplerState linear_sampler : register(s0);

float4 main(VertexShaderOut pin) : SV_TARGET
{
    float4 color = texture.Sample(linear_sampler, pin.texcoord);
    return float4(color.rgb, clamp(constant.alpha_, 0, 1));
    return float4(pin.texcoord, 1.0f, 1.0f);
}