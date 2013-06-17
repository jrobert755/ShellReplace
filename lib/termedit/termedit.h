#ifndef _TERMEDIT_H_
#define _TERMEDIT_N_

#define UPCHAR "\033[A"
#define DOWNCHAR "\033[B"
#define RIGHTCHAR "\033[C"
#define LEFTCHAR "\033[D"

//#define DELCHAR "\033[[3~"
#define DELCHAR "\033\133\063\176"

#define PREFIX "SR-0.0.1: "

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>

struct lineInformation{
	char* line;
	char* prefix;
	int linePosition, lineAllocated;
	int cursorX, cursorY;
	struct winsize windowSize;
};

struct terminalInformation{
	struct termios original, config;
	int termFD;
};

void initTermEdit(struct lineInformation*, struct terminalInformation*);
void handleInput(struct lineInformation*);
void closeTermEdit(struct lineInformation*, struct terminalInformation*);

#endif
