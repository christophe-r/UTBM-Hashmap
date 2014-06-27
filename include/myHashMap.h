#include <stdbool.h>

#ifndef __MYHASHMAP__
#define __MYHASHMAP__

typedef struct {
	char *IMEI;
	char *recordTime;
	char *providerId;
	char *techno;
	double centroidX;
	double centroidY;
} Record;

typedef struct Entry Entry; // KV Container equivalent
struct Entry {
	char* key;
	int hash;
	Record* record;
	Entry* next;
};

typedef struct {
	Entry** buckets;
	size_t bucketCount;
	int (*hash)(char* key);
	size_t size;
} HashMap;


// HashMap (create, expand, free)
HashMap* hashmapCreate(size_t initialCapacity, int (*hash)(char* key));
void hashmapFree(HashMap* map);
size_t calculateIndex(size_t bucketCount, int hash);
void hashmapExpandTest(HashMap* map);
int getNextSize(int number);
int naivePrime(int number);

// Records (put, get, remove)
Entry* createEntry(char* key, int hash, char* IMEI, char *recordTime, char *providerId, char *techno, double centroidX, double centroidY);
void hashmapPut(HashMap* map, char* IMEI, char *recordTime, char *providerId, char *techno, double centroidX, double centroidY);
Entry* hashmapGet(HashMap* map, char* key);
void entryFree(Entry* entry);
bool hashmapRemove(HashMap* map, char* key);
Entry* hashmapPop(HashMap* map, char* key);

// Simple operations
int hashmapCountTechno(HashMap* map, char* techno);

// Miscellaneous (comparison, hashing)
bool equalKeys(char* keyA, int hashA, char* keyB, int hashB);
int hashmapIntHash(char* key);
//void hashmapForEach(HashMap* map, bool (*callback)(Entry* entry, int index) );

// Display functions (display hashmap: graph & stats, entry)
void entryDisplay(Entry* entry, int index);
void hashmapDisplay(HashMap* map);
void hashmapDisplayGraph(HashMap* map);

#endif

