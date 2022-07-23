#ifdef __WIIU__
#include "WiiUGamepad.h"
#include "GlobalCtx2.h"
#include "ImGuiImpl.h"

#include <vpad/input.h>

namespace Ship {
    WiiUGamepad::WiiUGamepad() : Controller() {
        memset(rumblePattern, 0xFF, sizeof(rumblePattern));

        GUID = "WiiUGamepad";
    }

    void WiiUGamepad::ReadFromSource(int32_t slot) {
        DeviceProfile& profile = profiles[slot];

        VPADStatus vStatus;
        VPADReadError vError;
        VPADRead(VPAD_CHAN_0, &vStatus, 1, &vError);

        if (vError != VPAD_READ_SUCCESS) {
            if (vError == VPAD_READ_INVALID_CONTROLLER) {
                connected = false;
            }
            return;
        } else {
            connected = true;
        }

        SohImGui::controllerInput.has_vpad = true;
        SohImGui::controllerInput.vpad = vStatus;

        dwPressedButtons[slot] = 0;
        wStickX = 0;
        wStickY = 0;
        wCamX = 0;
        wCamY = 0;

        if (SohImGui::hasImGuiOverlay || SohImGui::hasKeyboardOverlay) {
            return;
        }

        for (uint32_t i = VPAD_BUTTON_SYNC; i <= VPAD_STICK_L_EMULATION_LEFT; i <<= 1) {
            if (profile.Mappings.contains(i)) {
                // check if the stick is mapped to an analog stick
                if (i >= VPAD_STICK_R_EMULATION_DOWN) {
                    float axisX = i >= VPAD_STICK_L_EMULATION_DOWN ? vStatus.leftStick.x : vStatus.rightStick.x;
                    float axisY = i >= VPAD_STICK_L_EMULATION_DOWN ? vStatus.leftStick.y : vStatus.rightStick.y;

                    if (profile.Mappings[i] == BTN_STICKRIGHT || profile.Mappings[i] == BTN_STICKLEFT) {
                        wStickX = axisX * 84;
                        continue;
                    } else if (profile.Mappings[i] == BTN_STICKDOWN || profile.Mappings[i] == BTN_STICKUP) {
                        wStickY = axisY * 84;
                        continue;
                    } else if (profile.Mappings[i] == BTN_VSTICKRIGHT || profile.Mappings[i] == BTN_VSTICKLEFT) {
                        wCamX = axisX * 84 * profile.Thresholds[SENSITIVITY];
                        continue;
                    } else if (profile.Mappings[i] == BTN_VSTICKDOWN || profile.Mappings[i] == BTN_VSTICKUP) {
                        wCamY = axisY * 84 * profile.Thresholds[SENSITIVITY];
                        continue;
                    }
                }

                if (vStatus.hold & i) {
                    dwPressedButtons[slot] |= profile.Mappings[i];
                }
            }
        }

        // TODO gyro
        if (profile.UseGyro) {

        }
    }

    void WiiUGamepad::WriteToSource(int32_t slot, ControllerCallback* controller) {
        if (profiles[slot].UseRumble) {
            VPADControlMotor(VPAD_CHAN_0, rumblePattern, controller->rumble ? 120 : 0);
        }
    }

    int32_t WiiUGamepad::ReadRawPress() {
        VPADStatus vStatus;
        VPADReadError vError;
        VPADRead(VPAD_CHAN_0, &vStatus, 1, &vError);

        if (vError == VPAD_READ_SUCCESS) {
            for (uint32_t i = VPAD_BUTTON_SYNC; i <= VPAD_BUTTON_STICK_L; i <<= 1) {
                if (vStatus.hold & i) {
                    return i;
                }
            }
        }

        if (vStatus.leftStick.x > 0.7f) {
            return VPAD_STICK_L_EMULATION_RIGHT;
        }
        if (vStatus.leftStick.x < -0.7f) {
            return VPAD_STICK_L_EMULATION_LEFT;
        }
        if (vStatus.leftStick.y > 0.7f) {
            return VPAD_STICK_L_EMULATION_UP;
        }
        if (vStatus.leftStick.y < -0.7f) {
            return VPAD_STICK_L_EMULATION_DOWN;
        }

        if (vStatus.rightStick.x > 0.7f) {
            return VPAD_STICK_R_EMULATION_RIGHT;
        }
        if (vStatus.rightStick.x < -0.7f) {
            return VPAD_STICK_R_EMULATION_LEFT;
        }
        if (vStatus.rightStick.y > 0.7f) {
            return VPAD_STICK_R_EMULATION_UP;
        }
        if (vStatus.rightStick.y < -0.7f) {
            return VPAD_STICK_R_EMULATION_DOWN;
        }

        return -1;
    }

