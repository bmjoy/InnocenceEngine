#pragma once
#include "../../Common/InnoType.h"
#include "../../Component/GLRenderPassComponent.h"

namespace GLLightPass
{
	void initialize();

	void update();

	bool resize(uint32_t newSizeX,  uint32_t newSizeY);

	bool reloadShader();

	GLRenderPassComponent* getGLRPC();
}