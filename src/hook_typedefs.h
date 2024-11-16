#ifndef HOOK_TYPEDEFS_H
#define HOOK_TYPEDEFS_H

#include <QtCore/QObject>

#include "utils/data_structs.h"

typedef void (*ScreenUpdateFunc)(QObject*, point start, point end, int, int, int);

#ifdef RM_VERSION_3_14_4_0
#define HOOK_ADDRESS_DEFINED
inline const auto create_ep_fb_addr = reinterpret_cast<void*>(0x0090fed0);
inline const auto screen_update_func_addr = reinterpret_cast<void*>(0x0090b220);
#endif

#ifdef RM_VERSION_3_15_4_2
#define HOOK_ADDRESS_DEFINED
inline const auto screen_update_func_addr = reinterpret_cast<void*>(0x00794330);
inline const auto create_ep_fb_addr = reinterpret_cast<void*>(0x00792ac0);
#endif

#ifndef HOOK_ADDRESS_DEFINED
#error "Hook addresses not defined for this version"
#endif

#endif // HOOK_TYPEDEFS_H
