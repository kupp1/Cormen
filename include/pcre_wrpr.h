#pragma once
#include <pcre.h>

typedef struct pcreRegex_s
{
    char const *regex;
    pcre *reCompiled;
    pcre_extra *pcreExtra;
    int options;
    char const *str;
    int *subStrInxs;
    char const **subStrs;
    int maxGroups;
    int subStrInxsLen;
} pcreRegex_t;

int getMaxGroupsCount(char const *str);
pcreRegex_t *compileRegex(char const *regex, int options);
int execRegex(pcreRegex_t *regex);
int getGroups(pcreRegex_t *regex, int pcreExecRet);
int doRegex(pcreRegex_t *regex, char const *str);
int freeGroups(pcreRegex_t *regex);
int freeRegex(pcreRegex_t *regex);