    const char* WiiUGamepad::GetButtonName(int slot, int n64Button) {
        std::map<int32_t, int32_t>& Mappings = profiles[slot].Mappings;
        const auto find = std::find_if(Mappings.begin(), Mappings.end(), [n64Button](const std::pair<int32_t, int32_t>& pair) {
            return pair.second == n64Button;
        });

        if (find == Mappings.end()) return "Unknown";

        uint32_t btn = find->first;
        switch (btn) {
            case VPAD_BUTTON_A: return "A";
            case VPAD_BUTTON_B: return "B";
            case VPAD_BUTTON_X: return "X";
            case VPAD_BUTTON_Y: return "Y";
            case VPAD_BUTTON_LEFT: return "D-pad Left";
            case VPAD_BUTTON_RIGHT: return "D-pad Right";
            case VPAD_BUTTON_UP: return "D-pad Up";
            case VPAD_BUTTON_DOWN: return "D-pad Down";
            case VPAD_BUTTON_ZL: return "ZL";
            case VPAD_BUTTON_ZR: return "ZR";
            case VPAD_BUTTON_L: return "L";
            case VPAD_BUTTON_R: return "R";
            case VPAD_BUTTON_PLUS: return "+ (START)";
            case VPAD_BUTTON_MINUS: return "- (SELECT)";
            case VPAD_BUTTON_STICK_R: return "Stick Button R";
            case VPAD_BUTTON_STICK_L: return "Stick Button L";
            case VPAD_STICK_R_EMULATION_LEFT: return "Right Stick Left";
            case VPAD_STICK_R_EMULATION_RIGHT: return "Right Stick Right";
            case VPAD_STICK_R_EMULATION_UP: return "Right Stick Up";
            case VPAD_STICK_R_EMULATION_DOWN: return "Right Stick Down";
            case VPAD_STICK_L_EMULATION_LEFT: return "Left Stick Left";
            case VPAD_STICK_L_EMULATION_RIGHT: return "Left Stick Right";
            case VPAD_STICK_L_EMULATION_UP: return "Left Stick Up";
            case VPAD_STICK_L_EMULATION_DOWN: return "Left Stick Down";
        }

        return "Unknown";
    }

    const char* WiiUGamepad::GetControllerName() {
        return "Wii U GamePad";
    }

    void WiiUGamepad::CreateDefaultBinding(int32_t slot) {
        DeviceProfile& profile = profiles[slot];
        profile.Mappings.clear();

        profile.UseRumble = true;
        profile.RumbleStrength = 1.0f;
        profile.UseGyro = false;

        profile.Mappings[VPAD_STICK_R_EMULATION_RIGHT] = BTN_CRIGHT;
        profile.Mappings[VPAD_STICK_R_EMULATION_LEFT] = BTN_CLEFT;
        profile.Mappings[VPAD_STICK_R_EMULATION_DOWN] = BTN_CDOWN;
        profile.Mappings[VPAD_STICK_R_EMULATION_UP] = BTN_CUP;
        profile.Mappings[VPAD_BUTTON_X] = BTN_CRIGHT;
        profile.Mappings[VPAD_BUTTON_Y] = BTN_CLEFT;
        profile.Mappings[VPAD_BUTTON_R] = BTN_CDOWN;
        profile.Mappings[VPAD_BUTTON_L] = BTN_CUP;
        profile.Mappings[VPAD_BUTTON_ZR] = BTN_R;
        profile.Mappings[VPAD_BUTTON_MINUS] = BTN_L;
        profile.Mappings[VPAD_BUTTON_RIGHT] = BTN_DRIGHT;
        profile.Mappings[VPAD_BUTTON_LEFT] = BTN_DLEFT;
        profile.Mappings[VPAD_BUTTON_DOWN] = BTN_DDOWN;
        profile.Mappings[VPAD_BUTTON_UP] = BTN_DUP;
        profile.Mappings[VPAD_BUTTON_PLUS] = BTN_START;
        profile.Mappings[VPAD_BUTTON_ZL] = BTN_Z;
        profile.Mappings[VPAD_BUTTON_B] = BTN_B;
        profile.Mappings[VPAD_BUTTON_A] = BTN_A;
        profile.Mappings[VPAD_STICK_L_EMULATION_RIGHT] = BTN_STICKRIGHT;
        profile.Mappings[VPAD_STICK_L_EMULATION_LEFT] = BTN_STICKLEFT;
        profile.Mappings[VPAD_STICK_L_EMULATION_DOWN] = BTN_STICKDOWN;
        profile.Mappings[VPAD_STICK_L_EMULATION_UP] = BTN_STICKUP;

        profile.Thresholds[SENSITIVITY] = 16.0f;
    }
}
#endif
