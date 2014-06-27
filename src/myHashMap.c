#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include <sys/ioctl.h>
#include <unistd.h>

#include "../include/myHashMap.h"

#define LOAD_FACTOR (1/0.45)

// For Paul Hsieh hash function
#include <stdint.h> 
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif


// HashMap (create, expand, free)

HashMap* hashmapCreate(size_t initialCapacity, int (*hash)(char* key)) {
    assert(hash != NULL);
    
    HashMap* map = malloc(sizeof(HashMap));
    if (map == NULL) {
        return NULL;
    }

	map->bucketCount = initialCapacity;

    map->buckets = calloc(map->bucketCount, sizeof(Entry*));
    if (map->buckets == NULL) {
        free(map);
        return NULL;
    }
    
    map->size = 0;

    map->hash = hash; // function pointer

	#ifdef DEBUG
	fprintf(stdout, "HashMap Init OK. NumberOfBuckets=%d\n", (int)map->bucketCount);
    #endif

    return map;
}


void hashmapFree(HashMap* map) {
	#ifdef DEBUG
	fprintf(stdout, "Freeing the HashMap...\n");
	#endif

    size_t i;
    for (i = 0; i < map->bucketCount; i++) {
        Entry* entry = map->buckets[i];
        while (entry != NULL) {
            Entry* next = entry->next;
			entryFree(entry);
            entry = next;
        }
    }
    free(map->buckets);
    free(map);
}


size_t calculateIndex(size_t bucketCount, int hash) {
	return ((size_t) hash) % (bucketCount);
}


void hashmapExpandTest(HashMap* map) {

    // If the load factor exceeds 1/0.7...
    if (map->size > (map->bucketCount * LOAD_FACTOR)) {
		#ifdef DEBUG
		fprintf(stdout, "Expanding HashMap size...\n");
		#endif

        // Getting the size of the new HashMap and alloc a new space
        size_t newBucketCount = (size_t) getNextSize(map->bucketCount);
        Entry** newBuckets = calloc(newBucketCount, sizeof(Entry*));
        if (newBuckets == NULL) {
            // Abort expansion.
            return;
        }
        
        // Move existing entries to the new HashMap
        size_t i;
        for (i = 0; i < map->bucketCount; i++) {
            Entry* entry = map->buckets[i];
            while (entry != NULL) {
                Entry* next = entry->next;
                size_t index = calculateIndex(newBucketCount, entry->hash);
                entry->next = newBuckets[index];
                newBuckets[index] = entry;
                entry = next;
            }
        }

        // Free and update the HashMap
        free(map->buckets);
        map->buckets = newBuckets;
        map->bucketCount = newBucketCount;
    }
	#ifdef DEBUG
	else {
		fprintf(stdout, "HashMap size not changed.\n");
	}
	#endif
}


int getNextSize(int number) {
    int nextSize = number+(sqrt(number));
    if (nextSize%2 == 0) nextSize++;
    while (!naivePrime(nextSize)) nextSize+=2;
    return nextSize;
}


int naivePrime(int number) {
    long i, rac = (int)sqrt(number)+1;
    if (number%2 == 0) return 0;
    for (i=3; i<rac; i+=2) {
        if (number%i == 0) return 0;
    }
    return 1;
}


// Records (put, get, remove)

Entry* createEntry(char* key, int hash, char* IMEI, char *recordTime, char *providerId, char *techno, double centroidX, double centroidY) {

	// First, create record
	Record* record = malloc(sizeof(Record));
	if (record == NULL) {
		return NULL;
	}
	record->IMEI = IMEI;
	record->recordTime  = recordTime;
	record->providerId  = providerId;
	record->techno 		= techno;
	record->centroidX 	= centroidX;
	record->centroidY 	= centroidY;

	// Then, create entry
    Entry* entry = malloc(sizeof(Entry));
    if (entry == NULL) {
        return NULL;
    }
    entry->key 		= key;
    entry->hash		= hash;
    entry->record	= record;
    entry->next 	= NULL;
    return entry;
}


