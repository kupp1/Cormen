#pragma once
#include <stdbool.h>
#include <openssl/ssl.h>
#include "irc_cmd_res.h"
#include "irc_err_rpls.h"
#define IRC_BUFF_SIZE 512

typedef struct irc_s
{
    int sockfd;
    struct addrinfo *res;
    char const *nick, *username, *realname;
    bool sslflag, nonblockflag;
    SSL *ssl;
    SSL_CTX *ctx;
} irc_t;

irc_t *new_irc(char const *server,
               char const *port,
               char const *nick,
               char const *username,
               char const *realname,
               bool ssl, bool nonblockflag);
int free_irc(irc_t *in);
int irc_connect(irc_t *in);
int irc_send(irc_t const *in, char const *fmt, ...) \
__attribute__((format(printf, 2, 3)));
char *irc_read(irc_t const *in);

int irc_join(irc_t const *in, char const *channel);
int irc_quit(irc_t const *in, char const *msg);

extern char *rfc2812;
enum irc_groups
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