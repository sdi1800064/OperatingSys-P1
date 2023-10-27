#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

struct Participant {
    int PIN;
    char last_name[50];
    char first_name[50];
    int zip_code;
    char vote;
};

struct Bucket {
    int count;
    int of_count;
    struct Participant* participant;
    struct Bucket* overflowBucket;
};

struct LinearHashTable {
    int last_size;
    int vote_count;
    int level;
    int p;
    int num_of_buckets;
    int m;
    int max_keys_per_bucket;
    int size;
    
    struct Bucket* bucket;
};




struct LinearHashTable* initializeHashTable(int max_keys_per_bucket);
int hashFunction(int key, struct LinearHashTable * hashTable);
int reHashFunction(int key, struct LinearHashTable* hashTable);
struct Participant* getParticipant(struct LinearHashTable* hashTable, int key);
void insert(struct LinearHashTable* table, int key, const char* last_name, const char* first_name, int zip_code);
void reInsert(struct LinearHashTable* table, int key, const char* last_name, const char* first_name, int zip_code);
void freeHashTable(struct LinearHashTable* table);
void split(struct LinearHashTable* table);
void printOverflowBuckets(struct LinearHashTable* table);

#endif