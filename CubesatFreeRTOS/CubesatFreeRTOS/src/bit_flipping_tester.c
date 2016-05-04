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

#include "RTC_Commands.h"

struct rtc_calendar_time schedule_time; // set a time for the RTC cal alarm

uint16_t lfsr; // Global variable for LFSR RNG

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

  // With 0.02 probability, decide to flip a bit:
  if (coin < 1311u) { 
    uint32_t addr = (rand_LFSR() << 16) | rand_LFSR(); // generate a random address
    
    /* TODO: map this value to the range of valid memory (SRAM)
     * - there are 32KB SRAM according to the datasheet, so likely valid addresses
     *   between 0-32000 (? unsure ?). Plus, need to include registers as possible
     *   places to flip somehow
     */
  }
}

