#ifndef ZIPLIST_H
#define ZIPLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "hashTable.h"


struct ZipCodeNode {
    int zip_code;
    int count;
    struct ParticipantArrayNode* participants;
    struct ZipCodeNode* next;
    struct ZipCodeNode* prev;
    
};

struct ParticipantArrayNode {
    int count;
    struct Participant* participants;
    struct ParticipantArrayNode* next;
    struct ParticipantArrayNode* prev;
};


void addToList(struct ZipCodeNode* zipCodeNode, struct Participant* participant);
void changeVote(struct ZipCodeNode* zipCodeList, struct LinearHashTable* hashTable, int key);
void printZIPList(struct ZipCodeNode* zipCodeList);
void printPartZip(struct ZipCodeNode* zipCodeList, int zipCode);
void printZIPListDescending(struct ZipCodeNode* zipCodeList);
void freeZipCodeList(struct ZipCodeNode* zipCodeList);

#endif