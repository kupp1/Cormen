#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdarg.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "irc.h"

SSL_CTX* InitCTX()
{   
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    return ctx;
}

irc_t *newIrc(char const *server,
              char const *port,
              char const *nick,
              char const *username,
              char const *realname,
              bool ssl)
{
    irc_t *out;
    struct addrinfo hints, *res;
    hints = (struct addrinfo){.ai_family = AF_UNSPEC,
                              .ai_socktype = SOCK_STREAM};
    if (getaddrinfo(server, port, &hints, &res))
        out = NULL;
    else
    {
        out = malloc(sizeof(irc_t));
        memcpy(out, &(irc_t){.sockfd = socket(res->ai_family,
                                              res->ai_socktype,
                                              res->ai_protocol),
                             .nick = (char*)nick,
                             .username = username,
                             .realname = realname,
                             .res = res,
                             .sslflag = ssl,
                             .ssl = NULL,
                             .ctx = NULL},
               sizeof(irc_t));
    } 
    return out;
}

int freeIrc(irc_t *in)
{
    freeaddrinfo(in->res);
    if (in->sslflag)
    {
        SSL_free(in->ssl);
        SSL_CTX_free(in->ctx);
    }
    free(in);
    return 0;
}

int ircConnect(irc_t *in)
{
    int n = connect(in->sockfd, in->res->ai_addr, in->res->ai_addrlen);
    if (in->sslflag)
    {
        SSL_library_init();
        SSL_CTX *ctx = InitCTX();
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, in->sockfd);
        SSL_connect(ssl);
        in->ssl = ssl;
        in->ctx = ctx;
    }
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
    if (in->sslflag)
         SSL_write(in->ssl, sendBuff, msgLen + 2);
    else
        send(in->sockfd, sendBuff, msgLen + 2, 0);
    return 0;
}

char *ircRead(irc_t *in)
{
    static char recvBuff[BUFF_SIZE];
    int n;
    if (in->sslflag)
        n = SSL_read(in->ssl, recvBuff, BUFF_SIZE);
    else 
        n = recv(in->sockfd, recvBuff, BUFF_SIZE - 1, 0);
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