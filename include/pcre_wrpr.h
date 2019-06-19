#pragma once
#include <pcre.h>

typedef struct pcreRegex_s
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
} pcreRegex_t;

int getMaxGroupsCount(char const *str);
pcreRegex_t *compileRegex(char const *regex, int options);
int execRegex(pcreRegex_t *regex);
int getGroups(pcreRegex_t *regex, int pcreExecRet);
int doRegex(pcreRegex_t *regex, char const *str);
int freeGroups(pcreRegex_t *regex);
int freeRegex(pcreRegex_t *regex);