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
    printf("Table Initialized\n");
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
    //printf("level: %d key: %d result: %d\n",hashTable->level, key, result);
    return result;
}

struct Participant* getParticipant(struct LinearHashTable* hashTable, int key){
    int hashed_key = hashFunction(key, hashTable);
    if (hashed_key < hashTable->p) {
        hashed_key = reHashFunction(key, hashTable);
    }

    struct Bucket* bucket = &hashTable->bucket[hashed_key];

    if(bucket->overflowBucket == NULL){
        for (int i = 0; i < bucket->count; i++) {
        if (bucket->participant[i].PIN == key) {
            return &bucket->participant[i];  // Found the participant
        }
        }
    } else{
        for (int i = 0; i < bucket->of_count; i++) {
            for (int j=0; j<bucket->overflowBucket[i].count;j++){
                if (bucket->overflowBucket[i].participant[j].PIN == key) {
                    return &bucket->overflowBucket[i].participant[j];  // Found the participant
                }
            }
    }
    }

    for (int i = 0; i < bucket->count; i++) {
        if (bucket->participant[i].PIN == key) {
            return &bucket->participant[i];  // Found the participant
        }
    }
    // Participant not found, return a default participant with PIN set to -1
    return NULL;

}

void split(struct LinearHashTable* table){
    // Creating one new bucket
    table->num_of_buckets++;
    printf("num of buckets %d\n",table->num_of_buckets);
    table->bucket = (struct Bucket*)realloc(table->bucket, table->num_of_buckets * sizeof(struct Bucket));
    printf("-num of buckets increased to %d\n", table->num_of_buckets);
    struct Bucket * new_bucket = &table->bucket[table->num_of_buckets-1];
    new_bucket->overflowBucket = NULL;
    new_bucket->count = 0;
    new_bucket->of_count = 0;
    new_bucket->participant = (struct Participant*)malloc((table->max_keys_per_bucket+1) * sizeof(struct Participant));
    int cur_bucket_index = table->p;
    printf("-Spliting Bucket %d\n",cur_bucket_index);
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
        printf("Spliting main bucket: %d with %d part\n",cur_bucket_index, cur_Bucket->count);

        // Creating a copy of the main bucket
        struct Bucket* tempB = (struct Bucket*)malloc(sizeof(struct Bucket));
        tempB->participant = (struct Participant*)malloc((table->max_keys_per_bucket+1) * sizeof(struct Participant));
        tempB->count = cur_Bucket->count;

        // Copying the Participants
        for(int i=0; i<tempB->count; i++){
            tempB->participant[i] = cur_Bucket->participant[i];
        }
        printf("tempb count %d cur bucket count %d\n", tempB->count, cur_Bucket->count);
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
        printf("Trying to split overflowbuckets\n");
        struct Participant* Parts = (struct Participant*)malloc(2*sizeof(struct Participant));
        int PART_ARRAY_SIZE = 0;
        // for each overflow bucket
        for(int i=0; i<cur_Bucket->of_count; i++){
            // create a participant array of the overflow-buckets participants
            for(int j=0; j< cur_Bucket->overflowBucket[i].count; j++){
                PART_ARRAY_SIZE++;
                Parts = (struct Participant*)realloc(Parts ,(PART_ARRAY_SIZE +1) * sizeof(struct Participant));
                printf("scanned through %d parts\n", PART_ARRAY_SIZE);
                Parts[PART_ARRAY_SIZE -1] = cur_Bucket->overflowBucket[i].participant[j];
            }
            
            // clear the original one
            cur_Bucket->overflowBucket[i].participant = NULL;
            cur_Bucket->overflowBucket[i].count =0;
            cur_Bucket->overflowBucket[i].of_count =0;
            cur_Bucket->overflowBucket[i].overflowBucket =NULL;
        }
        cur_Bucket->of_count =0;
        free(cur_Bucket->overflowBucket);
        cur_Bucket->overflowBucket=NULL;
        for(int i=0; i<PART_ARRAY_SIZE; i++){
            struct Participant tempP = Parts[i];
            reInsert(table, tempP.PIN, tempP.last_name, tempP.first_name, tempP.zip_code);
        }
        free(Parts);
    }
    
    if(table->num_of_buckets == 2 * table->last_size){
        printf("-Buckets are doubled from %d to %d. Pointing to bucket 0 and uping the level to %d\n", table->last_size, table->num_of_buckets, table->level + 1 );
        table->p = 0;
        table->level++;
        table->last_size = table->num_of_buckets;
        
    }
    printf("-Split completed.\n");
}

