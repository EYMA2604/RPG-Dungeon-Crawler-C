#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

char* getString(const char* prompt) {
    printf("%s", prompt);
    //Getting chars one-by-one and realloc for each.
     while (getchar() != '\n');
    int count = 0;
    char* name = NULL;
    char c = getchar();;
    while (c != '\n') {
        count++;
        name = realloc(name, sizeof(char)*count);
        name[count-1] = c;
        c = getchar();
    }
    count++;
    name = realloc(name, sizeof(char)*count);
    name[count-1] = '\0';
    return name;
}

int getInt(const char* prompt) {
    printf("%s", prompt);
    int val;
    scanf(" %d", &val);
    return val;
}

int max(int a, int b) {
    //returns the bigger number between the two given.
    if (a>b)
        return a;
    return b;
}