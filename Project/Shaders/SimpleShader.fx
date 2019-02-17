cbuffer ConstantBuffer:register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
};

struct VS_OUTPUT
{
	float4 pos : POSITON;
};


VS_OUTPUT VS(float4 pos:POSITION)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = mul(pos, World);
	output.pos = mul(pos, View);
	output.pos = mul(pos, Projection);
	return output;
}

float4 PS(VS_OUTPUT input):SV_TARGET
{
	return float4(1.0f, 0.0f,0.0f,0.0f);
}