// run extract.py and then process.py before executing
#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sched.h> 
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#define HASH_SIZE 400007  // hashtable size ~ # of unique words
#define MAX_WORD_LEN 128      // word length, atmost
#define MAX_TID 12    

typedef struct WordNode { // represents each word + count
    char *word;
    int count;
    struct WordNode *next; // incase of collisions in hashtable
} WordNode;

WordNode *hashTable[HASH_SIZE];   // an array of linked lists -> hash table
pthread_mutex_t hashMutex[HASH_SIZE]; 
pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER; 

// global variables referring to each file 
char files[100500][1024];  
int totalFiles = 0;
int currentFileIndex = 0;

// H-func
unsigned int hashFunc(const char *str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash * 31) + *str++; //polynomial hashing
        // ascii values used to calculate hash
    }
    return hash % HASH_SIZE;
}

// Insert word into hash table/update count
void insertWord(const char *word) {
    unsigned int index = hashFunc(word);
    pthread_mutex_lock(&hashMutex[index]); // every word count is to be locked when accessed

    WordNode *node = hashTable[index];
    while (node) {
        if (strcmp(node->word, word) == 0) {
            node->count++;
            pthread_mutex_unlock(&hashMutex[index]); // word = hash table word, work is done
            return;
        }
        node = node->next; // else find word in linked lists
    }

    // word not in hash table
    WordNode *newNode = malloc(sizeof(WordNode)); 
    newNode->word = strdup(word);
    newNode->count = 1;
    newNode->next = hashTable[index]; // insert new node at the head
    hashTable[index] = newNode;

    pthread_mutex_unlock(&hashMutex[index]);
}



int isValidWord(const char *word) {
    int hasAlpha = 0;

    for (int i = 0; word[i]; i++) {
        if (isalpha(word[i])) {
            hasAlpha = 1; // just letters
        } 
        else if (isdigit(word[i])) {
            return 0; // number detected
        }
    }
    return hasAlpha; 
}

void distinguishWord(char *word) {
    char cleanWord[MAX_WORD_LEN];
    int index = 0;

    for (int i = 0; word[i]; i++) {
        if (isalpha(word[i])) {
            cleanWord[index++] = word[i]; 
        }
        else {
            // ignore special characters
            if (index > 0) {
                cleanWord[index] = '\0';
                if (isValidWord(cleanWord)) {
                    insertWord(cleanWord);
                }
                index = 0; 
            }
        }
    }

    // break-free
    if (index > 0) {
        cleanWord[index] = '\0';
        if (isValidWord(cleanWord)) {
            insertWord(cleanWord);
        }
    }
}

// open file by thread
void processFile(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char word[MAX_WORD_LEN];
    while (fscanf(file, "%127s", word) == 1) {
        for (int i = 0; word[i]; i++) {
            word[i] = tolower(word[i]);
        }
        distinguishWord(word);
    }
    fclose(file);
}


// Threads access files
// The files try to increment fileIndex
// or aternatively, try to access the next file from a pool of files
// Since file access must be synchronized, a mutex exists for each one
void *access_file(void *arg) {
    while (1) {
        pthread_mutex_lock(&fileMutex);
        if (currentFileIndex >= totalFiles) {
            pthread_mutex_unlock(&fileMutex);
            break;  // files finished
        }
        char filePath[1024]; // a file = a file path
        strcpy(filePath, files[currentFileIndex++]);
        pthread_mutex_unlock(&fileMutex);

        processFile(filePath);
    }
    return NULL;
}

// Output Verification
void storeOutput() {
    FILE *outputFile = fopen("word_frequencies.txt", "w");
    if (!outputFile) {
        perror("Error opening output file");
        return;
    }

    int uniqueWords = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        WordNode *node = hashTable[i];
        while (node) {
            fprintf(outputFile, "%s: %d\n", node->word, node->count);
            uniqueWords++;
            node = node->next;
        }
    }
    fprintf(outputFile, "Total Unique Words: %d\n", uniqueWords);
    fclose(outputFile);
}

// Cleanup memory
void cleanup() {
    for (int i = 0; i < HASH_SIZE; i++) {
        pthread_mutex_lock(&hashMutex[i]);
        WordNode *node = hashTable[i];
        while (node) {
            WordNode *temp = node;
            node = node->next;
            free(temp->word);
            free(temp);
        }
        pthread_mutex_unlock(&hashMutex[i]);
    }
}

int main() {
    struct timespec start, end;
    double elapsed_time_parallel;

    char combinedDir[] = "text/en/combined_folder";  // Hardcoded path

    DIR *dir = opendir(combinedDir);
    if (!dir) {
        perror("Error opening directory");
        return EXIT_FAILURE;
    }

    // Initialize mutexes
    for (int i = 0; i < HASH_SIZE; i++) {
        pthread_mutex_init(&hashMutex[i], NULL);
    }

    struct dirent *entry;

    // Store all file paths
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') { 
            snprintf(files[totalFiles], sizeof(files[totalFiles]), "%s/%s", combinedDir, entry->d_name);
            totalFiles++;
        }
    }
    closedir(dir);

    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t TID[MAX_TID];
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < MAX_TID; i++) {
        // Set affinity
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i % num_cores, &cpuset);  // bind thread ~ RR

        pthread_setaffinity_np(TID[i], sizeof(cpu_set_t), &cpuset);
        pthread_create(&TID[i], NULL, access_file, NULL);

    }

    for (int i = 0; i < MAX_TID; i++) {
        pthread_join(TID[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time_parallel = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Write results
    storeOutput();

    // Clean up
    cleanup();
    for (int i = 0; i < HASH_SIZE; i++) {
        pthread_mutex_destroy(&hashMutex[i]);
    }
    pthread_mutex_destroy(&fileMutex);

    printf("Parallel computation took: %.6f seconds\n", elapsed_time_parallel);
    printf("Number of threads used: %d\n", MAX_TID);

    return 0;
}
