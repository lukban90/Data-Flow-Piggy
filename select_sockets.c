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
#include "commands.h"

#define BUFSIZE 1024
#define HIST_LEN 10
#define MAXSLEEP 10
#define ADDRSIZE 32
#define HEAD 0
#define MIDDLE 1
#define TAIL 2

void update_win(int i);

int rc_write(int rc) {
  if (rc < 0){
    perror("send() failed"); return 1;
  }
  return 0;
}

int rc_read(int rc) {
  if(rc==0) {
    printw("Connection closed\n"); return 1;
  }
  return 0;
}

struct sockaddr_in peer_addr(int socket) {
  socklen_t len;
  struct sockaddr_in peer;
  len = sizeof(peer);
  if(getpeername(socket, (struct sockaddr*)&peer, &len) < 0) {
    nocbreak();
    endwin();
    perror("getpeername() failed\n");
    exit(1);
  }
  return peer;
}

struct sockaddr_in local_addr(int socket) {
  socklen_t len;
  struct sockaddr_in local;

  len = sizeof(local);
  if(getsockname(socket, (struct sockaddr*)&local, &len) < 0) {
    nocbreak();
    endwin();
    perror("getsockname() failed\n");
    exit(1);
  }
  return local;
}

// populate the master set
fd_set create_master(int left_con, int right_con, int keyboard, int piggy) {
  fd_set master_set;

  /*initialize the master fd_set*/
  FD_ZERO(&master_set);
  if(piggy == HEAD) {
    FD_SET(right_con, &master_set);
  } else if(piggy == MIDDLE) {
    FD_SET(left_con, &master_set);
    FD_SET(right_con, &master_set);
  } else if (piggy == TAIL) {
    FD_SET(left_con, &master_set);
  }
  FD_SET(keyboard, &master_set);
  return master_set;
}




void select_sd(int left_con, int right_con, int piggy,
               struct sockaddr_in ad, struct opt_cmd opt_t,FILE* myfile, WINDOW* w[], WINDOW* sw[])
{
  fd_set master_set, working_set;
  char *history[BUFSIZE]; int count=0;
  char buf[BUFSIZE] = { 0 };
  int max_sd, new_sd;
  int keyboard = fileno(stdin);
  int desc_ready, close_conn; // descriptor read to be read, terminate conn
  int end_server = 0;
  int rc = 0; int c = 0;
  int y = 1;
  int x, z = 0;

  //initialize each
  // for(int i; i < HIST_LEN; i++)
  // {
  //   history[i]=malloc(BUFSIZE+1);
  //   history[i][BUFSIZE+1]=0;
  // }

  //initialize max_sd, right_con, and left_con
  if(piggy == HEAD) {
    max_sd = right_con;
    left_con = max_sd+1; //ERROR HANDLE FOR LISTEN WHILE HEAD

  } else if(piggy == MIDDLE) {
    if(left_con > right_con) max_sd = left_con;
    else max_sd = right_con;

  } else if(piggy == TAIL) {
    max_sd = left_con;
    right_con = max_sd+1; //ERROR HANDLE FOR LISTEN WHILE HEAD
  }
  //initialize the master set
  master_set = create_master(left_con, right_con, keyboard, piggy);
  //put all left connections non-blocking
  fcntl(left_con,F_SETFL,O_NONBLOCK);

  if(opt_t.scrip == 1){
    while(fread(buf,sizeof(char),1,myfile) == 1){
      if(feof(myfile)){
         break;
      }
      send(right_con,buf,1,0);
   }
   opt_t.scrip = 0;
   fclose(myfile);
  }

