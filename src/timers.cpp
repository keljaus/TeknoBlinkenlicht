#include "timers.h"

#if defined(WIN32)
  /**
   * Windows compatible version of posix gettimeofday().
   */
  int gettimeofday(microTime *tv, timezone2 *tz) {
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag;
 
    if (NULL != tv) {
      GetSystemTimeAsFileTime(&ft);
 
      tmpres |= ft.dwHighDateTime;
      tmpres <<= 32;
      tmpres |= ft.dwLowDateTime;
 
      /*converting file time to unix epoch*/
      tmpres -= DELTA_EPOCH_IN_MICROSECS; 
      tmpres /= 10;  /*convert into microseconds*/
      tv->tv_sec = (long)(tmpres / 1000000UL);
      tv->tv_usec = (long)(tmpres % 1000000UL);
    }
 
    if (NULL != tz) {
      if (!tzflag) {
        _tzset();
        tzflag++;
      }

      tz->tz_minuteswest = _timezone / 60;
      tz->tz_dsttime = _daylight;
    }
 
    return 0;
  }
#endif

/**
 * Get microtime difference between two timeval structs.
 */
float getTimeDiffNow(microTime mtPast) {
  microTime mtNow;
  gettimeofday(&mtNow, NULL);

  long elapsed = mtNow.tv_sec - mtPast.tv_sec;
  long uelapsed = mtNow.tv_usec - mtPast.tv_usec;

  return (elapsed + uelapsed / 1000000.0);
}

/**
 * Get current microtime.
 */
microTime getTimeNow() {
  microTime mtNow;
  gettimeofday(&mtNow, NULL);
  return mtNow;
}
