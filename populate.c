#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <locale.h>
#include <curses.h>

#define HEAD 0
#define MIDDLE 1
#define TAIL 2


struct sockaddr_in create_sockaddr_in(char *host, char *myprog, int port) {
  struct hostent *hp;
  struct sockaddr_in ad;

  if((getservbyname("whois", "tcp")) == NULL) {
    fprintf(stderr, "%s: No whois service on this host\n", myprog);
    exit(1);
  }
  if((hp = gethostbyname(host)) == NULL) {
    fprintf(stderr, "%s: cannot get local host info?\n", myprog);
    exit(1);
  }
  ad.sin_port = htons((u_short)port);
  bcopy((char *)hp->h_addr, (char *)&ad.sin_addr,hp->h_length);
  ad.sin_family = hp->h_addrtype;

  return ad;
}

int create_socket(struct sockaddr_in ad) {
  int sd;

  if((sd = socket(ad.sin_family, SOCK_STREAM, 0)) < 0){
    perror("socket");
    exit(1);
  }
  return sd;
}