void insert(struct LinearHashTable* table, int key, const char* last_name, const char* first_name, int zip_code) {
    int hashed_key = hashFunction(key, table);
    if (hashed_key<table->p){
        hashed_key = reHashFunction(key,table);
    }
    printf(" PIN: %d intented bucket: %d\n",key, hashed_key);

    struct Bucket* bucket;
    bucket = &(table->bucket[hashed_key]);
    // Check if key exists
    for (int i = 0; i < bucket->count; i++) {
        if (bucket->participant[i].PIN == key) {
            // Key already exists, update the values 
            printf("-Participant with this PIN already exists. Data was kept the same\n");
            return;
        }
    }

    printf("-main bucket has %d / %d part\n", bucket->count, table->max_keys_per_bucket);
    //Check if the main Bucket is full
    if(bucket->count >= table->max_keys_per_bucket){
        // Create an overflow bucket if it doesnt already exist
        if ( bucket->overflowBucket == NULL){
            bucket->overflowBucket = (struct Bucket*)malloc(sizeof(struct Bucket));
            if(bucket->overflowBucket ==NULL){
                perror("Memory allocation for overflow buckets failed");
                exit(1);
            }
            
            bucket->overflowBucket[0].count = 0;
            bucket->overflowBucket[0].participant = (struct Participant*)malloc((table->max_keys_per_bucket +1)  * sizeof(struct Participant));
            printf("here\n");
            bucket->overflowBucket[0].overflowBucket = NULL;
            bucket->overflowBucket[0].of_count = 0;
            bucket->of_count = 1;
            //perror("Created overflow bucket");
            printf(" bucket %d has %d overflow buckets\n", hashed_key, bucket->of_count);
            
            
        }
        printf("-passing data to overflow buckets\n");

        // Create a pointer bucket
        struct Bucket* cur_bucket;
        cur_bucket = &(bucket->overflowBucket[bucket->of_count-1]);
        
        printf("Inserting overflow bucket %d of bucket %d \n", bucket->of_count, hashed_key);
        // Pass the data 
        cur_bucket->participant[cur_bucket->count].PIN = key;
        strcpy(cur_bucket->participant[cur_bucket->count].last_name, last_name);
        strcpy(cur_bucket->participant[cur_bucket->count].first_name, first_name);
        cur_bucket->participant[cur_bucket->count].zip_code = zip_code;
        cur_bucket->participant[cur_bucket->count].vote='N';
        cur_bucket->count++;

        printf("-Inserted %d %s %s %d at overflowbucket %d of bucket %d\n",
            cur_bucket->participant[cur_bucket->count -1].PIN,
            cur_bucket->participant[cur_bucket->count -1].last_name,
            cur_bucket->participant[cur_bucket->count -1].first_name,
            cur_bucket->participant[cur_bucket->count -1].zip_code,
            bucket->of_count, hashed_key);

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
            perror("Created overflow bucket");
            printf(" bucket %d has %d overflow buckets\n", hashed_key, bucket->of_count);
        }
    }else{
        // Pass data to main bucket

        printf("-passing data to main bucket\n");
        bucket->participant = (struct Participant*)realloc(bucket->participant,(table->max_keys_per_bucket+1) * sizeof(struct Participant));
        bucket->participant[bucket->count].PIN = key;
        strcpy(bucket->participant[bucket->count].last_name, last_name);
        strcpy(bucket->participant[bucket->count].first_name, first_name);
        bucket->participant[bucket->count].zip_code = zip_code;
        bucket->participant[bucket->count].vote='N';
        bucket->count++;

        printf("-Inserted %d %s %s %d at bucket %d - %d/%d\n",
            bucket->participant[bucket->count -1].PIN,
            bucket->participant[bucket->count -1].last_name,
            bucket->participant[bucket->count -1].first_name,
            bucket->participant[bucket->count -1].zip_code,
            hashed_key, bucket->count, table->max_keys_per_bucket);
    }

    table->size++;

    // Check if it's time to split a bucket
    int key_capacity = table->num_of_buckets * table->max_keys_per_bucket;
    float loadFactor = (float)table->size / key_capacity;
    if (loadFactor >= 0.75) {
        printf("--LoadFactor: %f bucket_num: %d \n",loadFactor, table->num_of_buckets);
        split(table);
    }

}

