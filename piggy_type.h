#ifndef PIGGY_TYPE
#define PIGGY_TYPE	1

void head(int right_con, struct sockaddr_in ad, WINDOW* sw[]);

void middle(int left_con, int right_con,
            struct sockaddr_in lad, struct sockaddr_in rad, WINDOW* sw[]);

void tail(int left_con, struct sockaddr_in ad, WINDOW* sw[]);

#endif
