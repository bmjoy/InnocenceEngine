#pragma once
#include "../IWindowSystem.h"
#include "../../exports/InnoSystem_Export.h"

class DXWindowSystem : INNO_IMPLEMENT IWindowSystem
{
public:
	INNO_CLASS_CONCRETE_NON_COPYABLE(DXWindowSystem);

	bool setup(void* hInstance, void* hPrevInstance, char* pScmdline, int nCmdshow) override;
	bool initialize() override;
	bool update() override;
	bool terminate() override;

	ObjectStatus getStatus() override;
	ButtonStatusMap getButtonStatus() override;

	void swapBuffer() override;
};