

struct VSInput
{
    float4 pos : POSITION; //モデルの頂点座標
    float4 norm : NORMAL; //モデルの法線
    float4 color : COLOR; //ディフューズ
    float4 color2 : COLOR1; //スペキュラ
    float2 uv : TEXCOORD0; //UV
    float2 suv : TEXCOORD1; //サブUV
};

struct VSOutput
{
    float4 pos : SV_POSITION; //ラスタライザを動作させるために必須
    float4 viewPos : POSITION; //プロジェクションを適用していない座標
    float4 worldPos : POSITION1; //ワールド変換だけを行った頂点座標
    float4 norm : NORMAL;
    float2 uv : TEXCOORD0;  
};

cbuffer BaseCBuffer : register(b4)
{
    matrix worldMat; //プロジェクション行列
    matrix viewMat; //カメラ行列
    matrix projMat; //ワールド行列
    float4 various;
}


VSOutput main( VSInput input )
{
    VSOutput output;
    
    
    float4 worldPos = mul(worldMat, input.pos); // モデル座標→ワールド座標
    float4 viewPos = mul(viewMat, worldPos); // ワールド座標→カメラ視点座標
    float4 projPos = mul(projMat, viewPos); // カメラ視点座標→画面座標
    
    output.pos = projPos;
    output.viewPos = viewPos;
    output.worldPos = worldPos;
    output.norm = input.norm;
    output.uv = input.uv;
    return output;
}