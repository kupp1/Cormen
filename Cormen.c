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
char *port = "6667";

char *nick = "Cormen";
char *username = "bot";
char *realname = "kupp bot";
char *hostNick = "kupp";

static bool killflag = false;

void sigintHandler(int dummy)
{
    killflag = true;
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigintHandler);

    setlocale(LC_CTYPE, (char const *)"ru.");

    pcreRegex_t *re = compileRegex(RFC2812, PCRE_UTF8);

    irc_t *irc = newIrc(server, port, nick, username, realname);
    ircConnect(irc);
    ircSend(irc, "NICK %s", nick);
    ircSend(irc, "USER %s 0 * :%s", username, realname);
    ircJoin(irc, "###kupp_trash");
    char *recvBuff;
    char inputBuff[BUFF_SIZE] = {0};
    bool quitFlag = false;
    fcntl(stdin->_fileno, F_SETFL, O_NONBLOCK);
    int n;
    while (1)
    {
        if ((n = read(stdin->_fileno, inputBuff, BUFF_SIZE - 1)) > 0)
        {
            inputBuff[n - 1] = '\0';
            ircSend(irc, "%s", inputBuff);
        }
        if ((recvBuff = ircRead(irc)))
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
                        strcasecmp("QUIT", re->subStrs[MSG]) == 0)
                        quitFlag = true;
                }
                free(msgs[i]);
                freeGroups(re);
            }
            free(msgs);
        }
        if (quitFlag || killflag)
        {
            ircQuit(irc, "bye!");
            freeRegex(re);
            break;
        }
    }
    freeIrc(irc);
    return 0;
}
