#pragma once

typedef struct
{
    char *regex;
    pcre *reCompiled;
    pcre_extra *pcreExtra;
    char *str;
    int subStrInxs[30];
    char* subStrs[15];
} pcreRegex;

pcreRegex makeRegex(char *regex, const unsigned char *pcreTables);
int execRegex(pcreRegex *regex);
int getGroups(pcreRegex *regex, int pcreExecRet);
int doRegex(pcreRegex *regex, char* str);