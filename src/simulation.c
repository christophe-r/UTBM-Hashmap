#include <stdlib.h>
#include <stdio.h>

#include "../include/simulation.h"
#include "../include/myHashMap.h"
#include "../include/csvParser.h"


void runSimulation() {

	HashMap* map = hashmapCreate(7, &hashmapIntHash);

	#ifdef DEBUG
	// Add a record to the HashMap
	hashmapPut(map, "IMEI", "12:00:00", "20815", "UMTS", (double)50000.0, (double)30000.0);
	#endif

	// Parse the CSV File
	parseCSV("./database/dataset.csv", map);

	
	#ifdef DEBUG
	// Display all records
	hashmapDisplay(map);
	#endif


	// Graphical display of the HashMap & statistics
	hashmapDisplayGraph(map);

	
	#ifdef DEBUG
	fprintf(stdout, "\n\n");

	// Get test "from CSV"
	fprintf(stdout, "Get test =>");
	entryDisplay(hashmapGet(map, "00037506853af325ddc63efc535e51ee,16:47:20"), -1);

	// Get test "from code"
	fprintf(stdout, "\nGet test =>");
	entryDisplay(hashmapGet(map, "IMEI,12:00:00"), -1);

	// Count UMTS
	fprintf(stdout, "Number of UMTS in the hashmap: %d\n\n", hashmapCountTechno(map, "UMTS"));

	fprintf(stdout, "\n\nPop function test:\nGet IMEI,12:00:00 => ");
	entryDisplay(hashmapGet(map, "IMEI,12:00:00"), -1);

	fprintf(stdout, "\nPop IMEI,12:00:00 => ");
	Entry* popped_entry = hashmapPop(map, "IMEI,12:00:00");
	entryDisplay(popped_entry, -1);

	fprintf(stdout, "Number of UMTS in the hashmap: %d\n\n", hashmapCountTechno(map, "UMTS"));
	entryFree(popped_entry); // All popped entries must be freed individually/manually
	#endif

	// Free the Hashmap
	hashmapFree(map);

	return;
}

 