void hashmapPut(HashMap* map, char* IMEI, char *recordTime, char *providerId, char *techno, double centroidX, double centroidY) {

	// Key (concat provId and time)
	size_t len1 = strlen(IMEI);
	size_t len2 = strlen(recordTime);
	char *key = calloc(len1 + len2 + 1 +1, sizeof(char*));
	memcpy(key, IMEI, len1);
	memcpy(key+len1, ",", 1);
	memcpy(key+len1+1, recordTime, len2+1);

	//IMEI
	char *IMEI_a = calloc(strlen(IMEI), sizeof(char*));
	memcpy(IMEI_a, IMEI, strlen(IMEI));

	// recordTime
	char *recordTime_a = calloc(strlen(recordTime), sizeof(char*));
	memcpy(recordTime_a, recordTime, strlen(recordTime));

	// providerId
	char *providerId_a = calloc(strlen(providerId), sizeof(char*));
	memcpy(providerId_a, providerId, strlen(providerId));

	// techno
	char *techno_a = calloc(strlen(techno), sizeof(char*));
	memcpy(techno_a, techno, strlen(techno));


	int hash = map->hash(key);
    size_t index = calculateIndex(map->bucketCount, hash);
	#ifdef DEBUG
	fprintf(stdout, "Add entry OK. index=%d, key=%s | imei=%s, time=%s, provId=%s, tech=%s, cX=%f, cY=%f\n", (int)index, (char *)key, (char *)IMEI_a, (char *)recordTime_a, (char *)providerId_a, (char *)techno, (double)centroidX, (double)centroidY);
	#endif

	Entry *newEntry = createEntry(key, hash, IMEI_a, recordTime_a, providerId_a, techno_a, centroidX, centroidY);

	Entry *hashmapNode = map->buckets[index];

	if( hashmapNode == NULL ){ // If nothing at the index
		map->buckets[index] = newEntry;
		#ifdef DEBUG
		fprintf(stdout, "(This entry is the first in this bucket)\n");
		#endif
	} else { // If an entry already exists at the index

        while( hashmapNode->next!=NULL ){
            hashmapNode = hashmapNode->next;
        }
        hashmapNode->next = newEntry; // inserts the value as the last element of the list
		#ifdef DEBUG
		fprintf(stdout, "(This entry is NOT the first in this bucket)\n");
		#endif
	}

	map->size++;

	// Check is it's necessary to expand the HashMap size. If yes, it expands.
	hashmapExpandTest(map);
}


Entry* hashmapGet(HashMap* map, char* key) {
	int hash = map->hash(key);
    size_t index = calculateIndex(map->bucketCount, hash);

    Entry* entry = map->buckets[index];
    while (entry != NULL) {
        if (equalKeys(entry->key, entry->hash, key, hash)) {
            return entry;
        }
        entry = entry->next;
    }

    return NULL;
}


void entryFree(Entry* entry){
	if( entry != NULL ){
		free(entry->key);
		free(entry->record->IMEI);
		free(entry->record->recordTime);
		free(entry->record->providerId);
		free(entry->record->techno);
		free(entry->record);
        free(entry);
	}

	return;
}


bool hashmapRemove(HashMap* map, char* key) {
	int hash = map->hash(key);
    size_t index = calculateIndex(map->bucketCount, hash);

    // Pointer to the current entry
	Entry** p = &(map->buckets[index]);
    Entry* current;
    while ((current = *p) != NULL) {
        if (equalKeys(current->key, current->hash, key, hash)) {
            *p = current->next;
			entryFree(current);
            map->size--;
            return true;
        }

        p = &current->next;
    }

    return false;
}


Entry* hashmapPop(HashMap* map, char* key){
	int hash = map->hash(key);
    size_t index = calculateIndex(map->bucketCount, hash);

    // Pointer to the current entry
	Entry** p = &(map->buckets[index]);
    Entry* current;
    while ((current = *p) != NULL) {
        if (equalKeys(current->key, current->hash, key, hash)) {
            *p = current->next;
            map->size--;
            return current;
        }

        p = &current->next;
    }

    return NULL;
}


