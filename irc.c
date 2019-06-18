#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include "irc.h"

irc_t *newIrc(char const *server,
            int port,
            char const *nick,
            char const *username,
            char const *realname)
{
    irc_t *out = malloc(sizeof(irc_t));
    memcpy(out, &(irc_t){.port = port,
                        .sockfd = socket(AF_INET, SOCK_STREAM, 0),
                        .nick = (char*)nick, .username = username,
                        .realname = realname},
           sizeof(irc_t));
    struct hostent *host = gethostbyname(server);
    out->servAddr = malloc(sizeof(sockaddr_in_t));
    memcpy(out->servAddr, 
           &(sockaddr_in_t){.sin_addr.s_addr = *(unsigned long *)host->h_addr_list[0],
                            .sin_family = AF_INET,
                            .sin_port = htons(port)},
           sizeof(sockaddr_in_t));
    return out;
}

int freeIrc(irc_t *in)
{
    free(in->servAddr);
    free(in);
    return 0;
}

int ircConnect(irc_t *in)
{
    int n = connect(in->sockfd, (struct sockaddr *)in->servAddr,
                   sizeof(*in->servAddr));
    fcntl(in->sockfd, F_SETFL, O_NONBLOCK);
    return n;
}

int ircSend(irc_t *in, char const *fmt, ...)
{
    static char sendBuff[BUFF_SIZE];
    va_list argp;
    va_start(argp, fmt);
    vsnprintf(sendBuff, BUFF_SIZE, fmt, argp);
    va_end(argp);
    int msgLen = strlen(sendBuff);
    if (msgLen > BUFF_SIZE - 2)
        return 1;
    sendBuff[msgLen] = '\r';
    sendBuff[msgLen + 1] = '\n';
    sendBuff[msgLen + 2] = '\0';
    fputs(sendBuff, stdout);
    send(in->sockfd, sendBuff, msgLen + 2, 0);
    return 0;
}

char *ircRead(irc_t *in)
{
    static char recvBuff[BUFF_SIZE];
    int n = read(in->sockfd, recvBuff, BUFF_SIZE - 1);
    if (n <= 0)
        return NULL;
    recvBuff[n] = '\0';
    return recvBuff;
}

int ircQuit(irc_t *in, char const *msg)
{
    ircSend(in, "QUIT :%s", msg);
    return 0;
}

int ircJoin(irc_t *in, char const *channel)
{
    ircSend(in, "JOIN %s", channel);
    return 0;
}

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