  do {

    //copy master_set over to working_set
    memcpy(&working_set, &master_set, sizeof(master_set));

    //check if select failed
    if((rc = select(max_sd+1, &working_set, NULL, NULL, NULL)) < 0) {
      perror(" select() failed"); break;
    }

    desc_ready = rc;
    for(int i = 0; i <= max_sd && desc_ready > 0; i++) {
      if(FD_ISSET(i, &working_set)) {
        desc_ready--;

        /******************************************/
        /*process any new connections and accept  */
        /******************************************/
        if(i == left_con) {
          printw("Listening socket is readable\n");
          do {
            new_sd = accept(left_con, NULL, NULL);
            if(new_sd < 0) {
              if(errno != EWOULDBLOCK) {
                perror("accept() failed");
                end_server = 1;
              } break;
            }
            printw("New incoming connection - %d\n", new_sd);
            FD_SET(new_sd,&master_set);

            if(new_sd > max_sd) max_sd = new_sd;
          } while(new_sd != -1);
          new_sd = max_sd;
        } //end of if 'any new connections'

        else {
          memset(buf,0,sizeof(buf));
          close_conn = 0;

          do {
            //handle connections from right or left
            if(i==right_con || i==max_sd) {

              while(rc > 0) {
                rc = recv(i,buf,1,0);
                int bspace = (int) buf[0];

                if(bspace == 127){
                  x--;
                  mvwdelch(sw[0],y,x);
                  mvwdelch(sw[0],y,x);
                  mvwdelch(sw[0],y,x);
                  wrefresh(sw[0]);
                } else if(buf[0] == '\n'){
                  y++; x = 0;
                  wprintw(sw[0],"\n");
                  wmove(sw[0],y,x);
                  update_win(0);
                } else {
                  if(i == max_sd && opt_t.dsplr == 1 && piggy == MIDDLE) printw("%s", buf);
                  else if(i == max_sd && opt_t.dsprl == 1 && piggy == MIDDLE) printw("");
                  else if(i == right_con && opt_t.dsprl == 1 && piggy == MIDDLE) printw("%s", buf);
                  else if(i == right_con && opt_t.dsplr == 1 && piggy == MIDDLE) printw("");
                  else {
                    if(piggy==HEAD && i==right_con) {
                      mvwaddch(sw[3],y,x,buf[0]);
                      update_win(3);
                    } else if(piggy==MIDDLE) {
                      if(i==max_sd) {
                        mvwaddch(sw[0],y,x,buf[0]);
                        update_win(0);
                      } else if(i==right_con) {
                        mvwaddch(sw[3],y,x,buf[0]);
                        update_win(3);
                      }
                    } else if(piggy==TAIL && i==right_con) {
                      mvwaddch(sw[0],y,x,buf[0]);
                      update_win(0);
                    }
                    x++;
                  }
                }

                if((close_conn=rc_read(rc))) break;

                /////****************LOOPR BLOCK***************************////////
                if(piggy==HEAD && i==right_con && opt_t.loopr==1)
                  rc = send(right_con,buf,rc,0);

                else if(piggy == MIDDLE) {
                  if(i==right_con) {
                    if(opt_t.loopr==1)
                      rc = send(right_con,buf,rc,0);
                    else
                      rc = send(new_sd,buf,rc,0);
                    if((close_conn = rc_write(rc))) break;

                  } else if(i==max_sd){
                    if(opt_t.loopl==1)
                      rc = send(max_sd,buf,rc,0);
                    else
                      rc = send(right_con,buf,rc,0);
                    if((close_conn = rc_write(rc))) break;
                  }
                }
                else if(piggy == TAIL && i==new_sd && opt_t.loopl==1)
                  rc = send(new_sd,buf,rc,0);
                /////****************END LOOPR BLOCK***********************////////

                if(strcmp(buf,"\n") == 0) break;
                memset(buf,0,sizeof(buf));
                }

                // This break allows to go back to select
                // once we are done reading from the keyboard
                memset(buf,0,sizeof(buf));
                break;

                /*******************************************/
                /* end of 'if(i == right_con || i == max_sd)' */
                /* if data is coming from keyboard,        */
                /* send data to corresponding piggy type   */
              /*******************************************/
            } else if(i==keyboard) {

              int ci;
              // X axis needed, but though unnecessary
              // I added the y axis to be consistent
              int x = 0; int y = 0;

              // First, it accumulates all the characters read
              // and puts them into a string
              fflush(stdout);
              do {
                // get character from window 5
                ci = wgetch(sw[4]);
                // Backspace funcionality
                // We store characters in an array and use the variable
                // to iterate through it to find my position.
                // del 3 characters: One for the character erased
                // and two for the backspace character ^G
                if( ci == 127){
                  buf[x] = 0;
                  x--;
                  if(x < 0){
                    x = 0;
                  }
                  mvwdelch(sw[4],0,10+x);
                  mvwdelch(sw[4],0,10+x);
                  mvwdelch(sw[4],0,10+x);
                  wrefresh(sw[4]);
                } else {
                  buf[x] = (char) ci;
                  x++;
                }
                // wmove(sw[6],0,0);
                // wprintw(sw[6],"this is the char: %d",ci);
                // update_win(6);
                //up and down arrow
                if(count >= 0)
                {
                  if(ci==657 || ci==65) {
                    // wclrtoeol(sw[7]);
                    update_win(7);
                    wmove(sw[7],0,0);
                    wprintw(sw[7],"Command: u arrow");
                    update_win(7);
                    count++;
                  } else if(ci==667 || ci==66) {
                    // wclrtoeol(sw[7]);
                    update_win(7);
                    wmove(sw[7],0,0);
                    wprintw(sw[7],"Command: d arrow");
                    update_win(7);
                    count--;
                  }
                }

              } while(ci != '\n');

              buf[x-1] = 0;
              //save the command into the history memory
              // if(strcmp(buf, "[[A") != 0) {
              //   history[z]=buf;
              //   z++;
              // }
              //strcpy(history[z],buf);

              // Go back to screen 5 commands and get that window ready
              wclrtoeol(sw[4]);
              wmove(sw[4],0,0);
              waddstr(sw[4],"Commands: ");
              update_win(4);

              // if(count > 0 && count < HIST_LEN) {
              //   wclrtoeol(sw[4]);
              //   wmove(sw[4],0,0);
              //   //waddstr(sw[4],history[count]);
              //   wprintw(sw[4],"Commands: %s",&history[count]);
              //   update_win(4);
              //   update_win(6);
              //   process_command(buf, &opt_t , sw ,piggy, new_sd,left_con, right_con);
              // } else {
              //   wclrtoeol(sw[4]);
              //   wmove(sw[4],0,0);
              //   waddstr(sw[4],"Commands: ");
              //   update_win(4);
              // }

              /***************/
              /*INSERT MODE  */
              /***************/
              if(strcmp(buf,"i") == 0){
                // Variables to control the position of the strings
                getyx(sw[2],y,x);
                //x = 0; y = 1;
                // clear buffered used from commands
                memset(buf,0,sizeof(buf));
                wprintw(sw[6],"Piggy: Insert mode entered\n");
                update_win(6);

                // Move and clear space to input text
                wmove(sw[5],0,0);
                wclrtoeol(sw[5]);

                // while loop of characters
                while(c != EOF) {

                  c = wgetch(sw[5]);
                  // backspace works the same as the one in commands
                  if( c == 127 ){
                    buf[x] = 0;
                    x--;
                    if(x < 0){
                      x = 0;
                    }
                    // Since keypad was enabled
                    // wgetch returns the bakspace character, so
                    // we need to call mvwdelch three times for ^G (backspace key)
                    mvwdelch(sw[5],0,x);
                    mvwdelch(sw[5],0,x);
                    mvwdelch(sw[5],0,x);
                    wrefresh(sw[5]);
                  } else {
                    buf[x] = c;
                    x++;
                  }
                  // When pressing enter we send the buffer to window 0
                  // clear buf and go back to window 5 aka input
                  if(c == '\n'){
                    memset(buf,0,sizeof(buf));
                    x = 0;
                    y++;
                    wmove(sw[5],0,0);
                    wclrtoeol(sw[5]);
                    update_win(5);
                    count=0;//reset counter
                  }

                  // when you exit insert mode
                  if(c == 27){
                    wgetch(sw[5]);
                    wprintw(sw[6],"Insert mode: Exited\n");
                    update_win(6);
                    wmove(sw[4],0,10);
                    update_win(4);
                    break;
                  }

                  if(piggy == HEAD) {
                    if(c == 127){
                      mvwdelch(sw[1],y,x);
                      wrefresh(sw[1]);
                    } else {
                      waddch(sw[1],c);
                    }
                    update_win(1);
                    wmove(sw[5],0,x);
                    update_win(5);
                    rc = send(right_con,(const char*)&c,1, 0);
                  } else if (piggy == MIDDLE) {
                    if(i==right_con)
                    rc = send(new_sd,(const char*)&c,1,0);

                    else if(opt_t.outputl==1) {
                      if(c == 127){
                        mvwdelch(sw[2],y,x);
                        wrefresh(sw[2]);
                      } else {
                        waddch(sw[2],c);
                      }
                      update_win(2);
                      wmove(sw[5],0,x);
                      update_win(5);
                      rc = send(new_sd,(const char*)&c,1,0);
                    }

                    else if(opt_t.outputr==1) {
                      if(c == 127){
                        mvwdelch(sw[1],y,x);
                        wrefresh(sw[1]);
                      } else {
                        waddch(sw[1],c);
                      }
                      update_win(1);
                      wmove(sw[5],0,x);
                      update_win(5);
                      rc = send(right_con,(const char*)&c,1,0);
                    }

                    else
                      rc = send(right_con,(const char*)&c,1,0);

                    if((close_conn = rc_write(rc))) break;

                  } else if (piggy == TAIL) {
                    if(c == 127){
                      mvwdelch(sw[2],y,x);
                      wrefresh(sw[2]);
                    } else {
                      waddch(sw[2],c);
                    }
                    update_win(2);
                    wmove(sw[5],0,x);
                    update_win(5);
                    rc = send(new_sd,(const char*)&c,1,0);
                  }
                }
              } else if(strcmp(buf,"s") == 0){
                //first clear the buffer
                memset(buf,0,sizeof(buf));
                wprintw(sw[6],"Insert name of the file to be sent\n");
                update_win(6);
                wmove(sw[5],0,0);
                wclrtoeol(sw[5]);
                x = 0;
                do {
                  c = wgetch(sw[5]);
                  if( c == 127 ){
                    buf[x] = 0;
                    x--;
                    if(x < 0){
                      x = 0;
                    }
                    // Since keypad was enabled
                    // wgetch returns the bakspace character, so
                    // we need to call mvwdelch three times for ^G (backspace key)
                    mvwdelch(sw[5],0,x);
                    mvwdelch(sw[5],0,x);
                    mvwdelch(sw[5],0,x);
                    update_win(5);
                  } else {
                    buf[x] = c;
                    x++;
                  }

                } while(c != '\n');
                buf[x-1] = 0;

                wprintw(sw[6],"Text file name: %s\n",buf);
                update_win(6);

                myfile = fopen(buf,"r");

                if(myfile == NULL){
                  wprintw(sw[6],"Failed to read the file\n");
                  update_win(6);
                  wmove(sw[4],0,10);
                  update_win(4);
                } else {
                  char tcommand[BUFSIZE];
                  memset(buf,0,sizeof(buf));
                  memset(tcommand,0,sizeof(tcommand));
                  x = 0;
                  while(fread(buf,sizeof(char),1,myfile) == 1){

                    if(feof(myfile)) break;
                    if(buf[0] == ' ' || buf[0] == '\n'){
                      waddstr(sw[5],tcommand);
                      waddstr(sw[5]," ");
                      wrefresh(sw[5]);
                      process_command(tcommand, &opt_t , sw ,piggy, new_sd,left_con, right_con);
                      memset(tcommand,0,sizeof(tcommand));
                      x = 0 ;
                    }  else {
                      tcommand[x] = buf[0];
                      x++;
                    }
                  }

                  //end of while
                  fclose(myfile);
                  opt_t.scrip = 0;
                }
              } else {
                process_command(buf, &opt_t , sw ,piggy, new_sd,left_con, right_con);
              }
              // if(count!=0 && count > 0) {
              //   //wclrtoeol(sw[4]);
              //   wmove(sw[4],0,0);
              //   // waddstr(sw[6],"did this work");
              //   wprintw(sw[4],"%s",&history[count]);
              //   update_win(4);
              // }

              break;

            } //end of i==keyboard
            y = 1; x = 0; // restart x and y to 0 and 1 for receiving
          } while(1);

          // ******************** PERSISTANCE BLOCK ********************/////////
          // when the persistance flag is on, we need to create a new socket
          // with the same informaton and try to connect x amount of times before
          // deciding to close the socket
          if(close_conn) {
            if(i==right_con) {
              if((piggy==HEAD || piggy==MIDDLE) && opt_t.persr==1) {
                close(right_con);
                FD_CLR(right_con, &master_set);
                right_con=create_socket(ad);
                FD_SET(right_con, &master_set);

                //will attempt to connect for five seconds
                for(int nsec = 1; nsec <= MAXSLEEP; nsec <<= 1) {
                  refresh();
                  printw("Re-connecting...\n");
                  if(connect(right_con, (struct sockaddr *)&ad, sizeof(ad)) == 0) {
                    printw("Connection accepted\n"); nsec = MAXSLEEP+1;
                  }
                  if(nsec <= MAXSLEEP/2) sleep(nsec);
                }
              } else { //close the given socket and decrement max_sd
                close(i);
                FD_CLR(i,&master_set);
                if(i == max_sd) {
                  while(FD_ISSET(max_sd,&master_set) == 0) max_sd--;
                }
              }
            } else if(i==max_sd) { //disrupt close loop
              close(i);
              FD_CLR(i,&master_set);
              if(i == max_sd) {
                while(FD_ISSET(max_sd,&master_set) == 0) max_sd--;
              }
            }
          }
        } //end of else FD_ISSET is NOT in the working_set
      } //end of if FD_ISSET is in the working set
    } //end of for loop - reading throuh available sockets
  } while(end_server == 0);

  // Clean up all open sockets
  for(int i = 0 ; i <= max_sd; i++) {
    if(FD_ISSET(i , &master_set)) close(i);
  }
}
