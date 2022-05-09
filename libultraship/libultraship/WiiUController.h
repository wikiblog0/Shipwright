#pragma once
#include "Controller.h"
#include <string>

namespace Ship {
    class WiiUController : public Controller {
        public:
            WiiUController(int32_t dwControllerNumber);
            ~WiiUController();

            void ReadFromSource();
            void WriteToSource(ControllerCallback* controller);
            bool Connected() const { return connected; };
            bool CanRumble() const { return true; };

            bool HasPadConf() const { return false; };
            std::optional<std::string> GetPadConfSection() { return {}; };

        protected:
            void CreateDefaultBinding();
            std::string GetControllerExtension();
            std::string GetControllerType();
            std::string GetConfSection();
            std::string GetBindingConfSection();

        private:
            bool connected = true;
            int extensionType = -1;
    };
}
