
Texture2D g_tex : register(t0);
SamplerState g_samp : register(s0);

struct PSInput
{
    float4 pos : SV_Position;
    float4 viewPos : POSITION;
    float4 worldPos : POSITION1;
    float4 norm : NORMAL;
    float2 uv : TEXCOORD0;
};

cbuffer UltBuffer : register(b5)
{
    float ultAmount; //0.0=通常　、1.0=黒
    float3 padding; //パディング
}




float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = g_tex.Sample(g_samp, input.uv);
 
    // ultAmountが0.0の場合は通常のテクスチャカラーを返す
    if(ultAmount == 0.0f)
    {
        return texColor;
    }
    
    //return texColor;
    
    texColor.rgb = lerp(texColor.rgb, float3(0.0f, 0.0f, 0.0f), ultAmount);
    
    return texColor;
}