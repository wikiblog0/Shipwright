#ifdef __WIIU__
#include "WiiUController.h"
#include "GlobalCtx2.h"
#include "Window.h"
#include "ImGuiImpl.h"

#include <padscore/kpad.h>
#include <padscore/wpad.h>

namespace Ship {
    WiiUController::WiiUController(WPADChan chan) : Controller(), chan(chan) {
        connected = false;
        extensionType = (WPADExtensionType) -1;

        controllerName = "Wii U Controller (Disconnected)";
    }

    bool WiiUController::Open() {
		if (WPADProbe(chan, &extensionType) != 0) {
            Close();
            return false;
        }

        connected = true;

        // Create a GUID and name based on extension and channel
        GUID = std::string("WiiU") + GetControllerExtension() + std::to_string((int) chan);
        controllerName = std::string("Wii U ") + GetControllerExtension() + std::string(" ") + std::to_string((int) chan);

        return true;
    }

    void WiiUController::Close() {
        connected = false;
        extensionType = (WPADExtensionType) -1;
    }

    void WiiUController::ReadFromSource(int32_t slot) {
        DeviceProfile& profile = profiles[slot];
        KPADStatus kStatus;
        KPADError kError;
        WPADExtensionType kType;

        if (WPADProbe(chan, &kType) != 0) {
            Close();
            return;
        }

        // Rescan if type changed
        if (kType != extensionType) {
            Window::ControllerApi->ScanPhysicalDevices();
            return;
        }

        KPADReadEx(chan, &kStatus, 1, &kError);
        if (kError != KPAD_ERROR_OK) {
            return;
        }

        SohImGui::controllerInput.has_kpad[chan] = true;
        SohImGui::controllerInput.kpad[chan] = kStatus;

        dwPressedButtons[slot] = 0;
        wStickX = 0;
        wStickY = 0;
        wCamX = 0;
        wCamY = 0;

        if (SohImGui::hasImGuiOverlay || SohImGui::hasKeyboardOverlay) {
            return;
        }

        switch (kType) {
            case WPAD_EXT_PRO_CONTROLLER:
                for (uint32_t i = WPAD_PRO_BUTTON_UP; i <= WPAD_PRO_STICK_R_EMULATION_UP; i <<= 1) {
                    if (profile.Mappings.contains(i)) {
                        // check if the stick is mapped to an analog stick
                        if (i >= WPAD_PRO_STICK_L_EMULATION_LEFT) {
                            float axisX = i >= WPAD_PRO_STICK_R_EMULATION_LEFT ? kStatus.pro.rightStick.x : kStatus.pro.leftStick.x;
                            float axisY = i >= WPAD_PRO_STICK_R_EMULATION_LEFT ? kStatus.pro.rightStick.y : kStatus.pro.leftStick.y;

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

                        if (kStatus.pro.hold & i) {
                            dwPressedButtons[slot] |= profile.Mappings[i];
                        }
                    }
                }
                break;
            case WPAD_EXT_CLASSIC:
            case WPAD_EXT_MPLUS_CLASSIC:
                for (uint32_t i = WPAD_CLASSIC_BUTTON_UP; i <= WPAD_CLASSIC_STICK_R_EMULATION_UP; i <<= 1) {
                    if (profile.Mappings.contains(i)) {
                        // check if the stick is mapped to an analog stick
                        if (i >= WPAD_CLASSIC_STICK_L_EMULATION_LEFT) {
                            float axisX = i >= WPAD_CLASSIC_STICK_R_EMULATION_LEFT ? kStatus.classic.rightStick.x : kStatus.classic.leftStick.x;
                            float axisY = i >= WPAD_CLASSIC_STICK_R_EMULATION_LEFT ? kStatus.classic.rightStick.y : kStatus.classic.leftStick.y;

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

                        if (kStatus.classic.hold & i) {
                            dwPressedButtons[slot] |= profile.Mappings[i];
                        }
                    }
                }
                break;
            case WPAD_EXT_NUNCHUK:
            case WPAD_EXT_MPLUS_NUNCHUK:
            case WPAD_EXT_CORE:
                for (uint32_t i = WPAD_BUTTON_LEFT; i <= WPAD_BUTTON_HOME; i <<= 1) {
                    if (profile.Mappings.contains(i)) {
                        if (kStatus.hold & i) {
                            dwPressedButtons[slot] |= profile.Mappings[i];
                        }
                    }
                }
                wStickX += kStatus.nunchuck.stick.x * 84;
                wStickY += kStatus.nunchuck.stick.y * 84;
                break;
        }
    }

    void WiiUController::WriteToSource(int32_t slot, ControllerCallback* controller) {
        if (profiles[slot].UseRumble) {
            WPADControlMotor(chan, controller->rumble);
        }
    }

    int32_t WiiUController::ReadRawPress() {
        KPADStatus kStatus;
        KPADError kError;

        KPADReadEx(chan, &kStatus, 1, &kError);
        if (kError == KPAD_ERROR_OK) {
            switch (extensionType) {
                case WPAD_EXT_PRO_CONTROLLER:
                    for (uint32_t i = WPAD_PRO_BUTTON_UP; i <= WPAD_PRO_STICK_R_EMULATION_UP; i <<= 1) {
                        if (kStatus.pro.trigger & i) {
                            return i;
                        }
                    }

                    if (kStatus.pro.leftStick.x > 0.7f) {
                        return WPAD_PRO_STICK_L_EMULATION_RIGHT;
                    }
                    if (kStatus.pro.leftStick.x < -0.7f) {
                        return WPAD_PRO_STICK_L_EMULATION_LEFT;
                    }
                    if (kStatus.pro.leftStick.y > 0.7f) {
                        return WPAD_PRO_STICK_L_EMULATION_UP;
                    }
                    if (kStatus.pro.leftStick.y < -0.7f) {
                        return WPAD_PRO_STICK_L_EMULATION_DOWN;
                    }

                    if (kStatus.pro.rightStick.x > 0.7f) {
                        return WPAD_PRO_STICK_R_EMULATION_RIGHT;
                    }
                    if (kStatus.pro.rightStick.x < -0.7f) {
                        return WPAD_PRO_STICK_R_EMULATION_LEFT;
                    }
                    if (kStatus.pro.rightStick.y > 0.7f) {
                        return WPAD_PRO_STICK_R_EMULATION_UP;
                    }
                    if (kStatus.pro.rightStick.y < -0.7f) {
                        return WPAD_PRO_STICK_R_EMULATION_DOWN;
                    }
                    break;
                case WPAD_EXT_CLASSIC:
                case WPAD_EXT_MPLUS_CLASSIC:
                    for (uint32_t i = WPAD_CLASSIC_BUTTON_UP; i <= WPAD_CLASSIC_STICK_R_EMULATION_UP; i <<= 1) {
                        if (kStatus.classic.trigger & i) {
                            return i;
                        }
                    }

                    if (kStatus.classic.leftStick.x > 0.7f) {
                        return WPAD_CLASSIC_STICK_L_EMULATION_RIGHT;
                    }
                    if (kStatus.classic.leftStick.x < -0.7f) {
                        return WPAD_CLASSIC_STICK_L_EMULATION_LEFT;
                    }
                    if (kStatus.classic.leftStick.y > 0.7f) {
                        return WPAD_CLASSIC_STICK_L_EMULATION_UP;
                    }
                    if (kStatus.classic.leftStick.y < -0.7f) {
                        return WPAD_CLASSIC_STICK_L_EMULATION_DOWN;
                    }

                    if (kStatus.classic.rightStick.x > 0.7f) {
                        return WPAD_CLASSIC_STICK_R_EMULATION_RIGHT;
                    }
                    if (kStatus.classic.rightStick.x < -0.7f) {
                        return WPAD_CLASSIC_STICK_R_EMULATION_LEFT;
                    }
                    if (kStatus.classic.rightStick.y > 0.7f) {
                        return WPAD_CLASSIC_STICK_R_EMULATION_UP;
                    }
                    if (kStatus.classic.rightStick.y < -0.7f) {
                        return WPAD_CLASSIC_STICK_R_EMULATION_DOWN;
                    }
                    break;
                case WPAD_EXT_NUNCHUK:
                case WPAD_EXT_MPLUS_NUNCHUK:
                case WPAD_EXT_CORE:
                    for (uint32_t i = WPAD_BUTTON_LEFT; i <= WPAD_BUTTON_HOME; i <<= 1) {
                        if (kStatus.trigger & i) {
                            return i;
                        }
                    }
                    break;
            }
        }

        return -1;
    }

    const char* WiiUController::GetButtonName(int slot, int n64Button) {
        std::map<int32_t, int32_t>& Mappings = profiles[slot].Mappings;
        const auto find = std::find_if(Mappings.begin(), Mappings.end(), [n64Button](const std::pair<int32_t, int32_t>& pair) {
            return pair.second == n64Button;
        });

        if (find == Mappings.end()) return "Unknown";

        uint32_t btn = find->first;

        switch (extensionType) {
            case WPAD_EXT_PRO_CONTROLLER:
                switch (btn) {
                    case WPAD_PRO_BUTTON_A: return "A";
                    case WPAD_PRO_BUTTON_B: return "B";
                    case WPAD_PRO_BUTTON_X: return "X";
                    case WPAD_PRO_BUTTON_Y: return "Y";
                    case WPAD_PRO_BUTTON_LEFT: return "D-pad Left";
                    case WPAD_PRO_BUTTON_RIGHT: return "D-pad Right";
                    case WPAD_PRO_BUTTON_UP: return "D-pad Up";
                    case WPAD_PRO_BUTTON_DOWN: return "D-pad Down";
                    case WPAD_PRO_TRIGGER_ZL: return "ZL";
                    case WPAD_PRO_TRIGGER_ZR: return "ZR";
                    case WPAD_PRO_TRIGGER_L: return "L";
                    case WPAD_PRO_TRIGGER_R: return "R";
                    case WPAD_PRO_BUTTON_PLUS: return "+ (START)";
                    case WPAD_PRO_BUTTON_MINUS: return "- (SELECT)";
                    case WPAD_PRO_BUTTON_STICK_R: return "Stick Button R";
                    case WPAD_PRO_BUTTON_STICK_L: return "Stick Button L";
                    case WPAD_PRO_STICK_R_EMULATION_LEFT: return "Right Stick Left";
                    case WPAD_PRO_STICK_R_EMULATION_RIGHT: return "Right Stick Right";
                    case WPAD_PRO_STICK_R_EMULATION_UP: return "Right Stick Up";
                    case WPAD_PRO_STICK_R_EMULATION_DOWN: return "Right Stick Down";
                    case WPAD_PRO_STICK_L_EMULATION_LEFT: return "Left Stick Left";
                    case WPAD_PRO_STICK_L_EMULATION_RIGHT: return "Left Stick Right";
                    case WPAD_PRO_STICK_L_EMULATION_UP: return "Left Stick Up";
                    case WPAD_PRO_STICK_L_EMULATION_DOWN: return "Left Stick Down";
                }
                break;
            case WPAD_EXT_CLASSIC:
            case WPAD_EXT_MPLUS_CLASSIC:
                switch (btn) {
                    case WPAD_CLASSIC_BUTTON_A: return "A";
                    case WPAD_CLASSIC_BUTTON_B: return "B";
                    case WPAD_CLASSIC_BUTTON_X: return "X";
                    case WPAD_CLASSIC_BUTTON_Y: return "Y";
                    case WPAD_CLASSIC_BUTTON_LEFT: return "D-pad Left";
                    case WPAD_CLASSIC_BUTTON_RIGHT: return "D-pad Right";
                    case WPAD_CLASSIC_BUTTON_UP: return "D-pad Up";
                    case WPAD_CLASSIC_BUTTON_DOWN: return "D-pad Down";
                    case WPAD_CLASSIC_BUTTON_ZL: return "ZL";
                    case WPAD_CLASSIC_BUTTON_ZR: return "ZR";
                    case WPAD_CLASSIC_BUTTON_L: return "L";
                    case WPAD_CLASSIC_BUTTON_R: return "R";
                    case WPAD_CLASSIC_BUTTON_PLUS: return "+ (START)";
                    case WPAD_CLASSIC_BUTTON_MINUS: return "- (SELECT)";
                    case WPAD_CLASSIC_STICK_R_EMULATION_LEFT: return "Right Stick Left";
                    case WPAD_CLASSIC_STICK_R_EMULATION_RIGHT: return "Right Stick Right";
                    case WPAD_CLASSIC_STICK_R_EMULATION_UP: return "Right Stick Up";
                    case WPAD_CLASSIC_STICK_R_EMULATION_DOWN: return "Right Stick Down";
                    case WPAD_CLASSIC_STICK_L_EMULATION_LEFT: return "Left Stick Left";
                    case WPAD_CLASSIC_STICK_L_EMULATION_RIGHT: return "Left Stick Right";
                    case WPAD_CLASSIC_STICK_L_EMULATION_UP: return "Left Stick Up";
                    case WPAD_CLASSIC_STICK_L_EMULATION_DOWN: return "Left Stick Down";
                }
                break;
            case WPAD_EXT_NUNCHUK:
            case WPAD_EXT_MPLUS_NUNCHUK:
            case WPAD_EXT_CORE:
                switch (btn) {
                    case WPAD_BUTTON_A: return "A";
                    case WPAD_BUTTON_B: return "B";
                    case WPAD_BUTTON_1: return "1";
                    case WPAD_BUTTON_2: return "2";
                    case WPAD_BUTTON_LEFT: return "D-pad Left";
                    case WPAD_BUTTON_RIGHT: return "D-pad Right";
                    case WPAD_BUTTON_UP: return "D-pad Up";
                    case WPAD_BUTTON_DOWN: return "D-pad Down";
                    case WPAD_BUTTON_Z: return "Z";
                    case WPAD_BUTTON_C: return "C";
                    case WPAD_BUTTON_PLUS: return "+ (START)";
                    case WPAD_BUTTON_MINUS: return "- (SELECT)";
                }
                break;
        }

        return "Unknown";
    }

    const char* WiiUController::GetControllerName() {
        return controllerName.c_str();
    }

    void WiiUController::CreateDefaultBinding(int32_t slot) {
        DeviceProfile& profile = profiles[slot];
        profile.Mappings.clear();

        profile.UseRumble = true;
        profile.RumbleStrength = 1.0f;
        profile.UseGyro = false;

        switch (extensionType) {
            case WPAD_EXT_PRO_CONTROLLER:
                profile.Mappings[WPAD_PRO_STICK_R_EMULATION_RIGHT] = BTN_CRIGHT;
                profile.Mappings[WPAD_PRO_STICK_R_EMULATION_LEFT] = BTN_CLEFT;
                profile.Mappings[WPAD_PRO_STICK_R_EMULATION_DOWN] = BTN_CDOWN;
                profile.Mappings[WPAD_PRO_STICK_R_EMULATION_UP] = BTN_CUP;
                profile.Mappings[WPAD_PRO_BUTTON_X] = BTN_CRIGHT;
                profile.Mappings[WPAD_PRO_BUTTON_Y] = BTN_CLEFT;
                profile.Mappings[WPAD_PRO_TRIGGER_R] = BTN_CDOWN;
                profile.Mappings[WPAD_PRO_TRIGGER_L] = BTN_CUP;
                profile.Mappings[WPAD_PRO_TRIGGER_ZR] = BTN_R;
                profile.Mappings[WPAD_PRO_BUTTON_MINUS] = BTN_L;
                profile.Mappings[WPAD_PRO_BUTTON_RIGHT] = BTN_DRIGHT;
                profile.Mappings[WPAD_PRO_BUTTON_LEFT] = BTN_DLEFT;
                profile.Mappings[WPAD_PRO_BUTTON_DOWN] = BTN_DDOWN;
                profile.Mappings[WPAD_PRO_BUTTON_UP] = BTN_DUP;
                profile.Mappings[WPAD_PRO_BUTTON_PLUS] = BTN_START;
                profile.Mappings[WPAD_PRO_TRIGGER_ZL] = BTN_Z;
                profile.Mappings[WPAD_PRO_BUTTON_B] = BTN_B;
                profile.Mappings[WPAD_PRO_BUTTON_A] = BTN_A;
                profile.Mappings[WPAD_PRO_STICK_L_EMULATION_RIGHT] = BTN_STICKRIGHT;
                profile.Mappings[WPAD_PRO_STICK_L_EMULATION_LEFT] = BTN_STICKLEFT;
                profile.Mappings[WPAD_PRO_STICK_L_EMULATION_DOWN] = BTN_STICKDOWN;
                profile.Mappings[WPAD_PRO_STICK_L_EMULATION_UP] = BTN_STICKUP;
                break;
            case WPAD_EXT_CLASSIC:
            case WPAD_EXT_MPLUS_CLASSIC:
                profile.Mappings[WPAD_CLASSIC_STICK_R_EMULATION_RIGHT] = BTN_CRIGHT;
                profile.Mappings[WPAD_CLASSIC_STICK_R_EMULATION_LEFT] = BTN_CLEFT;
                profile.Mappings[WPAD_CLASSIC_STICK_R_EMULATION_DOWN] = BTN_CDOWN;
                profile.Mappings[WPAD_CLASSIC_STICK_R_EMULATION_UP] = BTN_CUP;
                profile.Mappings[WPAD_CLASSIC_BUTTON_X] = BTN_CRIGHT;
                profile.Mappings[WPAD_CLASSIC_BUTTON_Y] = BTN_CLEFT;
                profile.Mappings[WPAD_CLASSIC_BUTTON_ZR] = BTN_CDOWN;
                profile.Mappings[WPAD_CLASSIC_BUTTON_ZL] = BTN_CUP;
                profile.Mappings[WPAD_CLASSIC_BUTTON_R] = BTN_R;
                profile.Mappings[WPAD_CLASSIC_BUTTON_MINUS] = BTN_L;
                profile.Mappings[WPAD_CLASSIC_BUTTON_RIGHT] = BTN_DRIGHT;
                profile.Mappings[WPAD_CLASSIC_BUTTON_LEFT] = BTN_DLEFT;
                profile.Mappings[WPAD_CLASSIC_BUTTON_DOWN] = BTN_DDOWN;
                profile.Mappings[WPAD_CLASSIC_BUTTON_UP] = BTN_DUP;
                profile.Mappings[WPAD_CLASSIC_BUTTON_PLUS] = BTN_START;
                profile.Mappings[WPAD_CLASSIC_BUTTON_L] = BTN_Z;
                profile.Mappings[WPAD_CLASSIC_BUTTON_B] = BTN_B;
                profile.Mappings[WPAD_CLASSIC_BUTTON_A] = BTN_A;
                profile.Mappings[WPAD_CLASSIC_STICK_L_EMULATION_RIGHT] = BTN_STICKRIGHT;
                profile.Mappings[WPAD_CLASSIC_STICK_L_EMULATION_LEFT] = BTN_STICKLEFT;
                profile.Mappings[WPAD_CLASSIC_STICK_L_EMULATION_DOWN] = BTN_STICKDOWN;
                profile.Mappings[WPAD_CLASSIC_STICK_L_EMULATION_UP] = BTN_STICKUP;
                break;
            case WPAD_EXT_NUNCHUK:
            case WPAD_EXT_MPLUS_NUNCHUK:
            case WPAD_EXT_CORE:
                profile.Mappings[WPAD_BUTTON_1] = BTN_R;
                profile.Mappings[WPAD_BUTTON_2] = BTN_L;
                profile.Mappings[WPAD_BUTTON_RIGHT] = BTN_DRIGHT;
                profile.Mappings[WPAD_BUTTON_LEFT] = BTN_DLEFT;
                profile.Mappings[WPAD_BUTTON_DOWN] = BTN_DDOWN;
                profile.Mappings[WPAD_BUTTON_UP] = BTN_DUP;
                profile.Mappings[WPAD_BUTTON_PLUS] = BTN_START;
                profile.Mappings[WPAD_BUTTON_MINUS] = BTN_Z;
                profile.Mappings[WPAD_BUTTON_B] = BTN_B;
                profile.Mappings[WPAD_BUTTON_A] = BTN_A;
                break;
        }

        profile.Thresholds[SENSITIVITY] = 16.0f;
    }

    std::string WiiUController::GetControllerExtension() {
        switch (extensionType) {
            case WPAD_EXT_PRO_CONTROLLER:
                return "ProController";
            case WPAD_EXT_CLASSIC:
            case WPAD_EXT_MPLUS_CLASSIC:
                return "ClassicController";
            case WPAD_EXT_NUNCHUK:
            case WPAD_EXT_MPLUS_NUNCHUK:
            case WPAD_EXT_CORE:
                return "WiiRemote";
        }

        return "Controller";
    }
}
#endif