// Simple operations

int hashmapCountTechno(HashMap* map, char* techno) {
    int count;
	count = 0;

    size_t i;
    for (i = 0; i < map->bucketCount; i++) {
        Entry* entry = map->buckets[i];
        while (entry != NULL) {
            if ( strcmp(entry->record->techno, techno) == 0 ){
                count++;
            }
            entry = entry->next;
        }
    }

    return count;
}


// Miscellaneous (comparison, hashing, for each)

bool equalKeys(char* keyA, int hashA, char* keyB, int hashB) {
    if (hashA != hashB) {
        return false;
    }
	return strcmp(keyA, keyB) == 0;
}


int hashmapIntHash(char* key) {

	/*
	// First test "hash" function (In fact, it's just a way to get an int from a string)
	int a;

	int i;
	for( i=0 ; i<strlen(key) ; i++){
		a += (int)key[i];
	}
	*/

	/*
	// 4-byte Integer Hash function (Robert Jenkins' 32 bit integer) from :
	http://burtleburtle.net/bob/hash/integer.html
	a = (a+0x7ed55d16) + (a << 12);
	a = (a^0xc761c23c) ^ (a >> 19);
	a = (a+0x165667b1) + (a << 5);
	a = (a+0xd3a2646c) ^ (a << 9);
	a = (a+0xfd7046c5) + (a << 3);
	a = (a^0xb55a4f09) ^ (a >> 16);
	*/

	/*
	// Robert Jenkins' hash Function
	a += (a << 12);
	a ^= (a >> 22);
	a += (a << 4);
	a ^= (a >> 9);
	a += (a << 10);
	a ^= (a >> 2);
	a += (a << 7);
	a ^= (a >> 12);
	*/

	/*
	// Knuth's Multiplicative Method
	a = (a >> 3) * 2654435761;
	*/

	// Paul Hsieh hash function
	// From: http://www.azillionmonkeys.com/qed/hash.html
	int len = strlen(key);
	uint32_t hash = len, tmp;
	int rem;

	if (len <= 0 || key == NULL) return 0;

	rem = len & 3;
	len >>= 2;

	// Main loop
	for (;len > 0; len--) {
		hash  += get16bits (key);
		tmp    = (get16bits (key+2) << 11) ^ hash;
		hash   = (hash << 16) ^ tmp;
		key  += 2*sizeof (uint16_t);
		hash  += hash >> 11;
	}

	// Handle end cases
	switch (rem) {
		case 3: hash += get16bits (key);
			hash ^= hash << 16;
			hash ^= ((signed char)key[sizeof (uint16_t)]) << 18;
			hash += hash >> 11;
		break;
		case 2: hash += get16bits (key);
			hash ^= hash << 11;
			hash += hash >> 17;
		break;
		case 1: hash += (signed char)*key;
			hash ^= hash << 10;
			hash += hash >> 1;
	}

	// Force "avalanching" of final 127 bits
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
   //return a;
}


// Display functions (display HashMap: graph & stats, entry)

void entryDisplay(Entry* entry, int index){
	if( entry != NULL ){
		if( index >= 0 ){
			fprintf(stdout, "[%d]: key=%s, ", index, entry->key);
		} else {
			fprintf(stdout, "[%s]: ", entry->key);
		}
		fprintf(stdout, "hash=%d ==> IMEI=%s, time=%s, provId=%s, tech=%s, cX=%0.1f, cY=%0.1f\n\n", entry->hash, entry->record->IMEI, entry->record->recordTime, entry->record->providerId, entry->record->techno, entry->record->centroidX, entry->record->centroidY);
	} else {
		fprintf(stdout, "No entry to display\n\n");
	}

	return;
}


