#ifndef _MERGETIFF_LIBRARY_SETTINGS
#define _MERGETIFF_LIBRARY_SETTINGS

#include <cstdio>
#include <memory>

//Allow users to override the smart pointer type used by the library
#ifndef MERGETIFF_SMART_POINTER_TYPE
#define MERGETIFF_SMART_POINTER_TYPE std::unique_ptr
#endif

//If a user-specified smart pointer type does not support the `get()` method then an equivalent must be supplied
#ifndef MERGETIFF_SMART_POINTER_GET
#define MERGETIFF_SMART_POINTER_GET(ptr) ptr.get()
#endif

//If a user-specified smart pointer type does not support the `release()` method then an equivalent must be supplied
#ifndef MERGETIFF_SMART_POINTER_RELEASE
#define MERGETIFF_SMART_POINTER_RELEASE(ptr) ptr.release()
#endif

//If a user-specified smart pointer type does not support the `reset()` method then an equivalent must be supplied
#ifndef MERGETIFF_SMART_POINTER_RESET
#define MERGETIFF_SMART_POINTER_RESET(ptr, val) ptr.reset(val)
#endif

//Allow users to specify a logging mechanism for error messages when exception handling is disabled
#ifndef MERGETIFF_ERROR_LOGGER
#define MERGETIFF_ERROR_LOGGER(message) fputs(message, stderr)
#endif

#endif
