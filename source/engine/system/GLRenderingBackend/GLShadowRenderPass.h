#pragma once
#include "../../common/InnoType.h"
#include "../../component/GLRenderPassComponent.h"

INNO_PRIVATE_SCOPE GLShadowRenderPass
{
	void initialize();

	void update();

	GLRenderPassComponent* getGLRPC(unsigned int index);
}