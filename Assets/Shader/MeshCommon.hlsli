


struct VertexShaderOutput
{
    float4 position_ : SV_POSITION;
    float4 normal_ : NORMAL;
    float4 tangent_ : TANGENT;
    float4 binormal_ : BINORMAL;
    float2 texcoord_ : TEXCOORD;
};


/**
 * \brief 
 * \param surface_normal Surface Normal
 * \param tangent Tangent
 * \param binormal Binormal
 * \param texture_normal ƒTƒ“ƒvƒŠƒ“ƒO‚µ‚½Œ‹‰Ê‚ð‚»‚Ì‚Ü‚Ü“ü‚ê‚Ä‚­‚¾‚³‚¢
 * \return Normal
 */
float4 CalcNormal(float4 surface_normal, float4 tangent, float4 binormal, float4 texture_normal)
{
    float3 N = normalize(surface_normal.xyz);
    float3 T = normalize(tangent.xyz);
    float3 B = normalize(binormal.xyz);

    texture_normal = (texture_normal * 2.0f) - 1.f;

    N = normalize((texture_normal.x * T) + (texture_normal.y * B) + (texture_normal.z * N));
    return float4(N.xyz, 0.0f);
}

float CalcDiffuseCoefficient(in float3 incident_light_ray, in float3 surface_normal)
{
    return saturate(dot(-incident_light_ray, surface_normal));
}

float4 CalcSpecularCoefficient(in float3 incident_light_ray, in float3 surface_normal, in float specular_power)
{
    float3 reflected_light_ray = normalize(reflect(incident_light_ray, surface_normal));
    return pow(saturate(dot(reflected_light_ray, normalize(incident_light_ray))), specular_power);
}



float4 CalcPhongLighting(in float4 light_direction, in float4 albedo_color, in float4 surface_normal,
						in float diffuse_coefficient, in float specular_coefficient,
						in float specular_power)
{

   // float3 incident_light_ray = normalize(scene_constant.light_position_.xyz);

    //White Light
    float4 light_color = float4(1, 1, 1, 1);
    float kd = CalcDiffuseCoefficient(light_direction.xyz, surface_normal.xyz);
    float4 diffuse_color = diffuse_coefficient * kd * light_color * albedo_color;

    float4 specular_color = float4(0, 0, 0, 0);
    float4 ks = CalcSpecularCoefficient(light_direction.xyz, surface_normal.xyz, specular_power);
    specular_color = specular_coefficient * ks * light_color;

    return diffuse_color + specular_color;
}