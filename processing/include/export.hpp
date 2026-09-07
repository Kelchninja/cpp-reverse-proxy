#pragma once

#if defined(_WIN32)
    #ifdef PROC_EXPORTS
        #define PROCLIB_API __declspec(dllexport)
    #else
        #define PROCLIB_API __declspec(dllimport)
    #endif
#else
    #define PROCLIB_API __attribute__((visibility("default")))
#endif