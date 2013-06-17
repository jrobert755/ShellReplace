#include "termedit.h"
#include "stringhelp.h"

struct lineInformation* li = NULL;
struct terminalInformation* ti = NULL;

void signalHandler(int signal){
	if(signal == SIGINT){
		li->reading = 0;
		li->kill = 1;
	}
}

void termedit_init(){
	ti = (struct terminalInformation*)malloc(sizeof(struct terminalInformation));
	struct termios* original, *config;
	original = &(ti->original);
	config = &(ti->config);
	setvbuf(stdout, (char *)NULL, _IONBF, 0); 
	char* termID = ctermid(NULL);
	ti->termFD = open(termID, O_RDWR);
	if(tcgetattr(ti->termFD, config) < 0){
		printf("ERROR GETTING ATTR");
		exit(1);
	}
	memcpy(original, config, sizeof(*config));
	config->c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
	//config->c_cc[VMIN]  = 1;
	//config->c_cc[VTIME] = 0;
	config->c_cc[VMIN]  = 0;
	config->c_cc[VTIME] = 0;
	if(tcsetattr(ti->termFD, TCSAFLUSH, config) < 0) {
		printf("ERROR SETTING ATTR");
		exit(1);
	}
	
	li = (struct lineInformation*)malloc(sizeof(struct lineInformation));
	li->line = (char*)malloc(sizeof(char) * 65);
	li->lineAllocated = 64;
	/*strcpy(li->line, PREFIX);
	li->linePosition = strlen(li->line);
	printf("%s", li->line);
	
	li->cursorX = strlen(li->line);*/
	li->prefixLength = 0;
	li->linePosition = 0;
	li->cursorX = 0;
	li->cursorY = 0;
	
	//ioctl(ti->termFD, TIOCSWINSZ, &(li->windowSize));
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &(li->windowSize));
	
	/*li->prefix = (char*)malloc(sizeof(char) * (strlen(PREFIX) + 1));
	strcpy(li->prefix, PREFIX);*/
	struct sigaction newAction;
	newAction.sa_handler = signalHandler;
	sigemptyset(&newAction.sa_mask);
	sigaction(SIGINT, &newAction, &(li->oldAction));
	li->kill = 0;
}

int calculateCursorStringPosition(){
	int location = li->cursorX;
	location += (li->cursorY * li->windowSize.ws_col);
	//location += strlen(li->prefix);
	return location;
}

void calculateXY(int* x, int* y, int position){
	int ws_col = li->windowSize.ws_col;
	*x = position % ws_col;
	*y = ((position - *x) / ws_col);
	if((position % ws_col) == 0){
		(*y)--;
		*x = ws_col;
	}
}

void wrapCursorUp(){
	li->cursorY--;
	li->cursorX = (li->windowSize.ws_col - 1);
	printf("%s", UPCHAR);
	printf("\033[%iC", li->windowSize.ws_col - 1);
}

void wrapCursorDown(){
	li->cursorY++;
	li->cursorX = 0;
	printf("%s", DOWNCHAR);
	printf("\033[%iD", li->windowSize.ws_col - 1);
}

void moveCursorToPosition(int positionX, int positionY, int currentPositionX, int currentPositionY){
	int toMoveX = positionX - currentPositionX;
	int toMoveY = currentPositionY - positionY;
	if(toMoveX > 0) printf("\033[%iC", toMoveX);
	else if(toMoveX < 0) printf("\033[%iD", -toMoveX);
	if(toMoveY > 0) printf("\033[%iA", toMoveY);
	else if(toMoveY < 0) printf("\033[%iB", -toMoveY);
}

/** Set backspace = 0 if the cursor should stay in position & delete
 * Set backspace != 0 if the cursor should move back 1 space & delete
 */