void hashmapDisplay(HashMap* map) {
    size_t i;
    for (i = 0; i < map->bucketCount; i++) {
        Entry* entry = map->buckets[i];
        while (entry != NULL) {
			entryDisplay(entry, i);
            entry = entry->next;
        }
    }
}


static void printSpaces(int nb){ // Graphic function
	int spaces;
	for( spaces=1 ; spaces<=nb ; spaces++ ){
		fprintf(stdout, " ");
	}
}


static void printMargin(int value){ // Graphic function
	if( value < 10 ){
		printSpaces(4);
	} else if( value < 100 ){
		printSpaces(3);
	} else if( value < 1000 ){
		printSpaces(2);
	} else if( value < 1000 ){
		printSpaces(1);
	}
}


void hashmapDisplayGraph(HashMap* map) {

	fprintf(stdout, "\n");

	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int margin_left = (w.ws_col-77)/2;
	int cols;
	fprintf(stdout, "━ HashMap display");
	for( cols=1 ; cols<=w.ws_col-17 ; cols++ ){
		fprintf(stdout, "━");
	}

	fprintf(stdout, "\n\e[4m");

	size_t i;
	int countEntries;

	int emptyBuckets = 0;
	int nonEmptyBuckets = 0;
	int nonEmptyBucketsSum = 0;

    for (i = 0; i < map->bucketCount; i++) {
		countEntries = 0;
        Entry* entry = map->buckets[i];
        while (entry != NULL) {
            countEntries++;
            entry = entry->next;
        }
		
		if( countEntries > 0 ){
			nonEmptyBucketsSum += countEntries;
			nonEmptyBuckets++;
			if( i % 2 == 0 ){
				fprintf(stdout, "\e[7m%d\e[27m", countEntries);
			} else {
				fprintf(stdout, "\e[100m%d\e[49m", countEntries);
			}
		} else {
			emptyBuckets++;
			if( i % 2 == 0 ){
				fprintf(stdout, "\e[7m \e[27m");
			} else {
				fprintf(stdout, "\e[100m \e[49m");
			}
		}
    }

	fprintf(stdout, "\e[24m\n");
	for( cols=1 ; cols<=w.ws_col ; cols++ ){
		fprintf(stdout, "─");
	}

	fprintf(stdout, "\n");
	printSpaces(margin_left);

	fprintf(stdout, "╔═Statistics════════════════════════════════════════════════════════════════╗\n");
	printSpaces(margin_left);
	fprintf(stdout, "  Number of buckets  : \e[1m%d\e[21m", (int)map->bucketCount);

		printMargin(map->bucketCount);

		fprintf(stdout, " \tNumber of empty buckets      : \e[1m%d\e[21m\n", emptyBuckets );

	printSpaces(margin_left);
	fprintf(stdout, "  Number of records  : \e[1m%d\e[21m", (int)map->size);
		
		printMargin(map->size);
		fprintf(stdout, " \tRate of empty buckets        : \e[1m%0.2f %%\e[21m\n", ((float)emptyBuckets/(float)map->bucketCount)*100);

	printSpaces(margin_left);
	fprintf(stdout, "  HashMap load       : \e[1m%0.3f\e[21m", (float)((float)map->size/(float)map->bucketCount));
		
		fprintf(stdout, " \tAvr size of non-empty buckets: \e[1m%0.2f\e[21m\n", ((float)nonEmptyBucketsSum/(float)nonEmptyBuckets));

	printSpaces(margin_left);
	fprintf(stdout, "  HashMap load factor: \e[1m%0.3f\e[21m\n", (float)LOAD_FACTOR);

	printSpaces(margin_left);
	fprintf(stdout, "  Load before resize : \e[1m%0.2f %%\e[21m\n", ((float)map->size/(float)map->bucketCount)/LOAD_FACTOR*100 );


	printSpaces(margin_left);
	fprintf(stdout, "╚═══════════════════════════════════════════════════════════════════════════╝\n");

	for( cols=1 ; cols<=w.ws_col ; cols++ ){
		fprintf(stdout, "━");
	}

	return;

}


