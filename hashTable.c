#include "hashTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

struct LinearHashTable* initializeHashTable(int max_keys_per_bucket) {
    struct LinearHashTable* hashTable = (struct LinearHashTable*)malloc(sizeof(struct LinearHashTable));
    hashTable->level=0;
    hashTable->m = 2;
    hashTable->num_of_buckets = 2;
    hashTable->last_size =2;
    hashTable->size =0;
    hashTable->p =0;
    hashTable->max_keys_per_bucket = max_keys_per_bucket;
    hashTable->bucket = (struct Bucket*)malloc(hashTable->num_of_buckets * sizeof(struct Bucket));
    for(int i=0; i<hashTable->num_of_buckets;i++){
        hashTable->bucket[i].participant = (struct Participant*)malloc((hashTable->max_keys_per_bucket +1) * sizeof(struct Participant));
        hashTable->bucket[i].overflowBucket=NULL;
        hashTable->bucket[i].count = 0;
        hashTable->bucket[i].of_count = 0;
    }
    return hashTable;
}

int hashFunction(int key, struct LinearHashTable * hashTable) {
    int temp = 1 << (hashTable->level);
    int result = key % (temp * hashTable->m);
    return result;
}

int reHashFunction(int key, struct LinearHashTable* hashTable){
    int temp = 1 << (hashTable->level + 1);
    int result = key % (temp * hashTable->m);
    return result;
}

struct Participant* getParticipant(struct LinearHashTable* hashTable, int key){
    // Get the hash key from H1
    int hashed_key = hashFunction(key, hashTable);
    struct Bucket* bucket = &hashTable->bucket[hashed_key];
    // Check if there are any overflow buckets
    if(bucket->overflowBucket == NULL){
        // Check the main bucket
        for (int i = 0; i < bucket->count; i++) {
            if (bucket->participant[i].PIN == key) {
                return &bucket->participant[i];  // Found the participant
            }
        }

    } else{
        // Check all the overflow buckets
        for (int i = 0; i < bucket->of_count; i++) {
            for (int j=0; j<bucket->overflowBucket[i].count;j++){
                if (bucket->overflowBucket[i].participant[j].PIN == key) {
                    return &bucket->overflowBucket[i].participant[j];  // Found the participant
                }
            }
        }

        for (int i = 0; i < bucket->count; i++) {
            if (bucket->participant[i].PIN == key) {
                return &bucket->participant[i];  // Found the participant
            }
        }
    }

    // Participant not found, return a default participant
    return NULL;

}

void split(struct LinearHashTable* table){
    // Creating one new bucket
    table->num_of_buckets++;
    table->bucket = (struct Bucket*)realloc(table->bucket, table->num_of_buckets * sizeof(struct Bucket));
    struct Bucket * new_bucket = &table->bucket[table->num_of_buckets-1];
    new_bucket->overflowBucket = NULL;
    new_bucket->count = 0;
    new_bucket->of_count = 0;
    new_bucket->participant = (struct Participant*)malloc((table->max_keys_per_bucket+1) * sizeof(struct Participant));

    // Move the Split Pointer to the next bucket
    int cur_bucket_index = table->p;
    table->p++; 
    
    if(new_bucket->participant == NULL){
        perror("Memory allocation for hashTable failed");
        exit(1);
    }
    // Getting the bucket p is pointing
    
    struct Bucket* cur_Bucket = &table->bucket[cur_bucket_index];
    // check if main bucket has overflow buckets
    if(cur_Bucket->overflowBucket == NULL){
        // Splitting the main bucket

        // Creating a copy of the main bucket
        struct Bucket* tempB = (struct Bucket*)malloc(sizeof(struct Bucket));
        tempB->participant = (struct Participant*)malloc((table->max_keys_per_bucket+1) * sizeof(struct Participant));
        tempB->count = cur_Bucket->count;

        // Copying the Participants
        for(int i=0; i<tempB->count; i++){
            tempB->participant[i] = cur_Bucket->participant[i];
        }
        tempB->overflowBucket = NULL;
        tempB->of_count = 0;

        // Emptying the table's copy
        free(cur_Bucket->participant);
        cur_Bucket->participant = (struct Participant*)malloc((table->max_keys_per_bucket+1) * sizeof(struct Participant));
        cur_Bucket->count=0;
        cur_Bucket->overflowBucket = NULL;

        // Reinserting
        for(int i=0; i<tempB->count;i++){
            struct Participant* cur_part = &tempB->participant[i];
            reInsert(table, cur_part->PIN, cur_part->last_name, cur_part->first_name, cur_part->zip_code);
        }


        
    } else{
        // Spliting all of the overflow bucket
        struct Participant* Parts = (struct Participant*)malloc(2*sizeof(struct Participant));
        int PART_ARRAY_SIZE = 0;
        // for each overflow bucket
        for(int i=0; i<cur_Bucket->of_count; i++){
            // create a participant array of the overflow-buckets participants
            for(int j=0; j< cur_Bucket->overflowBucket[i].count; j++){
                PART_ARRAY_SIZE++;
                Parts = (struct Participant*)realloc(Parts ,(PART_ARRAY_SIZE +1) * sizeof(struct Participant));
                Parts[PART_ARRAY_SIZE -1] = cur_Bucket->overflowBucket[i].participant[j];
            }
            
            // clear the original one
            cur_Bucket->overflowBucket[i].participant = NULL;
            cur_Bucket->overflowBucket[i].count =0;
            cur_Bucket->overflowBucket[i].of_count =0;
            cur_Bucket->overflowBucket[i].overflowBucket =NULL;
        }
        cur_Bucket->of_count =0;

        // Free temporaty bucket
        free(cur_Bucket->overflowBucket);
        cur_Bucket->overflowBucket=NULL;
        for(int i=0; i<PART_ARRAY_SIZE; i++){
            struct Participant tempP = Parts[i];
            reInsert(table, tempP.PIN, tempP.last_name, tempP.first_name, tempP.zip_code);
        }
        // Clear temporary participants
        free(Parts);
    }
    
    if(table->num_of_buckets == 2 * table->last_size){
        table->p = 0;
        table->level++;
        table->last_size = table->num_of_buckets;
        
    }
}

