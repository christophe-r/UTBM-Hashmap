#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <sys/types.h>

#include "../include/csvParser.h"
#include "../include/myHashMap.h"


void parseCSV(char *fileName, HashMap *map){
	#ifdef DEBUG
	fprintf(stdout, "Parsing CSV file...\n");
	#endif

	FILE *file;
	file = fopen(fileName, "r");

	if( file == NULL ){
		perror(fileName);
		return;
	}


	char buffer[256];
	memset(buffer, '\0', sizeof(buffer)); // Reset buffer

	int count;
	char *token;
	char *delim = ",";
	while( fgets(buffer, 256, file) != NULL ){

			token = strtok(buffer, delim);

			count = 0;
			char *sub_strings[6];
			while( token != NULL && count < 6 ){ // count < 6 for only 6 values.

				sub_strings[count] = token;

				token = strtok(NULL, delim);
				count++;
			}

			if( count == 6 ){ // This avoids to put an incomplete entry.
				hashmapPut(map, sub_strings[0], sub_strings[1], sub_strings[2], sub_strings[3], (double)atof(sub_strings[4]), (double)atof(sub_strings[5]));
			}

	}

	fclose(file);

	return;
}


