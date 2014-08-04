#pragma once

#if defined(WIN32)
  #include <iostream>
  #include <stdio.h>

  #include <winsock2.h>
  #include <windows.h>
  #include <time.h>
  #include <stdlib.h>

  #if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
    #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
  #else
    #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
  #endif

  typedef struct microTime {
    long tv_sec;
    long tv_usec;
  } microTime;

  typedef struct timezone2 {
    int tz_minuteswest; /* minutes W of Greenwich */
    int tz_dsttime;     /* type of dst correction */
  } timezone2;

  int gettimeofday(microTime *tv, timezone2 *tz);
#else
  #include <sys/time.h>
  #include <unistd.h>

  typedef struct timeval microTime;
#endif

float getTimeDiffNow(microTime mtPast);

microTime getTimeNow();
