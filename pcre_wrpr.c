#include <pcre.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pcre_wrpr.h"

int get_max_groups_count(char const *re)
{
    int count = 0, re_max_indx = strlen(re) - 1;
    char *tmp = (char *)re;
    while ((tmp = strchr(tmp, '(')) != NULL)
    {
        int current_pos = tmp - re;
        if (!((current_pos + 2 <= re_max_indx && *(tmp + 1) == '?' &&
               *(tmp + 2) == ':') ||
              (current_pos - 1 >= 0 && *(tmp - 1) == '\\')))
            count++;
        tmp++;
    }
    return count;
}

pcre_regex_t *compile_regex(char const *regex, int options,
                            int study_options)
{
    pcre_regex_t *out = malloc(sizeof(pcre_regex_t));
    out = NULL;
    const char *error_str;
    int error_offset;
    pcre *tmp_re_compiled;
    pcre_extra *tmp_extra;
    char unsigned const *tables = pcre_maketables();
    tmp_re_compiled = pcre_compile(regex, options,
                                   &error_str, &error_offset,
                                   tables);
    pcre_free((char unsigned *)tables);
    if (tmp_re_compiled == NULL)
    {
        printf("ERROR: Could not compile regex '%s': %s\n", regex,
               error_str);
        pcre_free(tmp_re_compiled);
        return out;
    }
    tmp_extra = pcre_study(tmp_re_compiled, study_options,
                           &error_str);
    if (error_str != NULL)
    {
        printf("ERROR: Could not study regex '%s': %s\n", regex,
               error_str);
        pcre_free(tmp_re_compiled);
        pcre_free_study(tmp_extra);
        return out;
    }
    out = malloc(sizeof(pcre_regex_t));
    int max_groups = get_max_groups_count(regex) + 1;
    *out = (pcre_regex_t){.regex = regex,
                         .re_compiled = tmp_re_compiled,
                         .extra = tmp_extra,
                         .sub_strs_inxs = \
                         malloc(3 * max_groups * sizeof(int)),
                         .sub_strs = NULL,
                         .max_groups = max_groups,
                         .sub_strs_inxs_len = 3 * max_groups};
    return out;
}

int exec_regex(pcre_regex_t *regex, int startoffset, int options)
{
    return pcre_exec(regex->re_compiled, regex->extra,
                     regex->str, strlen(regex->str),
                     startoffset, options, regex->sub_strs_inxs,
                     regex->sub_strs_inxs_len);
}

int get_groups(pcre_regex_t *regex, int groups_count)
{
    return pcre_get_substring_list(regex->str,
                                   regex->sub_strs_inxs,
                                   groups_count,
                                   &regex->sub_strs);
}

int do_regex(pcre_regex_t *regex, char const *str,
             int startoffset, int exec_options)
{
    regex->str = (char*)str;
    int ret = exec_regex(regex, startoffset, exec_options);
    if (ret)
        get_groups(regex, ret);
    return ret;
}

int free_pcre_groups(pcre_regex_t *regex)
{
    pcre_free_substring_list(regex->sub_strs);
    return 0;
}

int free_pcre_regex(pcre_regex_t *regex)
{
    pcre_free((pcre*)regex->re_compiled);
    pcre_free_study((pcre_extra*)regex->extra);
    free(regex->sub_strs_inxs);
    free(regex);
    return 0;
}