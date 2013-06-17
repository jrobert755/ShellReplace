#include "termedit.h"

#include <signal.h>

int running = 1;

void signalHandler(int signal);

int main(int argc, char* argv[]){
	signal(SIGINT, signalHandler);
	//signal(SIGQUIT, signalHandler);
	struct terminalInformation term;
	struct lineInformation lineInfo;
	
	initTermEdit(&lineInfo, &term);
	while(running) handleInput(&lineInfo);
	closeTermEdit(&lineInfo, &term);
	
	return 0;
}

void signalHandler(int signal){
	running = 0;
}