void reInsert(struct LinearHashTable* table, int key, const char* last_name, const char* first_name, int zip_code){
    
    int hashed_key = reHashFunction(key, table);
    printf("-ReInserting participant with PIN: %d, starting location Bucket: %d, new Bucket: %d num of buckets %d\n",key,table->p-1,hashed_key, table->num_of_buckets);
    struct Bucket* bucket;
    bucket = &(table->bucket[hashed_key]);
    // Check if key exists
    for (int i = 0; i < bucket->count; i++) {
        if (bucket->participant[i].PIN == key) {
            // Key already exists, update the values 
            printf("-Participant with this PIN already exists. Data was kept the same\n");
            return;
        }
    }

    printf("-main bucket has %d / %d part with %d overflow-buckets\n", bucket->count, table->max_keys_per_bucket, bucket->of_count);
    //Check if the main Bucket is full
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
            printf(" bucket %d has %d overflow buckets\n", hashed_key, bucket->of_count);
            
            
        }
        printf("-passing data to overflow buckets\n");
        if(bucket->overflowBucket[bucket->of_count-1].participant == NULL){
            printf("overflowbuvket participants unallocated\n");
            exit(1);
        }
        // Create a temp bucket
        struct Bucket* cur_bucket;
        cur_bucket = &(bucket->overflowBucket[bucket->of_count-1]);
        
        printf("Inserting overflow bucket %d of bucket %d \n", bucket->of_count, hashed_key);
        // Pass the data 
        cur_bucket->participant[cur_bucket->count].PIN = key;
        strcpy(cur_bucket->participant[cur_bucket->count].last_name, last_name);
        strcpy(cur_bucket->participant[cur_bucket->count].first_name, first_name);
        cur_bucket->participant[cur_bucket->count].zip_code = zip_code;
        cur_bucket->participant[cur_bucket->count].vote='N';
        cur_bucket->count++;

        printf("-Inserted %d %s %s %d at overflowbucket %d of bucket %d\n",
            cur_bucket->participant[cur_bucket->count -1].PIN,
            cur_bucket->participant[cur_bucket->count -1].last_name,
            cur_bucket->participant[cur_bucket->count -1].first_name,
            cur_bucket->participant[cur_bucket->count -1].zip_code,
            bucket->of_count, hashed_key);

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
            perror("Created overflow bucket");
            printf(" bucket %d has %d overflow buckets\n", hashed_key, bucket->of_count);
        }
    }else{
        // Pass data to main bucket

        printf("-passing data to main bucket\n");
        bucket->participant = (struct Participant*)realloc(bucket->participant,(table->max_keys_per_bucket+1) * sizeof(struct Participant));
        bucket->participant[bucket->count].PIN = key;
        strcpy(bucket->participant[bucket->count].last_name, last_name);
        strcpy(bucket->participant[bucket->count].first_name, first_name);
        bucket->participant[bucket->count].zip_code = zip_code;
        bucket->participant[bucket->count].vote='N';
        bucket->count++;

        printf("-Inserted %d %s %s %d at bucket %d - %d/%d\n",
            bucket->participant[bucket->count -1].PIN,
            bucket->participant[bucket->count -1].last_name,
            bucket->participant[bucket->count -1].first_name,
            bucket->participant[bucket->count -1].zip_code,
            hashed_key, bucket->count, table->max_keys_per_bucket);
    }
        
    printf("-Reinsertion complete.\n");
}

void printOverflowBuckets(struct LinearHashTable* table){
    for(int i=0;i<table->num_of_buckets;i++){
        for(int j=0; j<table->bucket[i].of_count; j++){
            printf("OverflowBucket %d of bucket %d has %d part in it\n",j,i,table->bucket[i].overflowBucket[j].count);
            for(int y=0; y<table->bucket[i].overflowBucket[j].count; y++){
                struct Participant* part = &table->bucket[i].overflowBucket[j].participant[y];
                printf("%d %s %s %d\n",part->PIN, part->last_name, part->first_name, part->zip_code);
            }
        }
    }
}

