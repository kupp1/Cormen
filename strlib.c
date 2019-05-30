#include <string.h>
#include <stdlib.h>
#include "strlib.h"

int strcount(char *str, char *substr)
{
    int count = 0, subLen = strlen(substr);
    while ( (str = strstr(str, substr)) != NULL )
    {
        count++;
        str += subLen;
    }
    return count;
}

int strsplit(char ***result, char *str, char *splitstr)
{
    int splitLen = strlen(splitstr);
    int sizeOfResult = strcount(str, splitstr) + 1;
    *result = malloc(sizeOfResult * sizeof(char*));
    char *tmp = strstr(str, splitstr);
    for (int i = 0; i < sizeOfResult - 1; i++)
    {
        int sizeOfSubstr = tmp - str;
        (*result)[i] = malloc(sizeOfSubstr + 1);
        strncpy((*result)[i], str, sizeOfSubstr);
        (*result)[i][sizeOfSubstr] = 0;
        str = tmp + splitLen;
        tmp = strstr(str, splitstr);
    }
    (*result)[sizeOfResult - 1] = malloc(strlen(str) + 1);
    strcpy((*result)[sizeOfResult - 1], str);
    (*result)[sizeOfResult - 1][strlen(str)] = 0;
    return sizeOfResult;
}