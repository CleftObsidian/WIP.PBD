struct cbTransformation
{
    matrix World;
    matrix View;
    matrix Projection;
    float3 CameraPos;
};
ConstantBuffer<cbTransformation> cbTransform : register(b0);

struct Vertex
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Color : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

PSInput VSMain(Vertex IN)
{
    PSInput result;

	result.position = mul(float4(IN.Position, 1.0f), cbTransform.World);
    result.worldPosition = result.position;
	result.position = mul(result.position, cbTransform.View);
	result.position = mul(result.position, cbTransform.Projection);
    
    result.normal = normalize(mul(float4(IN.Normal, 0.0f), cbTransform.World).xyz);
    
    result.color = float4(IN.Color, 1.0f);

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 directionalLightDirection = normalize(float3(1.0f, 1.0f, -1.0f));
    
    float3 ambient = float3(0.5f, 0.5f, 0.5f);
    
    float3 diffuse = saturate(dot(input.normal, directionalLightDirection));
    
    float3 viewDirection = normalize(cbTransform.CameraPos - input.worldPosition);
    float3 reflectDirection = reflect(-directionalLightDirection, input.normal);
    float shiness = 1.0f;
    float3 specular = pow(saturate(dot(reflectDirection, viewDirection)), shiness);
    
    return float4(ambient + diffuse + specular, 1.0f) * input.color;
}