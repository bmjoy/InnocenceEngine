#pragma once
#include "../../Interface/IWindowSystem.h"
#include "MacWindowSystemBridge.h"

class MacWindowSystem : public IWindowSystem
{
public:
	INNO_CLASS_CONCRETE_NON_COPYABLE(MacWindowSystem);

	bool setup(void* hInstance, void* hwnd) override;
	bool initialize() override;
	bool update() override;
	bool terminate() override;

	ObjectStatus getStatus() override;

	IWindowSurface* getWindowSurface() override;
	const std::vector<ButtonState>& getButtonState() override;

	bool sendEvent(uint32_t umsg, uint32_t WParam, int32_t LParam) override;
	bool addEventCallback(WindowEventCallbackFunctor* functor) override;

	void setBridge(MacWindowSystemBridge* bridge);
};
