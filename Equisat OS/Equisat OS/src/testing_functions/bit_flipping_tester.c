/*
 * Bit Flipping Tester:
 *
 * Timed interrupt - should happen randomly in code - that decides if a bit
 * should be flipped. Then, pick a random memory location, including CPU and
 * peripheral registers. Flip the bit by XOR'ing its value. Aiming to make 10x
 * worse than expected in an actual LEO environment.
 *
 * Ideally, these bit flips should be picked up by the watchdogs if things fail
 * terribly.
 *
 * Author: Arun Drelich
 */

#include "../processor_drivers/RTC_Commands.h"

volatile struct rtc_calendar_time schedule_time; // set a time for the RTC cal alarm

volatile uint16_t lfsr; // Global variable for LFSR RNG

/* Seed the random number generator */
void srand_LFSR(uint16_t seed) {
  lfsr = seed;
}

/* Generate a random 16-bit integer using a Linear-Feedback Shift Register.
   The LFSR should be initialised (seeded) with a value first.
 */
uint16_t rand_LFSR(void) {
  uint16_t bit;                    /* Must be 16bit to allow bit<<15 later in the code */
  unsigned period = 0;
  /* taps: 16 14 13 11; feedback polynomial: x^16 + x^14 + x^13 + x^11 + 1 */
  for (int j = 0; j < 16; j++) {
    bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
    lfsr = (lfsr >> 1) | (bit << 15); // lfsr is a global variable
    ++period;
  }

  return lfsr;
}

/* Setup the RTC calendar alarm and bind to a callback. Call this function from
   e.g. main() in order to enable the bit flip test.
 */
void init_bitFlip_test(void) {
  
  rtc_calendar_get_time_defaults(&schedule_time);
  rtc_calendar_get_time(&schedule_time); // Get the current time
  
  /* Configure the alarm to match on the second of the timestamp,
     i.e. match every minute */
  configure_rtc_calendar(&schedule_time, RTC_CALENDAR_ALARM_MASK_SEC);
  configure_rtc_callback(bitFlip_callbackFunc);

  srand_LFSR(1); // TODO: Figure out a way to randomly seed this
}

/* The bit flip function: Randomly decide if a bit should be flipped, and if so,
   randomly select a memory address or registry location and randomly flip a bit
   at that location.
 */
void bitFlip_callbackFunc(void) {
  uint16_t coin = rand_LFSR();

  // With 0.2 probability, decide to *possibly* flip a bit:
  if (coin < 13108u) {
    // The random number will be a valid memory address with only 0.5 probability:
    coin = rand_LFSR();
    int addr = ((uint32_t)coin << 16) >> 16; // generate a random address between 0 and 2^15 - 1

    /* If the addr is nonnegative, we treat its value as a randomly-selected byte
     * of memory in which we flip a bit. This happens with probability 0.1
     */
    if (addr >= 0) { // select from SRAM (p=0.1)
      uint8_t *ptr = (uint8_t *)addr; // cast address to a pointer to 8-bit uint

      /* We know that 8 divides 2^15, so (coin % 8) remains uniformly distributed.
       * This selects a bit to flip uniformly at random */
      uint8_t flipmask = 1 << (coin % 8);

      *ptr ^= flipmask; // XOR to flip that bit

    }
  }
}

