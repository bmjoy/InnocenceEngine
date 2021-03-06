#pragma once
#include "../Common/InnoType.h"
#include "../Component/MeshDataComponent.h"
#include "../Component/MaterialDataComponent.h"
#include "../Component/TextureDataComponent.h"

struct alignas(16) PerFrameConstantBuffer
{
	Mat4 p_original;
	Mat4 p_jittered;
	Mat4 v;
	Mat4 v_prev;
	Mat4 p_inv;
	Mat4 v_inv;
	float zNear;
	float zFar;
	float minLogLuminance;
	float maxLogLuminance;
	Vec4 sun_direction;
	Vec4 sun_illuminance;
	Vec4 viewportSize;
	Vec4 posWSNormalizer;
	Vec4 camera_posWS;
	float padding[8];
};

struct alignas(16) CSMConstantBuffer
{
	Mat4 p;
	Mat4 v;
	Vec4 AABBMax;
	Vec4 AABBMin;
	float padding[24];
};

// w component of luminance is attenuationRadius
struct alignas(16) PointLightConstantBuffer
{
	Vec4 pos;
	Vec4 luminance;
	//float attenuationRadius;
};

// w component of luminance is sphereRadius
struct alignas(16) SphereLightConstantBuffer
{
	Vec4 pos;
	Vec4 luminance;
	//float sphereRadius;
};

struct alignas(16) PerObjectConstantBuffer
{
	Mat4 m;
	Mat4 m_prev;
	Mat4 normalMat;
	float UUID;
	float padding[15];
};

struct alignas(16) MaterialConstantBuffer
{
	MaterialAttributes materialAttributes;
	uint32_t textureSlotMask = 0x00000000;
	uint32_t materialType = 0;
	float padding1[6];
	Mat4 padding2;
	Mat4 padding3;
	Mat4 padding4;
};

struct alignas(16) DispatchParamsConstantBuffer
{
	TVec4<uint32_t> numThreadGroups;
	TVec4<uint32_t> numThreads;
};

struct alignas(16) GIConstantBuffer
{
	Mat4 p;
	Mat4 r[6];
	Mat4 t;
	Mat4 p_inv;
	Mat4 v_inv[6];
	Vec4 probeCount;
	Vec4 probeRange;
	Vec4 workload;
	Vec4 irradianceVolumeOffset;
};

struct DrawCallInfo
{
	MeshDataComponent* mesh;
	MaterialDataComponent* material;
	uint32_t meshConstantBufferIndex;
	uint32_t materialConstantBufferIndex;
	bool castSunShadow;
	VisibilityType visibilityType;
};

struct BillboardPassDrawCallInfo
{
	TextureDataComponent* iconTexture;
	uint32_t meshConstantBufferOffset;
	uint32_t instanceCount;
};

struct DebugPassDrawCallInfo
{
	MeshDataComponent* mesh;
};

// Sample point on geometry surface
struct Surfel
{
	Vec4 pos;
	Vec4 normal;
	Vec4 albedo;
	Vec4 MRAT;

	bool operator==(const Surfel &other) const
	{
		return (pos == other.pos);
	}
};

// 1x1x1 m^3 of surfels
using SurfelGrid = Surfel;

// 4x4x4 m^3 of surfels
struct Brick
{
	AABB boundBox;
	uint32_t surfelRangeBegin;
	uint32_t surfelRangeEnd;

	bool operator==(const Brick &other) const
	{
		return (boundBox.m_center == other.boundBox.m_center);
	}
};

struct BrickFactor
{
	float basisWeight;
	uint32_t brickIndex;

	bool operator==(const BrickFactor &other) const
	{
		return (brickIndex == other.brickIndex);
	}
};

struct Probe
{
	Vec4 pos;
	uint32_t brickFactorRange[12];
	float skyVisibility[6];
	uint32_t padding[10];
};

struct ProbeInfo
{
	Vec4 probeCount;
	Vec4 probeRange;
};