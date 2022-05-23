#include <std_include.hpp>
#include "../steam.hpp"

namespace steam
{
	bool controller::Init()
	{
		return true;
	}

	bool controller::Shutdown()
	{
		return true;
	}

	void controller::RunFrame()
	{
	}

	int controller::GetConnectedControllers(ControllerHandle_t* handlesOut)
	{
		return 0;
	}

	bool controller::ShowBindingPanel(ControllerHandle_t controllerHandle)
	{
		return false;
	}

	uint64_t controller::GetActionSetHandle(const char* pszActionSetName)
	{
		return 0;
	}

	void controller::ActivateActionSet(ControllerHandle_t controllerHandle, uint64_t actionSetHandle)
	{
	}

	uint64_t controller::GetCurrentActionSet(ControllerHandle_t controllerHandle)
	{
		return 0;
	}

	uint64_t controller::GetDigitalActionHandle(const char* pszActionName)
	{
		return 0;
	}

	uint64_t controller::GetDigitalActionData(ControllerHandle_t controllerHandle, uint64_t digitalActionHandle)
	{
		return 0;
	}

	int controller::GetDigitalActionOrigins(ControllerHandle_t controllerHandle, uint64_t actionSetHandle,
	                                        uint64_t digitalActionHandle, uint64_t* originsOut)
	{
		return 0;
	}

	uint64_t controller::GetAnalogActionHandle(const char* pszActionName)
	{
		return 0;
	}

	uint64_t controller::GetAnalogActionData(ControllerHandle_t controllerHandle, uint64_t analogActionHandle)
	{
		return 0;
	}

	int controller::GetAnalogActionOrigins(ControllerHandle_t controllerHandle, uint64_t actionSetHandle,
	                                       uint64_t analogActionHandle, uint64_t* originsOut)
	{
		return 0;
	}

	void controller::StopAnalogActionMomentum(ControllerHandle_t controllerHandle, uint64_t eAction)
	{
	}

	void controller::TriggerHapticPulse(ControllerHandle_t controllerHandle, uint64_t eTargetPad,
	                                    unsigned short usDurationMicroSec)
	{
	}
}