void deleteFromLine(int backspace){
	int location = calculateCursorStringPosition();
	if(backspace) location--;
	//if(location >= strlen(li->prefix) && (backspace || location < li->linePosition)){
	if(location >= li->prefixLength && (backspace || location < li->linePosition)){
		deleteCharacter(li->line, location);
		if(backspace){
		li->cursorX--;
			if(li->cursorX < 0){
				wrapCursorUp(li);
			} else printf("%s", LEFTCHAR);
		}
		printf("%s ", &(li->line[location]));
		int x,y;
		calculateXY(&x, &y, li->linePosition);
		moveCursorToPosition(li->cursorX, li->cursorY, x, y);
		li->linePosition--;
	}
}

void readchar(){
	char input[4];
	memset(input, 0, 4);
	
	memset(input, 0, 4);
	if(read(STDIN_FILENO, &input, 4) > 0){
		if(strcmp(input, UPCHAR) == 0){
		} else if(strcmp(input, LEFTCHAR) == 0){
			li->cursorX--;
			int location = calculateCursorStringPosition();
			if(location < li->prefixLength) li->cursorX++;
			else if(li->cursorX < 0 && li->cursorY > 0){
					wrapCursorUp(li);
			}
			else printf("%s", LEFTCHAR);
		} else if(strcmp(input, RIGHTCHAR) == 0){
			li->cursorX++;
			int location = calculateCursorStringPosition();
			if(location > li->linePosition) li->cursorX--;
			else if(li->cursorX >= li->windowSize.ws_col){
				wrapCursorDown(li);
			}
			else printf("%s", RIGHTCHAR);
		} else if(input[0] == 127){
			deleteFromLine(1);
		} else if(strncmp(input, DELCHAR, strlen(DELCHAR)) == 0){
			deleteFromLine(0);
		} else if(input[0] == '\033'){
		} else{
			int location = calculateCursorStringPosition();
		
			if(location == (li->linePosition/* + strlen(li->prefix)*/)){
				if((li->linePosition + strlen(input)) > li->lineAllocated){
					li->line = (char*)realloc(li->line, sizeof(char) * (li->lineAllocated * 2 + 1));
					li->lineAllocated *= 2;
				}
				strcat(li->line, input);
				li->linePosition += strlen(input);
				printf("%s", input);
				li->cursorX += strlen(input);
			} else{
				li->line = insertCharacters(li->line, input, &(li->lineAllocated), location/* - strlen(li->prefix)*/);
				li->linePosition += strlen(input);
				li->cursorX += strlen(input);
				if(li->cursorX > li->windowSize.ws_col){
					li->cursorX %= li->windowSize.ws_col;
					li->cursorY++;
				}
			
				printf("%s",&(li->line[location]));
				int x,y;
				calculateXY(&x, &y, li->linePosition);
				moveCursorToPosition(li->cursorX, li->cursorY, x, y);
			}
			if(li->cursorX >= li->windowSize.ws_col){
				printf("\n");
				li->cursorX = 0;
				li->cursorY++;
			}
		}
	}
}

char* termedit_read(const char* prefix){
	termedit_init();
	li->reading = 1;
	while(li->reading){
		li->prefix = prefix;
		li->prefixLength = prefix == NULL ? 0 : strlen(prefix);
		if(prefix != NULL && li->linePosition == 0){
			printf("%s", prefix);
			strcpy(li->line, prefix);
			li->linePosition += li->prefixLength;
			li->cursorX += li->prefixLength;
		}
		readchar();
		if(li->line[li->linePosition-1] == '\n') li->reading = 0;
	}
	char* returnValue = (char*)malloc(sizeof(char) * (li->linePosition));
	strncpy(returnValue, &(li->line[li->prefixLength]), li->linePosition-li->prefixLength-1);
	termedit_close();
	return returnValue;
}

void termedit_close(){
	sigaction(SIGINT, &(li->oldAction), NULL);
	
	int x,y;
	calculateXY(&x, &y, li->linePosition);
	moveCursorToPosition(x, y, li->cursorX, li->cursorY);
	
	tcsetattr(ti->termFD, TCSAFLUSH, &(ti->original));
	
	free(li->line);
	li->line = NULL;
	/*free(li->prefix);
	li->prefix = NULL;*/
	free(li);
	if(li->kill) printf("\n");
	
	close(ti->termFD);
	free(ti);
}
