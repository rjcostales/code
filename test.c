l//
//  main.c
//  CF.TimingsHighResC01
//
//  Created by Alan Sampson on 2/14/21.
//
//  MARK: - Reference.
//  @see: https://www.manpagez.com/man/3/clock_gettime/
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#define MACRO_NAME(x) #x

typedef struct timespec timespec;

inline static
timespec diff(timespec start, timespec end);
int driver(int argc, const char * argv[]);

void ff0(void);

/*
 *  MARK: main()
 */
int main(int argc, const char * argv[]) {
  // insert code here...
  printf("CF.TimingsHighResC01\n");

  ff0();

  puts("\nTest complete");

  return EXIT_SUCCESS;
}

/*
 *  MARK: ff0()
 *  CLOCK_REALTIME     the system's real time (i.e. wall time) clock, expressed
 *                     as the amount of time since the Epoch.  This is the same
 *                     as the value returned by gettimeofday(2).
 *
 *  CLOCK_MONOTONIC    clock that increments monotonically, tracking the time
 *                     since an arbitrary point, and will continue to increment
 *                     while the system is asleep.
 *
 *  CLOCK_MONOTONIC_RAW
 *                     clock that increments monotonically, tracking the time
 *                     since an arbitrary point like CLOCK_MONOTONIC.  However,
 *                     this clock is unaffected by frequency or time adjustments.
 *                     It should not be compared to other system time sources.
 *
 *  CLOCK_MONOTONIC_RAW_APPROX
 *                     like CLOCK_MONOTONIC_RAW, but reads a value cached by
 *                     the system at context switch.  This can be read
 *                     faster, but at a loss of accuracy as it may return
 *                     values that are milliseconds old.
 *
 *  CLOCK_UPTIME_RAW   clock that increments monotonically, in the same manner as
 *                     CLOCK_MONOTONIC_RAW, but that does not increment while the
 *                     system is asleep.  The returned value is identical to the
 *                     result of mach_absolute_time() after the appropriate
 *                     mach_timebase conversion is applied.
 *
 *  CLOCK_UPTIME_RAW_APPROX
 *                     like CLOCK_UPTIME_RAW, but reads a value cached by the
 *                     system at context switch.  This can be read faster,
 *                     but at a loss of accuracy as it may return values that
 *                     are milliseconds old.
 *
 *  CLOCK_PROCESS_CPUTIME_ID
 *                     clock that tracks the amount of CPU (in user- or kernel-mode)
 *                     used by the calling process.
 *
 *  CLOCK_THREAD_CPUTIME_ID
 *                     clock that tracks the amount of CPU (in user- or kernel-mode)
 *                     used by the calling thread.
 *
 */
