#ifndef MACROS_DEFS
#define MACROS_DEFS

// API macros allows export some symbols from the library
// It has different value for microsoft and gnu compiler
// Other compilers aren't supported
#ifdef _MSC_VER                 // Microsoft compiler
#ifdef FDP_LIBRARY_EXPORTS
#define API _declspec(dllexport)
#else
#define API _declspec(dllimport)
#endif
#elif __GNUC__                  // GCC compiler
#define API __attribute__ ((visibility ("default")))
#else
#error UNKNOWN COMPILER TYPE (ONLY MICROSOFT AND GCC SUPPORTED) // error
#endif

// Extern "C" macros prevents name mangling
#ifdef __cplusplus
#define EXTERN_C extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C
#define EXTERN_C_END
#endif

#define SIZE_OF_IEFACENET_V1 512

#endif