/*

void insert(struct LinearHashTable* table, int key, const char* last_name, const char* first_name, int zip_code) {
    int hashed_key = hashFunction(key, table);
    if (hashed_key<table->p){
        hashed_key = reHashFunction(key,table);
    }
    printf(" PIN: %d intented bucket: %d\n",key, hashed_key);

    struct Bucket* bucket = (struct Bucket*)malloc(sizeof(struct Bucket));
    bucket = &table->bucket[hashed_key];
    // Check if key exists
    for (int i = 0; i < bucket->count; i++) {
        if (bucket->participant[i].PIN == key) {
            // Key already exists, update the values
            printf("-Participant with this PIN already exists. Data was kept the same\n");
            return;
        }
    }

    printf("-main bucket has %d / %d part\n", bucket->count, table->max_keys_per_bucket);
    //Check if the main Bucket is full
    if(bucket->count >= table->max_keys_per_bucket){
        // Create an overflow bucket if it doesnt already exist
        if ( bucket->overflowBucket == NULL){
            bucket->overflowBucket = (struct Bucket*)malloc(sizeof(struct Bucket));
            if(bucket->overflowBucket ==NULL){
                perror("Memory allocation for overflow buckets failed");
                exit(1);
            }
            bucket->overflowBucket[bucket->of_count].count = 0;
            bucket->overflowBucket[bucket->of_count].participant = (struct Participant*)malloc(sizeof(struct Participant));
            bucket->overflowBucket[bucket->of_count].overflowBucket = NULL;
            bucket->overflowBucket[bucket->of_count].of_count = 0;
            bucket->of_count++;
            perror("Created overflow bucket");
        }
        printf("-passing data to overflow buckets\n");

        // Create a temp bucket
        struct Bucket* cur_bucket = (struct Bucket*)malloc(sizeof(struct Bucket));
        cur_bucket = &bucket->overflowBucket[bucket->of_count-1];
        if(sizeof(cur_bucket->participant) != 2 * sizeof(struct Participant)){
            cur_bucket->participant = (struct Participant*)realloc( cur_bucket->participant, table->max_keys_per_bucket * sizeof(struct Participant));
        }
        // Point it at the overflow bucket
        

        // Pass the data 
        cur_bucket->participant[cur_bucket->count].PIN = key;
        strcpy(cur_bucket->participant[cur_bucket->count].last_name, last_name);
        strcpy(cur_bucket->participant[cur_bucket->count].first_name, first_name);
        cur_bucket->participant[cur_bucket->count].zip_code = zip_code;
        cur_bucket->participant[cur_bucket->count].vote='N';
        cur_bucket->count++;

        printf("-Inserted %d %s %s %d at overflowbucket %d of bucket %d\n",
            cur_bucket->participant[cur_bucket->count -1].PIN,
            cur_bucket->participant[cur_bucket->count -1].last_name,
            cur_bucket->participant[cur_bucket->count -1].first_name,
            cur_bucket->participant[cur_bucket->count -1].zip_code,
            bucket->of_count, hashed_key);

        // if overflow bucket gets full, create a new one
        if (cur_bucket->of_count == table->max_keys_per_bucket){
            bucket->of_count++;
            bucket->overflowBucket = (struct Bucket*)realloc(bucket->overflowBucket, sizeof(struct Bucket) * (bucket->of_count));
            bucket->overflowBucket[bucket->of_count-1].participant = (struct Participant*)malloc(table->max_keys_per_bucket * sizeof(struct Participant));
            if (bucket->overflowBucket[bucket->of_count-1].participant==NULL)
            {
                perror("Memory reallocation for overflow bucket failed");
                exit(1);
            }
        }

    }else{
        // Pass data to main bucket

        printf("-passing data to main bucket\n");
        //bucket->participant = (struct Participant*)realloc(bucket->participant,table->max_keys_per_bucket * sizeof(struct Participant));
        bucket->participant[bucket->count].PIN = key;
        strcpy(bucket->participant[bucket->count].last_name, last_name);
        strcpy(bucket->participant[bucket->count].first_name, first_name);
        bucket->participant[bucket->count].zip_code = zip_code;
        bucket->participant[bucket->count].vote='N';
        bucket->count++;

        printf("-Inserted %d %s %s %d at bucket %d - %d/%d\n",
            bucket->participant[bucket->count -1].PIN,
            bucket->participant[bucket->count -1].last_name,
            bucket->participant[bucket->count -1].first_name,
            bucket->participant[bucket->count -1].zip_code,
            hashed_key, bucket->count, table->max_keys_per_bucket);
    }

    table->size++;



    // Check if it's time to split a bucket
    int key_capacity = table->num_of_buckets * table->max_keys_per_bucket;
    float loadFactor = (float)table->size / key_capacity;
    if (loadFactor >= 0.75) {
        printf("--LoadFactor: %f bucket_num: %d \n",loadFactor, table->num_of_buckets);
        split(table);
    }

}
*/

void freeHashTable(struct LinearHashTable* table) {
    for (int i = 0; i < table->num_of_buckets; i++) {
        free(table->bucket[i].participant);
        if (table->bucket[i].overflowBucket) {
            free(table->bucket[i].overflowBucket->participant);
            free(table->bucket[i].overflowBucket);
        }
    }
    free(table->bucket);
    free(table);
}
