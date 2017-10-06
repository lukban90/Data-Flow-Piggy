#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include <locale.h>
#include <curses.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "populate.h"

#define BUFSIZE 1024
#define MAXSLEEP 10
#define ADDRSIZE 32
#define HEAD 0
#define MIDDLE 1
#define TAIL 2

void update_win(int i);

struct sockaddr_in peer_addr(int socket);

struct sockaddr_in local_addr(int socket);


void process_command(char* command, struct opt_cmd *opt_t, WINDOW* sw[],int piggy, int new_sd,int left_con, int right_con){

  if(strcmp(command,"persr") == 0){
    if(opt_t->persr == 1){
      wprintw(sw[6],"Piggy2 : persr flag was set previously.\n");
    } else {
      opt_t->persr = 1;
      wprintw(sw[6],"Piggy2 : persr flag set.\n");
    }
    update_win(6);
    wmove(sw[4],0,10);
    update_win(4);
  } else if(strcmp(command,"dsplr") == 0){
    if(opt_t->dsplr == 1){
      wprintw(sw[6],"Piggy2 : dsplr flag was set previously.\n");
    } else {
      opt_t->dsplr = 1;
      opt_t->dsprl = 0; //reset the flag
      wprintw(sw[6],"Piggy2 : dsplr flag set.\n");
    }
    update_win(6);
    wmove(sw[4],0,10);
    update_win(4);
  } else if (strcmp(command,"dsprl") == 0){
    if(opt_t->dsprl == 1){
      wprintw(sw[6],"Piggy2 : dsprl flag was set previously. \n");
    } else {
      opt_t->dsprl = 1;
      opt_t->dsplr = 0;
      wprintw(sw[6],"Piggy : dsprl flag set. \n");
    }
    update_win(6);
    wmove(sw[4],0,10);
    update_win(4);
  } else if(strcmp(command,"loopr") == 0){
    if(opt_t->loopr == 1){
      wprintw(sw[6],"Piggy2 : loopr flag was set previously.\n");
    } else {
      opt_t->loopr = 1;
      wprintw(sw[6],"Piggy2 : loopr flag set.\n");
    }
    update_win(6);
    wmove(sw[4],0,10);
    update_win(4);
  } else if(strcmp(command,"loopl") == 0){
    if(opt_t->loopl == 1){
      wprintw(sw[6],"Piggy2 : loopl flag was set previously.\n");
    } else {
      opt_t->loopl = 1;
      wprintw(sw[6],"Piggy2 : loopl flag set.\n");
    }
    update_win(6);
    wmove(sw[4],0,10);
    update_win(4);
  } else if(strcmp(command,"outputl") == 0){
    if(opt_t->outputl == 1){
      wprintw(sw[6],"Piggy2 : outputl flag was set previously.\n");
    } else {
      opt_t->outputl = 1;
      opt_t->outputr = 0; //reset the flag
      wprintw(sw[6],"Piggy2 : outputl flag set.\n");
    }
    update_win(6);
    wmove(sw[4],0,10);
    update_win(4);
  } else if(strcmp(command,"outputr") == 0){
    if(opt_t->outputr == 1){
      wprintw(sw[6],"Piggy2 : outputr flag was set previously.\n");
    } else {
      opt_t->outputr = 1;
      opt_t->outputl = 0; //reset the flag
      wprintw(sw[6],"Piggy2 : outputr flag set.\n");
    }
    update_win(6);
    wmove(sw[4],0,10);
    update_win(4);
  } else if(strcmp(command,"dropl") == 0){
    wprintw(sw[6],"Left connection dropped\n");
    close(new_sd);
  } else if(strcmp(command,"dropr") == 0){
    wprintw(sw[6],"Right connection dropped\n");
    close(right_con);
  } else if(strcmp(command,"right") == 0 && (piggy == HEAD || piggy == MIDDLE)){

    char peer_name[ADDRSIZE];
    struct sockaddr_in peer = peer_addr(right_con);
    struct sockaddr_in local = local_addr(right_con);

    //convert network addr to a string
    inet_ntop(AF_INET, &(peer.sin_addr), &peer_name[0], ADDRSIZE);

    wprintw(sw[6], "%s:%d:%s:%d\n",
    inet_ntoa(local.sin_addr), (int)ntohs(local.sin_port),
    inet_ntoa(peer.sin_addr), (int)ntohs(peer.sin_port));
    update_win(6);
    wmove(sw[4],0,10);
    update_win(4);


  } else if(strcmp(command,"left") == 0 && (piggy == MIDDLE || piggy == TAIL)){


    char peer_name[ADDRSIZE];
    struct sockaddr_in peer = peer_addr(new_sd);
    struct sockaddr_in local = local_addr(new_sd);

    //convert network addr to a string
    inet_ntop(AF_INET, &(peer.sin_addr), &peer_name[0], ADDRSIZE);

    //remote:IP:remote port:local IP:local port:
    wprintw(sw[6], "%s:%d:%s:%d\n",
    inet_ntoa(peer.sin_addr), (int)ntohs(peer.sin_port),
    inet_ntoa(local.sin_addr), (int)ntohs(local.sin_port));
    update_win(6);
    wmove(sw[4],0,10);
    update_win(4);
  } else if (strcmp(command,"clear") == 0){
    wclear(sw[0]);
    wclear(sw[1]);
    wclear(sw[2]);
    wclear(sw[3]);
    update_win(0);
    update_win(1);
    update_win(2);
    update_win(3);
  } else if (strcmp(command,"q") == 0) {
    nocbreak();
    endwin();
    printf("Piggy: Exited\n");
    exit(1);
  } else {
    wprintw(sw[6],"Invalid Command: %s\n",command);
    update_win(6);
    wmove(sw[4],0,10);
    update_win(4);
  }

}
