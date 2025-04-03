#define public __attribute__((visibility("default")))
#define private __attribute__((visibility("hidden")))

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef long long sint64;
typedef int sint32;
typedef short sint16;
typedef char sint8;

typedef char bool;
#define true 1
#define false 0

typedef void* function_pointer;
typedef void* structure_pointer;
#define none 0

#ifdef platform_32bit
#define BYTES_COUNT 4
#define BITS_COUNT 32
typedef uint32 size;
typedef uint32 uint;
typedef sint32 sint;
#elifdef platform_64bit
#define BYTES_COUNT 8
#define BITS_COUNT 64
typedef uint64 size;
typedef uint64 uint;
typedef sint64 sint;
#else
#error "Specify correct build's bitness."
#endif

typedef sint32 steam_pipe;
typedef sint32 steam_user;
typedef uint64 steam_call_result_handle;
typedef void(*steam_warning_message_hook)(sint32, char*);

#define __macro_to_string(m) #m
#define macro_to_string(m) __macro_to_string(m)