void insert(struct LinearHashTable* table, int key, const char* last_name, const char* first_name, int zip_code) {
    printf("inserting %d\n", key);
    int hashed_key = hashFunction(key, table);
    if (hashed_key<table->p){
        hashed_key = reHashFunction(key,table);
    }

    struct Bucket* bucket;
    bucket = &(table->bucket[hashed_key]);
    // Check if key exists
    for (int i = 0; i < bucket->count; i++) {
        if (bucket->participant[i].PIN == key) {
            // Key already exists, update the values 
            printf("Participant with this PIN already exists. Data was kept the same\n");
            return;
        }
    }
    if(bucket->overflowBucket!=NULL){
        for(int i=0; i<bucket->of_count; i++){
            for(int j=0; j<bucket->overflowBucket[i].count; j++){
                if(bucket->overflowBucket[i].participant[j].PIN == key){
                    // Key already exists, update the values 
                    printf("Participant with this PIN already exists. Data was kept the same\n");
                    return;
                }
            }
        }
    }
        

    //Check if the main Bucket is full
    if(bucket->count >= table->max_keys_per_bucket){
        // Create an overflow bucket if it doesnt already exist
        if ( bucket->overflowBucket == NULL){
            bucket->overflowBucket = (struct Bucket*)malloc(sizeof(struct Bucket));
            if(bucket->overflowBucket ==NULL){
                perror("Memory allocation for overflow buckets failed");
                exit(1);
            }
            // Initiallize an overflow bucket
            bucket->overflowBucket[0].count = 0;
            bucket->overflowBucket[0].participant = (struct Participant*)malloc((table->max_keys_per_bucket +1)  * sizeof(struct Participant));
            bucket->overflowBucket[0].overflowBucket = NULL;
            bucket->overflowBucket[0].of_count = 0;
            bucket->of_count = 1;
        }
        struct Bucket* cur_bucket;
        cur_bucket = &(bucket->overflowBucket[bucket->of_count-1]);

        // Pass the data to the the last overflow bucket
        cur_bucket->participant[cur_bucket->count].PIN = key;
        strcpy(cur_bucket->participant[cur_bucket->count].last_name, last_name);
        strcpy(cur_bucket->participant[cur_bucket->count].first_name, first_name);
        cur_bucket->participant[cur_bucket->count].zip_code = zip_code;
        cur_bucket->participant[cur_bucket->count].vote='N';
        cur_bucket->count++;

        // if overflow bucket gets full, create a new one
        if (cur_bucket->count == table->max_keys_per_bucket){
            bucket->of_count++;
            bucket->overflowBucket = (struct Bucket*)realloc(bucket->overflowBucket, sizeof(struct Bucket) * (bucket->of_count));
            bucket->overflowBucket[bucket->of_count-1].participant = (struct Participant*)malloc((table->max_keys_per_bucket+1) * sizeof(struct Participant));
            if (bucket->overflowBucket[bucket->of_count-1].participant==NULL)
            {
                perror("Memory reallocation for overflow bucket failed");
                exit(1);
            }
        }
    }else{
        // Pass data to main bucket
        bucket->participant = (struct Participant*)realloc(bucket->participant,(table->max_keys_per_bucket+1) * sizeof(struct Participant));
        bucket->participant[bucket->count].PIN = key;
        strcpy(bucket->participant[bucket->count].last_name, last_name);
        strcpy(bucket->participant[bucket->count].first_name, first_name);
        bucket->participant[bucket->count].zip_code = zip_code;
        bucket->participant[bucket->count].vote='N';
        bucket->count++;
    }

    table->size++;

    // Check if it's time to split a bucket
    int key_capacity = table->num_of_buckets * table->max_keys_per_bucket;
    float loadFactor = (float)table->size / key_capacity;
    if (loadFactor >= 0.75) {
        split(table);
    }

}

