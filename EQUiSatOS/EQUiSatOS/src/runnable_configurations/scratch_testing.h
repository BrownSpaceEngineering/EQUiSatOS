/*
 * scratch_testing.h
 *
 * Created: 9/18/2016 4:11:11 PM
 *  Author: Daniel
 */ 


#ifndef SCRATCH_TESTING_H_
#define SCRATCH_TESTING_H_

void runit(void);
void radioTest(void);
void rsTest(void);
static void byte_err (int err, int loc, unsigned char *dst);
static void byte_erasure (int loc, unsigned char dst[], int cwsize, int erasures[]);

#endif /* SCRATCH_TESTING_H_ */