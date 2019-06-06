#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pcre.h>
#include <locale.h>
#include "irc.h"
#include "pcre_wrpr.h"
#include "strlib.h"

char *server = "irc.freenode.net";
int port = 6667;

char *nick = "Cormen";
char *username = "bot";
char *realname = "kupp bot";
char *hostNick = "kupp";

int main(int argc, char *argv[])
{

    setlocale(LC_CTYPE, (char const *)"ru.");

    pcreRegex re = makeRegex(RFC2812);
    pcreRegex *reP = &re;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        puts("Error : Could not create socket");
        return 1;
    }
    struct hostent *host = gethostbyname(server);
    if (host == NULL)
    {
        puts("Invalid host");
        return 1;
    }

    struct sockaddr_in servAddr;
    servAddr.sin_addr.s_addr = *(unsigned long *)host->h_addr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    char recvBuff[512];
    char sendBuff[512];
    sprintf(sendBuff, "NICK %s", nick);
    sendFromBuff(sockfd, sendBuff);
    sprintf(sendBuff, "USER %s 0 * :%s", username, realname);
    sendFromBuff(sockfd, sendBuff);
    join(sockfd, sendBuff, "###kupp_trash");
    int n;
    int quitFlag = 0;
    while ((n = read(sockfd, recvBuff, sizeof(recvBuff) - 1)) > 0)
    {
        recvBuff[n] = 0;
        fputs(recvBuff, stdout);
        char **msgs = NULL;
        int msgsCount = strsplit(&msgs, recvBuff, "\r\n");
        char *msg;
        for (int i = 0; i < msgsCount; i++)
        {
            msg = msgs[i];
            int pcreExecRet = doRegex(reP, msg);
            if (pcreExecRet > 7)
            {
                if (strcmp("PING", reP->subStrs[VERB]) == 0)
                {
                    sprintf(sendBuff, "PONG %s", reP->subStrs[MSG]);
                    sendFromBuff(sockfd, sendBuff);
                }
                if (strcmp(hostNick, reP->subStrs[NICK]) == 0 &&
                    strcmp("PRIVMSG", reP->subStrs[VERB]) == 0 &&
                    strcmp("QUIT", reP->subStrs[MSG]) == 0)
                    quitFlag = 1;
            }
            free(msgs[i]);
            freeGroups(reP);
        }
        free(msgs);
        if (quitFlag)
        {
            freeRegex(reP);
            break;
        }
    }

    if (n < 0)
    {
        puts("Read error");
    }

    return 0;
}
