#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pcre.h>
#include <locale.h>
#include <stdbool.h>
#include "irc.h"
#include "pcre_wrpr.h"
#include "strlib.h"

char *server = "irc.freenode.net";
int port = 6667;

char *nick = "Cormen";
char *username = "bot";
char *realname = "kupp bot";
char *hostNick = "kupp";

int main(int argc, char **argv)
{
    setlocale(LC_CTYPE, (char const *)"ru.");

    pcreRegex *re = makeRegex(RFC2812, PCRE_UTF8);

    irc_t *irc = newIrc(server, port, nick, username, realname);
    ircConnect(irc);
    ircSend(irc, "NICK %s", nick);
    ircSend(irc, "USER %s 0 * :%s", username, realname);
    join(irc, "###kupp_trash");
    char *recvBuff;
    bool quitFlag = false;
    while ((recvBuff = ircRead(irc)) != NULL)
    {
        fputs(recvBuff, stdout);
        char **msgs = NULL;
        int msgsCount = strsplit(&msgs, recvBuff, "\r\n");
        char *msg;
        for (int i = 0; i < msgsCount; i++)
        {
            msg = msgs[i];
            int pcreExecRet = doRegex(re, msg);
            if (pcreExecRet > 7)
            {
                if (strcmp("PING", re->subStrs[VERB]) == 0)
                    ircSend(irc, "PONG %s", re->subStrs[MSG]);
                if (strcmp(hostNick, re->subStrs[NICK]) == 0 &&
                    strcmp("PRIVMSG", re->subStrs[VERB]) == 0 &&
                    strcmp("QUIT", re->subStrs[MSG]) == 0)
                    quitFlag = true;
            }
            free(msgs[i]);
            freeGroups(re);
        }
        free(msgs);
        if (quitFlag)
        {
            quit(irc, "bye!");
            freeRegex(re);
            break;
        }
    }
    freeIrc(irc);
    return 0;
}
