#include "SohHooks.h"
#include <map>
#include <string>
#include <vector>
#include <stdarg.h>
#include <iostream>

std::map<std::string, std::vector<HookFunc>> listeners;
std::string hookName;
std::map<std::string, void*> initArgs;
std::map<std::string, void*> hookArgs;

/*
#############################
   Module: Hook C++ Handle
#############################
*/

namespace ModInternal {

    void registerHookListener(HookListener listener) {
#if 0
        listeners[listener.hookName].push_back(listener.callback);
#endif
    }

    bool handleHook(std::shared_ptr<HookCall> call) {
#if 0
        std::string hookName = std::string(call->name);
        for (size_t l = 0; l < listeners[hookName].size(); l++) {
            (listeners[hookName][l])(call);
        }
        return call->cancelled;
#else
        return false;
#endif
    }

    void bindHook(std::string name) {
#if 0
        hookName = name;
#endif
    }

    void initBindHook(int length, ...) {
#if 0
        if (length > 0) {
            va_list args;
            va_start(args, length);
            for (int i = 0; i < length; i++) {
                HookParameter currentParam = va_arg(args, struct HookParameter);
                initArgs[currentParam.name] = currentParam.parameter;
            }
            va_end(args);
        }
#endif
    }

    bool callBindHook(int length, ...) {
#if 0
        if (length > 0) {
            va_list args;
            va_start(args, length);
            for (int i = 0; i < length; i++) {
                HookParameter currentParam = va_arg(args, struct HookParameter);
                hookArgs[currentParam.name] = currentParam.parameter;
            }
            va_end(args);
        }

        HookCall call = {
            .name = hookName,
            .baseArgs = initArgs,
            .hookedArgs = hookArgs
		};
        const bool cancelled = handleHook(std::make_shared<HookCall>(call));

        hookName = "";
        initArgs.clear();
        hookArgs.clear();

        return cancelled;
#else
        return false;
#endif
    }
}

/*
#############################
    Module: Hook C Handle
#############################
*/

extern "C" {

    void bind_hook(char* name) {
#if 0
        hookName = std::string(name);
#endif
    }

    void init_hook(int length, ...) {
#if 0
        if (length > 0) {
            va_list args;
            va_start(args, length);
            for (int i = 0; i < length; i++) {
                HookParameter currentParam = va_arg(args, struct HookParameter);
                initArgs[currentParam.name] = currentParam.parameter;
            }
            va_end(args);
        }
#endif
    }

    bool call_hook(int length, ...) {
#if 0
        if (length > 0) {
            va_list args;
            va_start(args, length);
            for (int i = 0; i < length; i++) {
                HookParameter currentParam = va_arg(args, struct HookParameter);
                hookArgs[currentParam.name] = currentParam.parameter;
            }
            va_end(args);
        }

        HookCall call = {
            .name = hookName,
            .baseArgs = initArgs,
            .hookedArgs = hookArgs
        };

        const bool cancelled = ModInternal::handleHook(std::make_shared<HookCall>(call));

        hookName = "";
        initArgs.clear();
        hookArgs.clear();

        return cancelled;
#else
        return false;
#endif
    }

}