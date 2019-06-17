#include <pcre.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pcre_wrpr.h"

int getMaxGroupsCount(char const *re)
{
    int count = 0, reMaxIndx = strlen(re) - 1;
    char *tmp = (char *)re;
    while ((tmp = strchr(tmp, '(')) != NULL)
    {
        int currentPos = tmp - re;
        if (!((currentPos + 2 <= reMaxIndx && *(tmp + 1) == '?' && *(tmp + 2) == ':') ||
              (currentPos - 1 >= 0 && *(tmp - 1) == '\\')))
            count++;
        tmp++;
    }
    return count;
}

pcreRegex *makeRegex(char const *regex, int options)
{
    pcreRegex *out = malloc(sizeof(pcreRegex));
    memcpy(out, &(pcreRegex){}, sizeof(pcreRegex));
    const char *pcreErrorStr;
    int pcreErrorOffset;
    pcre *tmpReCompiled;
    pcre_extra *tmpPcreExtra;
    char unsigned const *pcreTables = pcre_maketables();
    tmpReCompiled = pcre_compile(regex, options, &pcreErrorStr, &pcreErrorOffset, pcreTables);
    pcre_free((char unsigned *)pcreTables);
    if (tmpReCompiled == NULL)
    {
        printf("ERROR: Could not compile regex '%s': %s\n", regex, pcreErrorStr);
        pcre_free(tmpReCompiled);
        return out;
    }
    tmpPcreExtra = pcre_study(tmpReCompiled, 0, &pcreErrorStr);
    if (pcreErrorStr != NULL)
    {
        printf("ERROR: Could not study regex '%s': %s\n", regex, pcreErrorStr);
        pcre_free(tmpReCompiled);
        pcre_free_study(tmpPcreExtra);
        return out;
    }
    int maxGroups = getMaxGroupsCount(regex) + 1;
    memcpy(out, &(pcreRegex){.regex = regex,
                             .reCompiled = tmpReCompiled,
                             .pcreExtra = tmpPcreExtra,
                             .subStrInxs = malloc(3 * maxGroups * sizeof(int)),
                             //.subStrs = malloc(maxGroups * sizeof(char *)),
                             .maxGroups = maxGroups,
                             .subStrInxsLen = 3 * maxGroups},
           sizeof(pcreRegex));
    return out;
}

int execRegex(pcreRegex *regex)
{
    return pcre_exec(regex->reCompiled, regex->pcreExtra, regex->str,
                     strlen(regex->str), 0, 0, regex->subStrInxs,
                     regex->subStrInxsLen);
}

int getGroups(pcreRegex *regex, int pcreExecRet)
{
    return pcre_get_substring_list(regex->str, regex->subStrInxs, pcreExecRet,
                                   &regex->subStrs);
}

int doRegex(pcreRegex *regex, char const *str)
{
    regex->str = (char *)str;
    int ret = execRegex(regex);
    if (ret)
        getGroups(regex, ret);
    return ret;
}

int freeGroups(pcreRegex *regex)
{
    pcre_free_substring_list(regex->subStrs);
    return 0;
}

int freeRegex(pcreRegex *regex)
{
    pcre_free((pcre*)regex->reCompiled);
    pcre_free_study((pcre_extra*)regex->pcreExtra);
    free(regex->subStrInxs);
    free(regex);
    return 0;
}