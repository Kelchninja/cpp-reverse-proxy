#pragma once

#if defined(_WIN32)
    #ifdef BACKEND_GATEWAY_EXPORTS
        #define BACKEND_API __declspec(dllexport)
    #else
        #define BACKEND_API __declspec(dllimport)
    #endif
#else
    #define BACKEND_API __attribute__((visibility("default")))
#endif