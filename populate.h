#ifndef OPT_DEF
#define OPT_DEF	1

struct opt_cmd
{
  int noleft;
  int noright;
  int leftport;
  int rightport;
  int rraddr;
  int scrip;
  int persr;
  int persl;
  int dsplr;
  int dsprl;
  int outputl;
  int outputr;
  int loopr;
  int loopl;
  int dropl;
  int dropr;
};

// establish tcp protocol
// establish a hostent interface
// populate a sockaddr_in interface
struct sockaddr_in create_sockaddr_in(char *host, char *myprog, int port);

// create a specified socket
int create_socket(struct sockaddr_in ad);

#endif
