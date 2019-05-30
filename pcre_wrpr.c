#include <pcre.h>
#include <stdio.h>
#include <string.h>
#include "pcre_wrpr.h"

pcreRegex makeRegex(char *regex, const unsigned char *pcreTables)
{
    const char *pcreErrorStr;
    int pcreErrorOffset;
    pcre *tmpReCompiled;
    pcre_extra *tmpPcreExtra;
    tmpReCompiled = pcre_compile(regex, 0, &pcreErrorStr, &pcreErrorOffset, pcreTables);
    if (tmpReCompiled == NULL) 
    {
        printf("ERROR: Could not compile regex '%s': %s\n", regex, pcreErrorStr);
        pcre_free(tmpReCompiled);
        return (pcreRegex){};
    }
    tmpPcreExtra = pcre_study(tmpReCompiled, 0, &pcreErrorStr);
    if (pcreErrorStr != NULL) 
    {
        printf("ERROR: Could not study regex '%s': %s\n", regex, pcreErrorStr);
        pcre_free(tmpReCompiled);
        pcre_free_study(tmpPcreExtra);
        return (pcreRegex){};
    }
    return (pcreRegex){.regex = regex, .reCompiled = tmpReCompiled, .pcreExtra = tmpPcreExtra};
}

int execRegex(pcreRegex *regex)
{
    return pcre_exec(regex->reCompiled, regex->pcreExtra, regex->str, strlen(regex->str), 0, 0, regex->subStrInxs, 30);
}

int getGroups(pcreRegex *regex, int pcreExecRet)
{
    const char *psubStrMatchStr;
    for(int i = 0; i < pcreExecRet; i++)
    {
        pcre_get_substring(regex->str, regex->subStrInxs, pcreExecRet, i, &psubStrMatchStr);
        regex->subStrs[i] = realloc(regex->subStrs[i], strlen(psubStrMatchStr) + 1);
        strcpy(regex->subStrs[i], psubStrMatchStr);
    }
    return 0;
}

int doRegex(pcreRegex *regex, char* str)
{
    regex->str = str;
    int ret = execRegex(regex);
    if (ret)
        getGroups(regex, ret);
    return ret;
}