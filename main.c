/*************************************|
* Program: pigg2.c                   *|
* Author: Chris Lukban, Jose Avalos  *|
* Data: July 2, 2017                 *|
*///**********************************/

//tar -cf myfiles.tar *.c *.h Makefile
//tar -tf myfiles.tar

#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <termios.h> // for insert mode

//Libraries for ncurses
#include <locale.h>
#include <curses.h>

#include "populate.h"
#include "piggy_type.h"
#include "select_sockets.h"


#define PROTOPORT 36729 // default protocol port number
#define MAXHOSTNAME 32 // standard size IPV4 addr
#define BUFSIZE 1024
#define HEAD 0
#define MIDDLE 1
#define TAIL 2
#define NUMWINS 8
#define RES_BUF_SIZE 80

WINDOW *w[NUMWINS];
WINDOW *sw[NUMWINS];

void update_win(int i){
  touchwin(w[i]);
  wrefresh(sw[i]);
}

int main(int argc, char *argv[])
{

  char *myprog; //name of program
  char *host; //ip addr
  char local_host[MAXHOSTNAME+1]; // local host of this machine
  int rport, lport = 0; //valued ports
  FILE *myfile;

//parameter options for get_opts
  static struct option long_options[] =
  {
    {"noleft",    no_argument,       0,  'a' },
    {"noright",   no_argument,       0,  'b' },
    {"rraddr",    required_argument, 0,  'c' },
    {"s",         optional_argument, 0,  'd' },
    {"rrport",    required_argument, 0,  'e' },
    {"llport",    required_argument, 0,  'f' },
    {"persl",     no_argument,       0,  'g' },
    {"persr",     no_argument,       0,  'h' },
    {"dsplr",     no_argument,       0,  'i' },
    {"dsprl",     no_argument,       0,  'j' },
    {"loopl",     no_argument,       0,  'k' },
    {"loopr",     no_argument,       0,  'l' },
    {0,           0,                 0,  0   }
  };

  int opt; //opt type
  struct opt_cmd opt_t = { 0 }; //flags
  //find all options that user inputed
  while((opt = getopt_long_only(argc, argv, "", long_options, NULL)) != -1)
  {
    if(opt < 0) break;

    switch (opt) {
      case 'a':
        opt_t.noleft = 1;
        break;
      case 'b':
        opt_t.noright = 1;
        break;
      case 'c':
        opt_t.rraddr = 1;
        host = optarg;
        break;
      case 'd':
        opt_t.scrip = 1;
        /*************************/
        /* check optind is valid */
        /* not a null string     */
        /* not another option    */
        /*************************/
        if( optarg == NULL
           && argv[optind] != NULL
           && argv[optind][0] != '-' ) {
             myfile = fopen(argv[optind], "r" );
             ++optind;
        } else
          myfile = fopen("scriptin.txt", "r");
        break;
      case 'e':
        opt_t.rightport = 1;
        rport = atoi(optarg);
        printf("Right Port: %d\n", rport);
        break;
      case 'f':
        opt_t.leftport = 1;
        lport = atoi(optarg);
        printf("Left Port: %d\n", lport);
        break;
      case 'g':
        opt_t.persl = 1; break;
      case 'h':
        opt_t.persr = 1; break;
      case 'i':
        opt_t.dsplr = 1; break;
      case 'j':
        opt_t.dsprl = 1; break;
      case 'k':
        opt_t.loopl = 1; break;
      case 'l':
        opt_t.loopr = 1; break;
      default:
        printf("Error options\n");
        break;
    } //end of switch case
  } //end of while loop for getopt_long_only

  myprog = argv[0]; // './main ** ** ' run program

  //ncurses mode begin, creating window and subwindows...
  int i;
  char response[RES_BUF_SIZE];
  int WPOS[NUMWINS][4] = { {16,66,0,0}, {16,66,0,66}, {16,66,16,0}, {16,66,16,66}, {3,132,32,0},
                           {5,66,35,0}, {5,66,35,66},{3,132,40,0} };

  setlocale(LC_ALL,"");
  initscr();
  cbreak();
  intrflush(stdscr,FALSE);
  keypad(stdscr,TRUE);
  clear();
  refresh(); // maybe we need it. unsure

  if( !(LINES == 43) || !(COLS == 132)){
    move(0,0);
    addstr("Piggy3 requires a screen size of 132 columns and 43 rows");
    move(1,0);
    addstr("Set screen size to 132 by 43 and try again");
    move(2,0);
    addstr("Press enter to terminate program");
    refresh();
    getstr(response);            // Pause so we can see the screen
    endwin();
    exit(EXIT_FAILURE);
  }

  // create the 7 windows ant the seven subwindows
  for (i=0;i<NUMWINS;i++) {
      w[i]=newwin(WPOS[i][0],WPOS[i][1],WPOS[i][2],WPOS[i][3]);
      sw[i]=subwin(w[i],WPOS[i][0]-2,WPOS[i][1]-2,WPOS[i][2]+1,WPOS[i][3]+1);
      scrollok(sw[i],TRUE);
      wborder(w[i],0,0,0,0,0,0,0,0);
      touchwin(w[i]);
      wrefresh(w[i]);
      wrefresh(sw[i]);
    }

    // Write some stuff to the windows
    wmove(sw[0],0,0);
    waddstr(sw[0],"Data arriving from the left\n");
    wmove(sw[1],0,0);
    waddstr(sw[1],"Data leaving right side\n");
    wmove(sw[2],0,0);
    waddstr(sw[2],"Data leaving the left side\n");
    wmove(sw[3],0,0);
    waddstr(sw[3],"Data arriving from the right\n");
    wmove(sw[4],0,0);
    waddstr(sw[4],"Commands: ");
    wmove(sw[5],0,0);
    waddstr(sw[5],"Input: ");
    wmove(sw[6],0,0);
    waddstr(sw[6],"Log: \n");
    wmove(sw[7],0,0);
    waddstr(sw[7],"Errors: ");

    for (i=0;i<NUMWINS;i++){
      update_win(i);
    }

    wmove(sw[4],0,10);
    update_win(4);

  /****************************************/
  /* HEAD                                 */
  /****************************************/
  if(opt_t.noleft == 1 && opt_t.rraddr == 1)
  {
    struct sockaddr_in ad;
    int right_con = 0; //right connection socket

    //populate sockaddr_in with tcp and hostent hp
    ad = create_sockaddr_in(host, myprog, PROTOPORT);
    right_con = create_socket(ad); // create the right the connection socket
    head(right_con, ad, sw); //create head piggy and connect to middle
    //MAKE A CALL TO SELECT HERE
    select_sd(0, right_con, HEAD, ad , opt_t, myfile,w, sw);

    //END OF HEAD CONDITION

    /****************************************/
    /* MIDDLE                               */
    /****************************************/
  } else if(opt_t.noleft == 0 && opt_t.noright == 0 && opt_t.rraddr == 1) {
    struct sockaddr_in lad, rad; // lad -> left addr | rad -> right addr
    int left_con, right_con = 0; // left and right connection sockets

    /*|||||||||||||POPULATE ALL RIGHT AFFILIATED CONS|||||||||||||*/

    //specify port if rightport flag is up
    if(opt_t.rightport == 1) {
      rad = create_sockaddr_in(host, myprog, rport);
    } else {
      rad = create_sockaddr_in(host, myprog, PROTOPORT);
    }

    right_con = create_socket(rad); // create the right connection socket

    /*|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*/

    /*|||||||||||||POPULATE ALL LEFT AFFILIATED CONS|||||||||||||*/

    //specify port if leftport flag is up
    gethostname(local_host, MAXHOSTNAME+1);
    if(opt_t.leftport == 1) {
      lad = create_sockaddr_in(host, myprog, lport);
    } else {
      lad = create_sockaddr_in(local_host, myprog, PROTOPORT);
    }
    left_con = create_socket(lad); // create the left connection socket

    /*|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*/

    middle(left_con, right_con, lad, rad, sw); //create middle piggy and connect to tail
    //MAKE A CALL TO SELECT HERE
    select_sd(left_con, right_con, MIDDLE, rad, opt_t, myfile,w,  sw);

    //END OF MIDDLE CONDITION

    /****************************************/
    /* TAIL                                 */
    /****************************************/
  } else if(opt_t.noright == 1) {
    struct sockaddr_in ad;
    int left_con = 0; //right connection socket

    gethostname(local_host, MAXHOSTNAME+1);
    //populate sockaddr_in with tcp and hostent hp
    ad = create_sockaddr_in(local_host, myprog, PROTOPORT);
    left_con = create_socket(ad); // create the left connection socket
    tail(left_con, ad,sw); //create tail piggy and connect to middle

    //MAKE A CALL TO SELECT HERE
    select_sd(left_con, 0, TAIL, ad , opt_t, myfile ,w, sw);

  } else if(argc==1 && (strcmp(argv[0], "./main") == 0)) {
    printf("Created the program\n");
  }

return 0;
}