void ff0(void) {

//  __attribute__((__unused__))
  size_t const max_getmain = 1024 * 100;  // dynamic allocation size
  size_t const test_loops = 1000000;  // choose a value that makes sense

#define NAME_SZ 32
  struct timebob {
    char name[NAME_SZ];
    clockid_t clkid;
    bool enable;
  };
  typedef struct timebob timebob;

  timebob timebobs[] = {
    { MACRO_NAME(CLOCK_REALTIME),             CLOCK_REALTIME,             true,  },
    { MACRO_NAME(CLOCK_MONOTONIC),            CLOCK_MONOTONIC,            true,  },
    { MACRO_NAME(CLOCK_MONOTONIC_RAW),        CLOCK_MONOTONIC_RAW,        true,  },
#ifdef CLOCK_MONOTONIC_RAW_APPROX
    { MACRO_NAME(CLOCK_MONOTONIC_RAW_APPROX), CLOCK_MONOTONIC_RAW_APPROX, false, },
#endif
#ifdef CLOCK_UPTIME_RAW
    { MACRO_NAME(CLOCK_UPTIME_RAW),           CLOCK_UPTIME_RAW,           true,  },
#endif
#ifdef CLOCK_UPTIME_RAW_APPROX
    { MACRO_NAME(CLOCK_UPTIME_RAW_APPROX),    CLOCK_UPTIME_RAW_APPROX,    false, },
#endif
    { MACRO_NAME(CLOCK_PROCESS_CPUTIME_ID),   CLOCK_PROCESS_CPUTIME_ID,   true,  },
    { MACRO_NAME(CLOCK_THREAD_CPUTIME_ID),    CLOCK_THREAD_CPUTIME_ID,    true,  },
  };
  size_t timebobs_e = sizeof(timebobs) / sizeof(*timebobs);

  for (size_t tb = 0; tb < timebobs_e; ++tb) {
    timebob bobby = timebobs[tb];
    printf("\n%32s : %2d : %s\n",
           bobby.name, bobby.clkid, (bobby.enable ? "true" : "false"));
    if (bobby.enable) {

      timespec time_bgn;
      timespec time_end;
      timespec time_dif;

      uint8_t * target;
      uint8_t x80;
      uint8_t x40;
      target = NULL;

      //  do it once to flush prime the pump
      target = malloc(max_getmain);
      memset(target, 0x80, max_getmain);
      free(target);
      target = NULL;

      //  TODO: test malloc
      clock_gettime(bobby.clkid, &time_bgn);
      for (size_t i_ = 0; i_ < test_loops; ++i_) {
        if (target != NULL) { free(target); }
        target = malloc(max_getmain);
      }
      clock_gettime(bobby.clkid, &time_end);
      time_dif = diff(time_bgn, time_end);
      printf("%20s %5ld:%09ld\n", "malloc", time_dif.tv_sec, time_dif.tv_nsec);

      // hoefully this stops the optimizer from bypassing the code
      memset(target, 0x80, max_getmain);
      x80 = *target + 1024 / 2;
      memset(target, 0x40, max_getmain);
      x40 = *target + 1024 / 2;
      printf("%p %p %02x %02x\n", target, target + 1024 / 2, x80, x40);
      free(target);
      target = NULL;

      //  TODO: test malloc,memset
      clock_gettime(bobby.clkid, &time_bgn);
      for (size_t i_ = 0; i_ < test_loops; ++i_) {
        if (target != NULL) { free(target); }
        target = malloc(max_getmain);
        memset(target, 0x80, max_getmain);
      }
      clock_gettime(bobby.clkid, &time_end);
      time_dif = diff(time_bgn, time_end);
      printf("%20s %5ld:%09ld\n", "malloc,memset", time_dif.tv_sec, time_dif.tv_nsec);

      x80 = *target + 1024 / 2;
      memset(target, 0x40, max_getmain);
      x40 = *target + 1024 / 2;
      printf("%p %p %02x %02x\n", target, target + 1024 / 2, x80, x40);
      free(target);
      target = NULL;

      //  TODO: test calloc
      clock_gettime(bobby.clkid, &time_bgn);
      for (size_t i_ = 0; i_ < test_loops; ++i_) {
        if (target != NULL) { free(target); }
        target = calloc(sizeof(char), max_getmain);
        memset(target, 0x80, max_getmain);
      }
      clock_gettime(bobby.clkid, &time_end);
      time_dif = diff(time_bgn, time_end);
      printf("%20s %5ld:%09ld\n", "calloc", time_dif.tv_sec, time_dif.tv_nsec);

      // hoefully this stops the optimizer from bypassing the code
      memset(target, 0x80, max_getmain);
      x80 = *target + 1024 / 2;
      memset(target, 0x40, max_getmain);
      x40 = *target + 1024 / 2;
      printf("%p %p %02x %02x\n", target, target + 1024 / 2, x80, x40);
      free(target);
      target = NULL;
    }
  }

  return;
}

/*
 *  MARK: diff()
 */
inline
static
timespec diff(timespec start, timespec end) {
  timespec temp;

  if ((end.tv_nsec - start.tv_nsec) < 0) {
    temp.tv_sec = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  }
  else {
    temp.tv_sec = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }

  return temp;
}
