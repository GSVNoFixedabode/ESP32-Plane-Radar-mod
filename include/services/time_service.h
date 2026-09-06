#pragma once

#include <cstdint>
#include <ctime>

namespace services::time {

/** Initialize SNTP client with NTP pool servers. */
void init();

/** True if NTP time has successfully synced with network. */
bool isTimeSynced();

/** Get current UTC timestamp. Returns 0 if not synced. */
time_t utcTime();

/** True if the sun is currently below the horizon at given GPS coordinates. */
bool isSunBelowHorizon(double lat, double lon);

}  // namespace services::time

