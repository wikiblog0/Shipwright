#include "WiiUController.h"
#include "GlobalCtx2.h"

namespace Ship {
	WiiUController::WiiUController(int32_t dwControllerNumber) : Controller(dwControllerNumber) {
		LoadBinding();
	}

	WiiUController::~WiiUController() {
		
	}

	void WiiUController::ReadFromSource() {
		wStickX = 0;
		wStickY = 0;
	}

	void WiiUController::WriteToSource(ControllerCallback* controller)
	{

	}

	std::string WiiUController::GetControllerType() {
		return "WIIU GAMEPAD";
	}

	std::string WiiUController::GetConfSection() {
		return GetControllerType() + " CONTROLLER " + std::to_string(GetControllerNumber() + 1);
	}

	std::string WiiUController::GetBindingConfSection() {
		return GetControllerType() + " CONTROLLER BINDING " + std::to_string(GetControllerNumber() + 1);
	}
}
