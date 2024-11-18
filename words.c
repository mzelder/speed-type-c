#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "words.h"

#define MAX_WORDS 370000 
#define MAX_WORDS_LEN 30
#define WORDS_COUNTER 20

int total_words = 0;

int* get_random_indexes(int count) {
    int *indexes = (int*)malloc(count * sizeof(int));
    if (indexes == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE); 
    }
    for (int i = 0; i < count; i++) {
        indexes[i] = rand() % MAX_WORDS;
    }
    return indexes;
}

int is_index_in_array(int index, int *array, int size) {
    for (int i = 0; i < size; i++) {
        if (array[i] == index) {
            return 1;
        }
    }
    return 0; 
}

void shuffle_words(char **words, int count) {
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1); 
        char *temp = words[i];
        words[i] = words[j];
        words[j] = temp;
    }
}

char** get_words() { 
    FILE *file;
    char buffer[MAX_WORDS_LEN];

    char **words = (char**)malloc(WORDS_COUNTER * sizeof(char*));
    for (int i = 0; i < WORDS_COUNTER; i++) {
        words[i] = (char*)malloc(MAX_WORDS_LEN * sizeof(char));
    }

    // Get random indexes 
    srand(time(NULL));
    int *random_indexes = get_random_indexes(WORDS_COUNTER);
    
    file = fopen("words_alpha.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }
    for (int i = 0; i < MAX_WORDS; i++) { 
        if (fgets(buffer, sizeof(buffer), file) == NULL) break; 

        buffer[strcspn(buffer, "\n")] = '\0';
        buffer[strcspn(buffer, "\r")] = '\0';
        
        if (is_index_in_array(i, random_indexes, WORDS_COUNTER)) {
            strcpy(words[total_words], buffer);
            total_words++;
        }      
    }
    fclose(file);
    free(random_indexes);

    shuffle_words(words, total_words);

    return words;
}

int get_words_length() {
    return total_words;
}

char* get_sentence(char **words) { 
    
    char* sentence = (char*)malloc(WORDS_COUNTER * MAX_WORDS_LEN * sizeof(char)); 
    sentence[0] = '\0';  

    for (int i = 0; i < WORDS_COUNTER; i++) {
        strcat(sentence, words[i]);
        strcat(sentence, " ");
    }

    // DEBUG
    // printf("%s", sentence);

    return sentence;
}