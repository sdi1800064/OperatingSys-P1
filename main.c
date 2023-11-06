#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashTable.h"
#include "zipList.h"

#define MAX_FILENAME_LENGTH 256
#define MAX_BUCKET_SIZE 100



int main(int argc, char** argv) {
    char filename[MAX_FILENAME_LENGTH] = "";
    int bucketSize = MAX_BUCKET_SIZE;

    if (argc != 5) {
        fprintf(stderr, "Usage: %s -f <filename> -b <bucket_size>\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-f") == 0) {
            strncpy(filename, argv[i + 1], MAX_FILENAME_LENGTH);
        } else if (strcmp(argv[i], "-b") == 0) {
            bucketSize = atoi(argv[i + 1]);
            if (bucketSize <= 0) {
                fprintf(stderr, "Bucket size must be a positive integer.\n");
                return 1;
            }
        } else {
            fprintf(stderr, "Usage: %s -f <filename> -b <bucket_size>\n", argv[0]);
            return 1;
        }
    }

    // Check if the filename and bucket size have been provided
    if (filename[0] == '\0') {
        fprintf(stderr, "You must specify a filename using the -f option.\n");
        return 1;
    }

    // Open the file for reading
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Unable to open the file");
        return 1;
    }

    char line[256];
    
    struct LinearHashTable* hashTable = initializeHashTable(bucketSize);
    struct ZipCodeNode* zipCodeList = (struct ZipCodeNode*)malloc(sizeof(struct ZipCodeNode));

    while (fgets(line, sizeof(line), file) != NULL) {

        int PIN, postCode;
        char lastName[100], firstName[100];

        if (sscanf(line, "%d %99s %99s %d", &PIN, lastName, firstName, &postCode) == 4) {
            insert(hashTable, PIN, lastName, firstName, postCode);
        } else {
            printf("Skipping invalid line: %s", line);
        }
    }

    fclose(file);

    // Interactive command processing loop
    char command[256];
    int searchPIN;
    printf("Enter a command :\n");
    printf(" 'l' <PIN>' to search\n");
    printf(" 'i' <PIN> <LNAME> <FNAME> <ZIP> to insert\n");
    printf(" 'm' <PIN> to change the vote\n");
    printf(" 'bv' <FILENAME> to change vote of participants included in file\n");
    printf(" 'v' to print the number of participants who have voted\n");
    printf(" 'perc' to print the percentage of participants who have voted\n");
    printf(" 'z <ZIP_CODE>' to print participants for a specific ZIP code\n");
    printf(" 'o' to print all zip codes in descending order\n");
    printf(" 'exit' to exit the program: ");
    while (1) {

        if (scanf("%s", command) != 1) {
            fprintf(stderr, "Invalid input.\n");
            continue;
        }

        if (strcmp(command, "exit") == 0) {

            break;  // Exit the program
        }
        else if (strcmp(command, "l") == 0) {

                if (scanf("%d", &searchPIN) != 1) {
                    fprintf(stderr, "Invalid input for 'l' command.\n");
                    continue;
                }
                // Call a function to search for and print the person with the specified PIN
                struct Participant* result = getParticipant(hashTable, searchPIN);
                if (result == NULL) {

                    printf("Person with PIN %d not found.\n", searchPIN);
                } else {
                    printf("Person with PIN %d: , Name: %s %s, ZIP Code: %d, Vote: %c\n", result->PIN, result->first_name, result->last_name, result->zip_code, result->vote);
                }
            } 
        else if (strcmp(command, "i") == 0) {

            int pin;
            char last_name[50], first_name[50];
            int zip_code;

            if (scanf("%d %49s %49s %d", &pin, last_name, first_name, &zip_code) != 4) {

                fprintf(stderr, "Invalid input for 'i' command.\n");
                continue;
            }

            // Check if the participant with the given PIN already exists
            struct Participant* existingParticipant = getParticipant(hashTable, pin);

            if (existingParticipant == NULL) {

                // Participant doesn't exist, insert them
                insert(hashTable, pin, last_name, first_name, zip_code);
                printf("Inserted participant with PIN %d: %s %s, ZIP Code: %d\n", pin, first_name, last_name, zip_code);
            } 
            else {

                printf("Participant with PIN %d already exists: %s %s, ZIP Code: %d\n", pin, existingParticipant->first_name, existingParticipant->last_name, existingParticipant->zip_code);
            }
        } 
        else if(strcmp(command, "m") == 0){
            int pin;
            if (scanf("%d", &pin) != 1) {
                fprintf(stderr, "Invalid input for 'm' command.\n");
                continue;
            }
            // Call the changeVote function to change the vote of the participant
            changeVote(zipCodeList, hashTable, pin);
        }
         else if (strcmp(command, "bv") == 0) {
            char filename[MAX_FILENAME_LENGTH];

            if (scanf("%s", filename) != 1) {
                fprintf(stderr, "Invalid input for 'bv' command.\n");
                continue;
            }

            // Read the file and change the votes for the participants
            FILE* keyFile = fopen(filename, "r");
            if (keyFile == NULL) {
                perror("Unable to open the key file");
                continue;
            }

            struct Participant fileParticipant;

            while (fscanf(keyFile, "%d %s %s %d", &fileParticipant.PIN, fileParticipant.last_name, fileParticipant.first_name, &fileParticipant.zip_code) == 4) {
                // Change the vote to 'Y' for each participant
                changeVote(zipCodeList,hashTable, fileParticipant.PIN);
            }

            fclose(keyFile);
        }else if (strcmp(command, "v") == 0) {
            
            // Print the number of participants who have voted
            printf("Number of participants who have voted: %d\n", hashTable->vote_count);
        }
        else if (strcmp(command, "perc") == 0) {
            // Calculate and print the percentage of participants who have voted
            if (hashTable->size > 0) {
                double percentage = (double)hashTable->vote_count / hashTable->size * 100;
                printf("Percentage of participants who have voted: %.2f%%\n", percentage);
            } else {
                printf("No participants in the hash table.\n");
            }
        }
        else if (strcmp(command, "z") == 0) {
            int zipCodeToPrint;
            if (scanf("%d", &zipCodeToPrint) != 1) {
                fprintf(stderr, "Invalid input for 'z' command.\n");
                continue;
            }
            // Call the printPartZip function to print participants for the specified ZIP code
            if(zipCodeList != NULL){
                printPartZip(zipCodeList, zipCodeToPrint);
            } else{
                printf("No one has voted.\n");
            }
                
        }
        else if (strcmp(command, "o") == 0) {
            // Print all zip codes in descending order
            if(zipCodeList!=NULL){
                printZIPListDescending(zipCodeList);
            } else{
                printf("No one has voted.\n");
            }
                
        }
        else {

        fprintf(stderr, "Invalid command.\n");
        }
        printf("Enter a new command : ");
    }

    freeZipCodeList(zipCodeList);
    freeHashTable(hashTable);

    return 0;
}