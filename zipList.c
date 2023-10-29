#include "hashTable.h"
#include "zipList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void addToList(struct ZipCodeNode* zipCodeNode, struct Participant* participant) {
    // Create a new participant node
    struct ParticipantArrayNode* participantNode = malloc(sizeof(struct ParticipantArrayNode));
    participantNode->count = 1; // Assuming you are adding one participant
    participantNode->participants = malloc(sizeof(struct Participant));
    participantNode->participants[0] = *participant;
    participantNode->next = NULL;
    participantNode->prev = NULL;

    // If list is empty put the participant node as head
    if (zipCodeNode->participants == NULL) {
        zipCodeNode->participants = participantNode;
        zipCodeNode->count++;
    } else {
        // Add the new participant node at the end of the list
        struct ParticipantArrayNode* temp = zipCodeNode->participants;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = participantNode;
        participantNode->prev = temp;
        zipCodeNode->count++;
    }
}

void changeVote(struct ZipCodeNode* zipCodeList, struct LinearHashTable* hashTable, int key) {
    struct Participant* participant = getParticipant(hashTable, key);
    if (participant != NULL) {
        if (participant->vote == 'N') {
            // Participant's vote is 'N', change it to 'Y'
            participant->vote = 'Y';
            hashTable->vote_count++;
            printf("%d marked 'Voted'\n", key);

            // Check if a corresponding zip code node exists
            struct ZipCodeNode* currentZipCodeNode = zipCodeList;
            while (currentZipCodeNode != NULL) {
                if (currentZipCodeNode->zip_code == participant->zip_code) {
                    // Participant's zip code matches the current node, add them to the array
                    addToList(currentZipCodeNode, participant);
                    return;
                }
                currentZipCodeNode = currentZipCodeNode->next;
            }

            // If the zip code node doesn't exist, create a new one and add the participant
            currentZipCodeNode = malloc(sizeof(struct ZipCodeNode));
            currentZipCodeNode->zip_code = participant->zip_code;
            currentZipCodeNode->participants = NULL;
            currentZipCodeNode->next = NULL;
            currentZipCodeNode->prev = NULL;

            // Add the new zip code node to the list
            if (zipCodeList == NULL) {
                zipCodeList = currentZipCodeNode;
            } else {
                // Add the new zip code node at the end of the list
                struct ZipCodeNode* temp = zipCodeList;
                while (temp->next != NULL) {
                    temp = temp->next;
                }
                temp->next = currentZipCodeNode;
                currentZipCodeNode->prev = temp;
            }

            // Now, add the participant to the new zip code node
            addToList(currentZipCodeNode, participant);
        } else if (participant->vote == 'Y') {
            // Participant's vote is already 'Y', print a message
            printf("Participant with PIN %d already has a voted\n", key);
        }
    } else {
        printf("Participant with PIN %d not found.\n", key);
    }
}

void printZIPList(struct ZipCodeNode* zipCodeList) {
    struct ZipCodeNode* currentZipCodeNode = zipCodeList;

    printf("Zip Codes in the List:\n");
    while (currentZipCodeNode != NULL) {
        printf("%d\n", currentZipCodeNode->zip_code);
        currentZipCodeNode = currentZipCodeNode->next;
    }
}

void printPartZip(struct ZipCodeNode* zipCodeList, int zipCode) {
    struct ZipCodeNode* currentZipCodeNode = zipCodeList;
    // Check the list
    while (currentZipCodeNode != NULL) {
        // Find the zip code
        if (currentZipCodeNode->zip_code == zipCode) {
            printf("%d in ZIP Code %d:\n",currentZipCodeNode->count, zipCode);

            struct ParticipantArrayNode* currentParticipantArrayNode = currentZipCodeNode->participants;
            // Print all the participants in the node
            while (currentParticipantArrayNode != NULL) {
                for (int i = 0; i < currentParticipantArrayNode->count; i++) {
                    printf("PIN: %d, Last Name: %s, First Name: %s, Vote: %c\n",
                        currentParticipantArrayNode->participants[i].PIN,
                        currentParticipantArrayNode->participants[i].last_name,
                        currentParticipantArrayNode->participants[i].first_name,
                        currentParticipantArrayNode->participants[i].vote);
                }

                currentParticipantArrayNode = currentParticipantArrayNode->next;
            }
            return; // Exit the function after printing for the given zip code
        }

        currentZipCodeNode = currentZipCodeNode->next;
    }

    // If the zip code is not found in the list
    printf("ZIP Code %d not found in the list.\n", zipCode);
}

void printZIPListDescending(struct ZipCodeNode* zipCodeList) {
    // Traverse the list to find the highest zip code
    struct ZipCodeNode* currentZipCodeNode = zipCodeList;
    int highestZipCode = 0;
    while (currentZipCodeNode != NULL) {
        if (currentZipCodeNode->zip_code > highestZipCode) {
            highestZipCode = currentZipCodeNode->zip_code;
        }
        currentZipCodeNode = currentZipCodeNode->next;
    }

    // Print zip codes from highest to smallest
    for (int zipCode = highestZipCode; zipCode > 0; zipCode--) {
        currentZipCodeNode = zipCodeList;
        while (currentZipCodeNode != NULL) {
            if (currentZipCodeNode->zip_code == zipCode) {
                printf("%d\n", zipCode);
                break;
            }
            currentZipCodeNode = currentZipCodeNode->next;
        }
    }
}

void freeZipCodeList(struct ZipCodeNode* zipCodeList) {
    while (zipCodeList != NULL) {
        struct ZipCodeNode* currentZipCodeNode = zipCodeList;
        zipCodeList = zipCodeList->next; // Move to the next node

        // Free the participants in the current zip code node
        struct ParticipantArrayNode* participantNode = currentZipCodeNode->participants;
        while (participantNode != NULL) {
            struct ParticipantArrayNode* currentParticipantNode = participantNode;
            participantNode = participantNode->next; // Move to the next participant node

            // Free the participants array
            free(currentParticipantNode);
        }

        // Free the current zip code node
        free(currentZipCodeNode);
    }
}