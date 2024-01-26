struct cbTransformation
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 CameraPos;
};
ConstantBuffer<cbTransformation> cbTransform : register(b0);

struct VertexPosColor
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(VertexPosColor IN)
{
    PSInput result;

	result.position = mul(float4(IN.Position, 1.0f), cbTransform.World);
	result.position = mul(result.position, cbTransform.View);
	result.position = mul(result.position, cbTransform.Projection);
    result.color = float4(IN.Color, 1.0f);

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}