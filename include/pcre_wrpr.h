#pragma once
#include <pcre.h>

typedef struct pcreRegexS
{
    char const *regex;
    pcre const *reCompiled;
    pcre_extra const *pcreExtra;
    int const options;
    char *str;
    int *subStrInxs;
    char const **subStrs;
    int const maxGroups;
    int const subStrInxsLen;
} pcreRegex;

int getMaxGroupsCount(char const *str);
pcreRegex *makeRegex(char const *regex, int options);
int execRegex(pcreRegex *regex);
int getGroups(pcreRegex *regex, int pcreExecRet);
int doRegex(pcreRegex *regex, char const *str);
int freeGroups(pcreRegex *regex);
int freeRegex(pcreRegex *regex);