#pragma once

typedef struct sockaddr_in sockaddr_in_t;

typedef struct
{
    int const port, sockfd;
    char *nick;
    char const *username, *realname;
    sockaddr_in_t *servAddr;
} irc_t;

irc_t *newIrc(char const *server,
            int port,
            char const *nick,
            char const *username,
            char const *realname);
int freeIrc(irc_t *in);
int ircConnect(irc_t *in);
int ircSend(irc_t *in, char const *fmt, ...) __attribute__(
                                          (format (printf, 2, 3)));
char *ircRead(irc_t *in);

int join(irc_t *in, char const *channel);
int quit(irc_t *in, char const *msg);

extern char *RFC2812;
enum
{
    VAR,
    IDENT,
    NICK,
    USERNAME,
    HOST,
    VERB,
    PARAMS,
    MSG
};