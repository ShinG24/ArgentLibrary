
struct VertexShaderOut
{
    float4 position_ : SV_POSITION;
    float2 texcoord_ : TEXCOORD;
};

Texture2D texture : register(t0);
SamplerState linear_sampler : register(s0);

float4 main(VertexShaderOut pin) : SV_TARGET
{
    float4 color = pin.position_ * 0.5f + 0.5f;
    return float4(color.rgb, 1.0f);
    return texture.Sample(linear_sampler, pin.texcoord_);
}