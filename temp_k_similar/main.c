#include "temp_k_similar.h"
#include "xtimer.h"

int main(void) {

    short int ret_value;
    xtimer_ticks32_t last_wakeup = xtimer_now();

    for (;;) {
        /* 1. Read temperatures from the filename file */
        ret_value = read_temperatures_from_file();
        // if file was parsed successfully go on.
        // #todo: if ret_value != 0 the on-demand version should return instead of continuing looping
        if (ret_value == 0) {

            /* 1.1 K shouldn't be >= num_days, if so set k = num_days - 1 */
            k = K < num_days ? K : num_days - 1;

            /* 2. Calculate the euklidean distance of current day with every other day comparing their corresponding
             * temperature values */
            calculate_day_distances();

            /* 3. Find the k most similar days according to their temperature values */
            find_k_most_similar_naive();

            /* 4. Calculate the prediction for the next prediction_length hours based on their corresponding
             *    averages in k selected days */
            calc_prediction();

            /* 5. Print the results */
            print_results();
        }

        /* Sleep until WAKEUP_MICROSEC pass */
        xtimer_periodic_wakeup(&last_wakeup, WAKEUP_MICROSEC);
    }

    return 0;
}
