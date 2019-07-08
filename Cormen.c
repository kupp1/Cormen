#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pcre.h>
#include <locale.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "irc.h"
#include "pcre_wrpr.h"
#include "strlib.h"

char *server = "irc.freenode.net";
char *port = "6697";

char *nick = "Cormen";
char *username = "bot";
char *realname = "kupp bot";
char *host_nick = "kupp";

static bool killflag = false;

void sigint_handler(int dummy)
{
    killflag = true;
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigint_handler);

    setlocale(LC_ALL, NULL);

    pcre_regex_t *re = pcre_compile_regex(rfc2812, PCRE_UTF8, 0);

    irc_t *irc = new_irc(server, port, nick, username, realname,
                        true, true);
    irc_connect(irc);
    irc_send(irc, "NICK %s", nick);
    irc_send(irc, "USER %s 0 * :%s", username, realname);
    irc_join(irc, "###kupp_trash");
    char *recv_buff;
    char input_buff[IRC_BUFF_SIZE] = {0};
    bool quitflag = false;
    fcntl(stdin->_fileno, F_SETFL, O_NONBLOCK);
    int n;
    while (1)
    {
        if ((n = read(stdin->_fileno, input_buff,
                      IRC_BUFF_SIZE - 1)) > 0)
        {
            input_buff[n - 1] = '\0';
            irc_send(irc, "%s", input_buff);
        }
        if ((recv_buff = irc_read(irc)))
        {
            fputs(recv_buff, stdout);
            char **msgs = NULL;
            int msgs_count = strsplit(&msgs, recv_buff, "\r\n");
            char *msg;
            for (int i = 0; i < msgs_count; i++)
            {
                msg = msgs[i];
                int groups_count = pcre_do_regex(re, msg, 0, 0);
                if (groups_count > 7)
                {
                    if (strcmp("PING", re->sub_strs[VERB]) == 0)
                        irc_send(irc, "PONG %s",
                                 pcre_get_group(re, MSG,
                                                groups_count));
                    if (strcmp(host_nick,
                               pcre_get_group(re, NICK,
                                              groups_count)) == 0 &&
                        strcmp("PRIVMSG",
                               pcre_get_group(re, VERB,
                                              groups_count)) == 0 &&
                        strcasecmp("QUIT",
                                   pcre_get_group(re, MSG,
                                                  groups_count)) == 0)
                        quitflag = true;
                }
                free(msgs[i]);
                free_pcre_groups(re);
            }
            free(msgs);
        }
        if (quitflag || killflag)
        {
            irc_quit(irc, "bye!");
            free_pcre_regex(re);
            break;
        }
    }
    free_irc(irc);
    return 0;
}
