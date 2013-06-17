#ifndef _TERMEDIT_H_
#define _TERMEDIT_N_

#define UPCHAR "\033[A"
#define DOWNCHAR "\033[B"
#define RIGHTCHAR "\033[C"
#define LEFTCHAR "\033[D"

//#define DELCHAR "\033[[3~"
#define DELCHAR "\033\133\063\176"

//#define PREFIX "SR-0.0.1: "

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <signal.h>

struct lineInformation{
	char* line;
	const char* prefix;
	int prefixLength;
	int linePosition, lineAllocated;
	int cursorX, cursorY;
	int reading, kill;
	struct winsize windowSize;
	struct sigaction oldAction;
};

struct terminalInformation{
	struct termios original, config;
	int termFD;
};

extern struct lineInformation* li;
extern struct terminalInformation* ti;

void signalHandler(int signal);
void termedit_init();
char* termedit_read(const char*);
void termedit_close();

#endif
