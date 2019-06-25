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

SSL_CTX* init_ctx()
{   
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    return ctx;
}

irc_t *new_irc(char const *server,
              char const *port,
              char const *nick,
              char const *username,
              char const *realname,
              bool ssl, bool nonblockflag)
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
        *out = (irc_t){.sockfd = socket(res->ai_family,
                                        res->ai_socktype,
                                        res->ai_protocol),
                       .nick = nick,
                       .username = username,
                       .realname = realname,
                       .res = res,
                       .sslflag = ssl,
                       .nonblockflag = nonblockflag,
                       .ssl = NULL,
                       .ctx = NULL};
    } 
    return out;
}

int free_irc(irc_t *in)
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

int irc_set_socket_nonblocking(irc_t *in)
{
    return fcntl(in->sockfd, F_SETFL, O_NONBLOCK);
}

int irc_connect(irc_t *in)
{
    int n = connect(in->sockfd, in->res->ai_addr, in->res->ai_addrlen);
    if (in->sslflag)
    {
        SSL_library_init();
        SSL_CTX *ctx = init_ctx();
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, in->sockfd);
        SSL_connect(ssl);
        in->ssl = ssl;
        in->ctx = ctx;
    }
    if (in->nonblockflag)
        irc_set_socket_nonblocking(in);
    return n;
}

int irc_send(irc_t const *in, char const *fmt, ...)
{
    static char send_buff[IRC_BUFF_SIZE];
    va_list argp;
    va_start(argp, fmt);
    vsnprintf(send_buff, IRC_BUFF_SIZE, fmt, argp);
    va_end(argp);
    int msg_len = strlen(send_buff);
    if (msg_len > IRC_BUFF_SIZE - 2)
        return 1;
    send_buff[msg_len] = '\r';
    send_buff[msg_len + 1] = '\n';
    send_buff[msg_len + 2] = '\0';
    fputs(send_buff, stdout);
    if (in->sslflag)
         SSL_write(in->ssl, send_buff, msg_len + 2);
    else
        send(in->sockfd, send_buff, msg_len + 2, 0);
    return 0;
}

char *irc_read(irc_t const *in)
{
    static char recv_buff[IRC_BUFF_SIZE];
    int n;
    if (in->sslflag)
        n = SSL_read(in->ssl, recv_buff, IRC_BUFF_SIZE);
    else 
        n = recv(in->sockfd, recv_buff, IRC_BUFF_SIZE - 1, 0);
    if (n <= 0)
        return NULL;
    recv_buff[n] = '\0';
    return recv_buff;
}

int irc_quit(irc_t const *in, char const *msg)
{
    irc_send(in, "QUIT :%s", msg);
    return 0;
}

int irc_join(irc_t const *in, char const *channel)
{
    irc_send(in, "JOIN %s", channel);
    return 0;
}

char *rfc2812 =
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