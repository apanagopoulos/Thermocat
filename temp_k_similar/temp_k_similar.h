//todo: (major) how are we going to return the result ?!?!
//todo: (minor) optimize making function inline, then static, etc

#include "configure.h"
#include <stdio.h>

// ensure that PREDICTION_LENGTH is not set bigger than VALUES_PER_DAY in configure.h:
#define MIN(a,b) (((a)<(b))?(a):(b))
#define PRED_LENGTH MIN(PREDICTION_LENGTH, VALUES_PER_DAY)

typedef unsigned short int usi;

typedef struct day_distance {
    usi is_k;        // boolean flag (0/1) to indicate if this has already been marked as a k-day
    float distance;               // euklidean temperature diff from the last day
} d;

// *** do NOT change the dimensions of the arrays! ***

/* temp_per_day contains the temperatures read from the file */
float temp_per_day[NUM_DAYS][VALUES_PER_DAY];
/* days holds the distance of each day from last day */
d days[NUM_DAYS - 1];
/* k_days holds indexes (identifiers) of the days selected */
usi k_days[NUM_DAYS - 1];  // TODO: if we are absolutely sure of fixed k then we can make this to [k] size
/* num_days represents how many days of complete data (== VALUES_PER_DAY)  were read */
usi num_days;
/* k represents the actual k which is equal to K, with the only exception being K >= num_days */
usi k;
/* the temperature prediction of the next hours (given as cmdline argument): */
float prediction[VALUES_PER_DAY];     // upper limit is VALUES_PER_DAY


//void debug(void) {
//    int i, j;
//
//    printf("\n----- DEBUG INFO -------\n*** Printing temperatures ***\n");
//    for(i=0; i < num_days; i++) {
//        for (j = 0; j < VALUES_PER_DAY; j++)
//            printf("temp[%d][%d] = %f, ", i, j, temp_per_day[i][j]);
//        printf("\n");
//    }
//
//    printf("*** Printing days distance table ***\n");
//    for(i=0; i < num_days - 1; i++)
//        printf("distances[%d] = %f, is_k (1/0): %d\n", i, days[i].distance, days[i].is_k);
//
//    printf("*** Printing k days table ***\n");
//    for(i=0; i < num_days - 1; i++)
//        printf("k-days[%d] = %d\n", i, k_days[i] + 1);
//    printf("--- END OF DEBUG INFO ----\n\n");
//}

void print_results(void) {
    usi i;

    printf("*** Most similar days: ***\n");
    for(i=0; i<k; i++)
        printf("day=%d, ", k_days[i] + 1);
    printf("\n");

    printf("=== Prediction for the next %hu values: ===\n[", PRED_LENGTH);
    for(i=0; i<PRED_LENGTH; i++)
        printf("%f, ", prediction[i]);
    printf("]\n");
}

// todo: make Q_rsqrt & Q_fabs inline, check compiler flags to determine C standard defined
/* highly efficient reverse sqrt function taken from:
 * https://github.com/id-Software/Quake-III-Arena/blob/master/code/game/q_math.c#L554-564 */
float Q_rsqrt( float number )
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;						// evil floating point bit level hacking
    i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
    y  = * ( float * ) &i;
    y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
    y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed in order to gain some nanosecs

    return y;
}

/* Taken also from Quake III like the Q_rsqrt() function above */
float Q_fabs(float f) {
    int tmp = * ( int * ) &f;
    tmp &= 0x7FFFFFFF;
    return * ( float * ) &tmp;
}

/* Prediction is the average of the corresponding temperatures of each next day after each k day */
void calc_prediction(void) {
    usi i, j;
    float sum;

    for(i = 0; i < PRED_LENGTH; i++) {
        sum = 0;
        for (j = 0; j < k; j++) {
            sum += temp_per_day[k_days[j] + 1][i];
        }
        prediction[i] = sum / k;
    }
}

void find_k_most_similar_naive(void) {
    usi i, j, min_idx=0;
    float min_distance;
    usi num_days_to_be_sorted = (usi) (num_days - 1);

    for (j=0; j<k; j++) {
        min_distance = 99999999;
        for (i = 0; i < num_days_to_be_sorted; i++) {
            /* If we find a smaller distance that hasn't already been marked as used, use it */
            if (days[i].distance < min_distance && days[i].is_k == 0) {
                //printf("found min with index %d\n", i);
                min_distance = days[i].distance;
                min_idx = i;
            }
        }
        //printf("j=%d, min_idx=%d\n", j, min_idx);
        days[min_idx].is_k = 1;     // mark as true, congratulations you have been selected
        k_days[j] = min_idx;
    }

    for (j=k; j<NUM_DAYS - 2; j++)
        k_days[j] = 65535;
}

/* calculate euklidean distance of last day with every other day */
void calculate_day_distances(void) {
    usi i, j;
    float sum, diff_tmp;
    float *cur_day = temp_per_day[num_days - 1];   // last day is today

    // i<num_days-1 => don't compare cur_day to itself
    for(i=0; i < num_days - 1; i++){
        sum = 0;
        for(j=0; j < VALUES_PER_DAY; j++) {
            diff_tmp = cur_day[j] - temp_per_day[i][j];
            sum += diff_tmp * diff_tmp;
        }
        days[i].distance = sum * Q_fabs(Q_rsqrt(sum));   // square root using the inverse function
        //days[i].day_init_idx = i;   // day identifier
        days[i].is_k = 0;     // initialize with false (hasnt been found as a k-day yet)
        //printf("sum %f, val %f", sum, days[i].distance);
    }

    //mark the current day as the last/not_used cell in the array to easily identify iteration mistakes
    days[i].distance = -2017;
    //days[i].day_init_idx = i;
    days[i].is_k = 0;       // initialize with false (hasnt been found as a k-day yet)
}

/* Open <filename>, and store:
 * -- temperature values in <temp_per_day> global array
 * -- number of days in <num_days>
 * -- how many temperature values last day had in <values_per_day>  
 */
short int read_temperatures_from_file(void) {
    usi temp_counter, offset, i, day_idx, temp_idx;
    float temp;
    FILE * fin;
    //temporary array to actually count the number of values found
    float buffer[MAX_VALUES_UPON_QUIT_EXECUTION];

    fin = fopen(FILENAME, "r");
    if (fin == 0) {
        fprintf(stdout, "ERROR: Failed to open %s\n", FILENAME);
        return -1;   // failed
    }

    temp_counter = 0;
    while(fscanf(fin, "%f\n", &temp) > 0) {
        if (temp_counter >= MAX_VALUES_UPON_QUIT_EXECUTION){
            fprintf(stderr, "%s values exceed upper limit of %d\n", FILENAME, MAX_VALUES_UPON_QUIT_EXECUTION);
            fclose(fin);
            return -1;
        }

        buffer[temp_counter++] = temp;
    }

    if (fclose(fin) != 0)
        fprintf(stdout, "WARNING: Error closing %s\n", FILENAME);

    if (temp_counter < MIN_VALUES_REJECT_BELOW){
        fprintf(stderr, "ERROR: Insufficient data to process: %d < %d\n", temp_counter, MIN_VALUES_REJECT_BELOW);
        return -1;
    }

    num_days = (usi) temp_counter / VALUES_PER_DAY;    // global variable
    offset = (usi) temp_counter % VALUES_PER_DAY;      // temporary, first <offset> values to discard
    for(i=offset; i<temp_counter; i++) {
        day_idx = (i - offset) / VALUES_PER_DAY;
        temp_idx = (i - offset) % VALUES_PER_DAY;
        temp_per_day[day_idx][temp_idx] = buffer[i];
    }

    return 0;   // success
}
