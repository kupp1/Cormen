#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <irc.h>

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