void reInsert(struct LinearHashTable* table, int key, const char* last_name, const char* first_name, int zip_code){
    
    int hashed_key = reHashFunction(key, table);
    struct Bucket* bucket;
    bucket = &(table->bucket[hashed_key]);

    //Check if the main Bucket is full                          *Περιττό αφού αδειάζω τον original bucket
    if(bucket->count >= table->max_keys_per_bucket){
        // Create an overflow bucket if it doesnt already exist
        if ( bucket->overflowBucket == NULL){
            bucket->overflowBucket = (struct Bucket*)malloc(sizeof(struct Bucket));
            if(bucket->overflowBucket ==NULL){
                perror("Memory allocation for overflow buckets failed");
                exit(1);
            }
            bucket->overflowBucket[0].count = 0;
           
            bucket->overflowBucket[0].participant = (struct Participant*)malloc((table->max_keys_per_bucket+1) * sizeof(struct Participant));

            bucket->overflowBucket[0].overflowBucket = NULL;
            bucket->overflowBucket[0].of_count = 0;
            bucket->of_count = 1;
            //perror("Created overflow bucket");
            
            
        }
        if(bucket->overflowBucket[bucket->of_count-1].participant == NULL){
            printf("overflowbuvket participants unallocated\n");
            exit(1);
        }
        // Create a temp bucket
        struct Bucket* cur_bucket;
        cur_bucket = &(bucket->overflowBucket[bucket->of_count-1]);
        
        // Pass the data 
        cur_bucket->participant[cur_bucket->count].PIN = key;
        strcpy(cur_bucket->participant[cur_bucket->count].last_name, last_name);
        strcpy(cur_bucket->participant[cur_bucket->count].first_name, first_name);
        cur_bucket->participant[cur_bucket->count].zip_code = zip_code;
        cur_bucket->participant[cur_bucket->count].vote='N';
        cur_bucket->count++;

        
        // if overflow bucket gets full, create a new one
        if (cur_bucket->count == table->max_keys_per_bucket){
            bucket->of_count++;
            bucket->overflowBucket = (struct Bucket*)realloc(bucket->overflowBucket, sizeof(struct Bucket) * (bucket->of_count));
            bucket->overflowBucket[bucket->of_count-1].participant = (struct Participant*)malloc((table->max_keys_per_bucket+1) * sizeof(struct Participant));
            if (bucket->overflowBucket[bucket->of_count-1].participant==NULL)
            {
                perror("Memory reallocation for overflow bucket failed");
                exit(1);
            }
        }
    }else{
        // Pass data to main bucket
        bucket->participant = (struct Participant*)realloc(bucket->participant,(table->max_keys_per_bucket+1) * sizeof(struct Participant));
        bucket->participant[bucket->count].PIN = key;
        strcpy(bucket->participant[bucket->count].last_name, last_name);
        strcpy(bucket->participant[bucket->count].first_name, first_name);
        bucket->participant[bucket->count].zip_code = zip_code;
        bucket->participant[bucket->count].vote='N';
        bucket->count++;
    }

}

void freeHashTable(struct LinearHashTable* table) {
    size_t memory;
    for (int i = 0; i < table->num_of_buckets; i++) {
        memory += sizeof(table->bucket[i].participant);
        free(table->bucket[i].participant);
        if (table->bucket[i].overflowBucket) {
            memory += sizeof(table->bucket[i].overflowBucket->participant);
            memory += sizeof(table->bucket[i].overflowBucket);
            free(table->bucket[i].overflowBucket->participant);
            free(table->bucket[i].overflowBucket);
        }
    }
    memory += sizeof(table->bucket);
    free(table->bucket);
    memory += sizeof(table);
    free(table);
    printf("Total memory freed %lu\n", memory);
}
