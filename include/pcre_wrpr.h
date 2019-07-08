#pragma once
#include <pcre.h>

typedef struct pcre_regex_s
{
    char const *regex;
    pcre *re_compiled;
    pcre_extra *extra;
    int options;
    char const *str;
    int *sub_strs_inxs;
    char const **sub_strs;
    int max_groups;
    int sub_strs_inxs_len;
} pcre_regex_t;

int get_max_groups_count(char const *str);
pcre_regex_t *pcre_compile_regex(char const *regex, int options,
                            int study_options);
int pcre_exec_regex(pcre_regex_t *regex, int startoffset, int options);
int pcre_get_groups(pcre_regex_t *regex, int groups_count);
int pcre_do_regex(pcre_regex_t *regex, char const *str,
             int startoffset, int exec_options);
char const *pcre_get_group(pcre_regex_t const *regex, int group,
                      int groups_count);
int free_pcre_groups(pcre_regex_t *regex);
int free_pcre_regex(pcre_regex_t *regex);