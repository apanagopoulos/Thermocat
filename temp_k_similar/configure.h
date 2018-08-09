#ifndef CONFIGURE_H
#define CONFIGURE_H

/* MODIFY THE FOLLOWING VALUES to meet your needs : */

#define FILENAME "temps"
#define K 5
#define PREDICTION_LENGTH 96
#define WAKEUP_MICROSEC 1000000 * 15 * 60    // period of execution (15 mins?)


/* Other values that are Less probable to modify: */

// VALUES_PER_DAY stands for the number of temperatures per day found in the temperature file.
#define VALUES_PER_DAY 96   // 24 hours * 4 temp_per_hour
// each filename up to 30 days
#define NUM_DAYS 30
// temps file must contain a minimum number of values in order for processing to take place
#define MIN_VALUES_REJECT_BELOW VALUES_PER_DAY * 3
// extra margin of 10 days in case file is bigger by mistake:
#define MAX_VALUES_UPON_QUIT_EXECUTION (NUM_DAYS + 10) * VALUES_PER_DAY


#endif