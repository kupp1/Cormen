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

char *server = "irc.run.net";
int port = 6660;

char *nick = "Cormen";
char *username = "bot";
char *realname = "kupp bot";

int join(int sockfd, char *Buff, char *channel);
int quit(int sockfd, char *Buff, char *channel);
int sendFromBuff(int sockfd, char *Buff);

typedef struct
{
    char *regex;
    pcre *reCompiled;
    pcre_extra *pcreExtra;
    int subStrVec[30];
} prceRegex;

prceRegex makeRegex(char *regex, const unsigned char *pcreTables);
int execRegex(prceRegex *regex, char *str);

int main(int argc, char *argv[]) 
{
    int pcreExecRet = 0;
    //int subStrVec[30];
    //const char *pcreErrorStr;
    //int pcreErrorOffset;
    const char *psubStrMatchStr;
    const unsigned char *pcreTables = NULL;
         
    setlocale (LC_CTYPE, (const char *) "ru.");
    pcreTables = pcre_maketables();
    char *RFC2812 =
        "^" // We'll match the whole line. Start.
            // Optional prefix and the space that separates it
            // from the next thing. Prefix can be a servername,
            // or nick[[!user]@host]
        "(?::("                // This whole set is optional but if it's
                               // here it begins with : and ends with space
            "([^@! ]*)"        // nick
            "(?:"              // then, optionally user/host
                "(?:"          // but user is optional if host is given
                    "!([^@]*)" // !user
                ")?"           // (user was optional)
                "@([^ ]*)"     // @host
            ")?"               // (host was optional)
        ") )?"                 // ":nick!user@host " ends
        "([^ ]+)"              // IRC command (required)
        // Optional args, max 15, space separated. Last arg is
        // the only one that may contain inner spaces. More than
        // 15 words means remainder of words are part of 15th arg.
        // Last arg may be indicated by a colon prefix instead.
        // Pull the leading and last args out separately; we have
        // to split the former on spaces.
        "("
            "(?:"
                " [^: ][^ ]*" // space, no colon, non-space characters
            "){0,14}"         // repeated up to 14 times
        ")"                   // captured in one reference
        "(?: :?(.*))"    // the rest, does not capture colon.
        "$"; // EOL
    prceRegex re = makeRegex(RFC2812, pcreTables);
    prceRegex *reP = &re;

    /* char *str = "PING :irc.run.net\r\n";
    pcreExecRet = pcre_exec(re, pcreExtra, str, strlen(str), 0, 0, subStrVec, 30);
    printf("%i\n", pcreExecRet);
    if (!pcreExecRet) {
        printf("No match\n");
    } else {
        for (int j = 0; j < pcreExecRet; j++) 
        {
            pcre_get_substring(str, subStrVec, pcreExecRet, j, &(psubStrMatchStr));
            printf("Match(%2d/%2d): (%2d,%2d): '%s'\n", j, pcreExecRet-1, subStrVec[j*2], subStrVec[j*2+1], psubStrMatchStr);
        }
    } */
    int sockfd = 0, n = 0;
    char recvBuff[512];
    char sendBuff[512];
    struct sockaddr_in serv_addr;
    struct hostent *host;

    memset(recvBuff, '\0', sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    if ( (host = gethostbyname(server)) == NULL )
    {
        printf("\n Invalid host\n");
        return 1;
    }

    serv_addr.sin_addr.s_addr = *(unsigned long*)host->h_addr_list[0];
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    }

    sprintf(sendBuff, "NICK %s", nick);
    sendFromBuff(sockfd, sendBuff);
    sprintf(sendBuff, "USER %s 0 * :%s", username, realname);
    sendFromBuff(sockfd, sendBuff);
    join(sockfd, sendBuff, "#16bits");
    //join(sockfd, sendBuff, "#16bit");
    while ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {
        recvBuff[n] = 0;
        if(fputs(recvBuff, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
        }
        pcreExecRet = execRegex(reP, recvBuff);
        if (pcreExecRet > 0)
        {
            pcre_get_substring(recvBuff, reP->subStrVec, pcreExecRet, 5, &(psubStrMatchStr));
            if (strcmp("PING", psubStrMatchStr) == 0)
            {
                pcre_get_substring(recvBuff, reP->subStrVec, pcreExecRet, 7, &(psubStrMatchStr));
                sprintf(sendBuff, "PONG %s", psubStrMatchStr);
                sendFromBuff(sockfd, sendBuff);
            }
        }
    }

    if(n < 0)
    {
        printf("\n Read error \n");
    }

    return 0;
}

int sendFromBuff(int sockfd, char *Buff)
{
    if (strlen(Buff) < 510)
    {
        strcat(Buff, "\r\n");
        send(sockfd, Buff, strlen(Buff), 0);
        printf("Send: %s", Buff);
        return 0;
    } else {
        return 1;
    }
}

int quit(int sockfd, char *Buff, char *message)
{
    sprintf(Buff, "QUIT %s", message);
    sendFromBuff(sockfd, Buff);
    return 0;
}

int join(int sockfd, char *Buff, char *channel)
{
    sprintf(Buff, "JOIN %s", channel);
    sendFromBuff(sockfd, Buff);
    return 0;
}

prceRegex makeRegex(char *regex, const unsigned char *pcreTables)
{
    const char *pcreErrorStr;
    int pcreErrorOffset;
    pcre *tmpReCompiled;
    pcre_extra *tmpPcreExtra;
    tmpReCompiled = pcre_compile(regex, 0, &pcreErrorStr, &pcreErrorOffset, pcreTables);
    if(tmpReCompiled == NULL) 
    {
        printf("ERROR: Could not compile regex '%s': %s\n", regex, pcreErrorStr);
        pcre_free(tmpReCompiled);
        return (prceRegex){};
    }
    tmpPcreExtra = pcre_study(tmpReCompiled, 0, &pcreErrorStr);
    if(pcreErrorStr != NULL) 
    {
        printf("ERROR: Could not study regex '%s': %s\n", regex, pcreErrorStr);
        pcre_free(tmpReCompiled);
        pcre_free_study(tmpPcreExtra);
        return (prceRegex){};
    }
    return (prceRegex){.regex = regex, .reCompiled = tmpReCompiled, .pcreExtra = tmpPcreExtra};
}

int execRegex(prceRegex *regex, char *str)
{
    return pcre_exec(regex->reCompiled, regex->pcreExtra, str, strlen(str), 0, 0, regex->subStrVec, 30);
}
