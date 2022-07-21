#pragma once
#include "Controller.h"
#include <string>

#include <padscore/wpad.h>

namespace Ship {
    class WiiUController : public Controller {
        public:
            WiiUController(int32_t index, int32_t extensionType);
            bool Open();

            void ReadFromSource(int32_t slot) override;
            void WriteToSource(int32_t slot, ControllerCallback* controller) override;
            bool Connected() const override { return connected; };
            bool CanGyro() const override { return false; }
            bool CanRumble() const override { return true; };

            void ClearRawPress() override {}
            int32_t ReadRawPress() override;

            const char* GetButtonName(int slot, int n64Button) override;
            const char* GetControllerName() override;

        protected:
            void CreateDefaultBinding(int32_t slot) override;

        private:
            std::string GetControllerExtension();
            std::string controllerName;

            bool connected = true;
            WPADChan chan;
            int extensionType = -1;
    };
}
