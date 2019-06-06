#pragma once
#include <pcre.h>

typedef struct
{
    char const *regex;
    pcre *reCompiled;
    pcre_extra *pcreExtra;
    char *str;
    int *subStrInxs;
    char const **subStrs;
    int const maxGroups;
} pcreRegex;

int getMaxGroupsCount(char const *str);
pcreRegex makeRegex(char const *regex);
int execRegex(pcreRegex *regex);
int getGroups(pcreRegex *regex, int pcreExecRet);
int doRegex(pcreRegex *regex, char const *str);
int freeGroups(pcreRegex *regex);
int freeRegex(pcreRegex *regex);