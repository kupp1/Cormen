#include <string.h>
#include <stdlib.h>
#include "strlib.h"

int strcount(char const *str, char const *substr)
{
    int count = 0, sub_len = strlen(substr);
    while ((str = strstr(str, substr)) != NULL)
    {
        count++;
        str += sub_len;
    }
    return count;
}

int strsplit(char ***result, char const *str, char const *splitstr)
{
    int split_len = strlen(splitstr);
    int size_of_result = strcount(str, splitstr) + 1;
    *result = malloc(size_of_result * sizeof(char *));
    char *tmp = strstr(str, splitstr);
    for (int i = 0; i < size_of_result - 1; i++)
    {
        int size_of_substr = tmp - str;
        (*result)[i] = malloc(size_of_substr + 1);
        strncpy((*result)[i], str, size_of_substr);
        (*result)[i][size_of_substr] = 0;
        str = tmp + split_len;
        tmp = strstr(str, splitstr);
    }
    (*result)[size_of_result - 1] = malloc(strlen(str) + 1);
    strcpy((*result)[size_of_result - 1], str);
    (*result)[size_of_result - 1][strlen(str)] = 0;
    return size_of_result;
}