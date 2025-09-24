/**
 * @file main.c
 *
 * @brief Main entry (for testing purposes)
 *
 * @author J. A. Corbal <jacorbal@gmail.com>
 */

#include <stdbool.h>    /* true */
#include <stdio.h>      /* printf */
#include <stdlib.h>     /* atof */

#include <csvparser.h>


int main(void)
{
    csv_row_td *row;
    csv_parser_td *csvparser =
        csv_parser_init("data/example.csv",
                        CSV_DELIM_COMMA, CSV_HAS_HEADER);

    while ((row = csv_parser_row(csvparser))) {
        const char **row_fields = csv_parser_fields(row);

    	printf("==NEW LINE==\n");
        for (size_t i = 0 ; i < csv_parser_num_fields(row) ; ++i) {
            printf("FIELD: %g\n", atof(row_fields[i]));
        }
		printf("\n");
        csv_parser_destroy_row(row);
    }
    csv_parser_destroy(csvparser);
	
    return 0;
}
