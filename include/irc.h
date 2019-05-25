#pragma once

int join(int sockfd, char *Buff, char *channel);
int quit(int sockfd, char *Buff, char *channel);
int sendFromBuff(int sockfd, char *Buff);

char *RFC2812;
enum {VAR, IDENT, NICK, USERNAME, HOST, VERB, PARAMS, MSG};