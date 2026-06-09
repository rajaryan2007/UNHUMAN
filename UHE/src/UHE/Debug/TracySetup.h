#pragma once

#include <tracy/Tracy.hpp>

#if UHE_PROFILE
#define UHE_PROFILE_SCOPE(name) ZoneScopedN(name)
#define UHE_PROFILE_FUNCTION() ZoneScoped
#define UHE_PROFILE_FRAMEMARK FrameMark
#define UHE_GPU_ZONE(name) ZoneScopedN(name)
#else
#define UHE_PROFILE_SCOPE(name)
#define UHE_PROFILE_FUNCTION()
#define UHE_PROFILE_FRAMEMARK
#define UHE_GPU_ZONE(name)
#endif

namespace UHE
{

}
