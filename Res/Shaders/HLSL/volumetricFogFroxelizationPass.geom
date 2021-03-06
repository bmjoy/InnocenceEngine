// shadertype=hlsl
#include "common/common.hlsl"

struct GeometryInputType
{
	float4 posCS : SV_POSITION;
	float3 posWS : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};

struct PixelInputType
{
	float4 posCS : SV_POSITION;
	float4 posCS_orig : POSITION;
	float4 AABB : AABB;
};

int CalculateAxis(float4 pos[3])
{
	float3 p1 = pos[1].xyz - pos[0].xyz;
	float3 p2 = pos[2].xyz - pos[0].xyz;
	float3 faceNormal = cross(p1, p2);

	float nDX = abs(faceNormal.x);
	float nDY = abs(faceNormal.y);
	float nDZ = abs(faceNormal.z);

	if (nDX > nDY && nDX > nDZ)
	{
		return 0;
	}
	else if (nDY > nDX && nDY > nDZ)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

float4 getAABB(float4 pos[3], float2 pixelDiagonal)
{
	float4 aabb;

	aabb.xy = min(pos[2].xy, min(pos[1].xy, pos[0].xy));
	aabb.zw = max(pos[2].xy, max(pos[1].xy, pos[0].xy));

	// enlarge by half-pixel
	aabb.xy -= pixelDiagonal;
	aabb.zw += pixelDiagonal;

	return aabb;
}

[maxvertexcount(3)]
void main(triangle GeometryInputType input[3], inout TriangleStream<PixelInputType> outStream)
{
	PixelInputType output = (PixelInputType)0;

	float4 pos[3];

	pos[0] = input[0].posCS;
	pos[1] = input[1].posCS;
	pos[2] = input[2].posCS;

	int selectedIndex = CalculateAxis(pos);

	// project along the dominant axis
	[unroll(3)]
	for (int i = 0; i < 3; i++)
	{
		pos[i] /= pos[i].w;

		[flatten]
		if (selectedIndex == 0)
		{
			pos[i] = float4(pos[i].y, pos[i].z, pos[i].x, 1);
		}
		else if (selectedIndex == 1)
		{
			pos[i] = float4(pos[i].z, pos[i].x, pos[i].y, 1);
		}
		else
		{
			pos[i] = float4(pos[i].x, pos[i].y, pos[i].z, 1);
		}
	}

	[unroll(3)]
	for (int j = 0; j < 3; ++j)
	{
		output.posCS = pos[j];
		output.posCS_orig = input[j].posCS;
		output.AABB = getAABB(pos, float2(1.0 / 64.0, 1.0 / 64.0));

		outStream.Append(output);
	}

	outStream.RestartStrip();
}