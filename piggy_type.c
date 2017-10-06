#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>

//For ncurses functions
#include <locale.h>
#include <curses.h>
#include "populate.h"

#define QLEN 15 /* size of request queue */

void update_win(int i);

void head(int right_con, struct sockaddr_in ad, WINDOW* sw[])
{
  if(connect(right_con, (struct sockaddr *)&ad, sizeof(ad)) < 0) {
    nocbreak();
    endwin();
    perror("connect");
    exit(1);
  }
  wprintw(sw[6],"Status: right_connection established.\n");
  update_win(6);
  wmove(sw[4],0,10);
  update_win(4);
}

void middle(int left_con, int right_con,
            struct sockaddr_in lad, struct sockaddr_in rad, WINDOW* sw[])
{
  int rc; int flag = 1;

  /*Behaves like a head and connects to the tail*/
  if(connect(right_con, (struct sockaddr *)&rad, sizeof(rad)) < 0) {
    nocbreak();
    endwin();
    perror("Connect");
    exit(1);
  } else {
    wprintw(sw[6],"Status: Right connection established.\n");
    wprintw(sw[6],"Waiting for left connection...\n");
    update_win(6);
    wmove(sw[4],0,10);
    update_win(4);
  }

  /*Eliminate "Address already in use "error message."*/
  if(setsockopt(left_con,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(int))==-1){
    nocbreak();
    endwin();
    perror("setsockopt");
    exit(1);
  }

  // bind the connection
  if(bind(left_con, (struct sockaddr *)&lad, sizeof(lad)) < 0) {
    nocbreak();
    endwin();
    perror("Bind");
    exit(1);
  } else {
    wprintw(sw[6],"Status: Left connection ready.");
    update_win(6);
    wmove(sw[4],0,10);
    update_win(4);
    wprintw(sw[6],"\n");
  }

  //make connection passive and recv incoming connecitons
  rc = listen(left_con, QLEN);
  if(rc < 0) {
    nocbreak();
    endwin();
    perror("listen() failed\n");
    exit(1);
  }
}

void tail(int left_con, struct sockaddr_in ad, WINDOW* sw[])
{
  int rc; int flag = 1;

  if(setsockopt(left_con,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(int))==-1){
    nocbreak();
    endwin();
    perror("setsockopt");
    exit(1);
  }

  //bind the connection
  if(bind(left_con, (struct sockaddr *)&ad, sizeof(ad)) < 0) {
    nocbreak();
    endwin();
    perror("Bind");
    exit(1);
  } else {
    wprintw(sw[6],"Status: Server Ready.\n");
    update_win(6);
    wmove(sw[4],0,10);
    update_win(4);
  }

  //make connection passive and recv incoming connecitons
  rc = listen(left_con, QLEN);
  if(rc < 0) {
    nocbreak();
    endwin();
    perror("listen() failed\n");
    exit(1);
  }
}
