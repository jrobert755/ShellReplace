#include "termedit.h"
#include "stringhelp.h"

void initTermEdit(struct lineInformation* inputInformation, struct terminalInformation* term){
	struct termios* original, *config;
	original = &(term->original);
	config = &(term->config);
	setvbuf(stdout, (char *)NULL, _IONBF, 0); 
	char* termID = ctermid(NULL);
	term->termFD = open(termID, O_RDWR);
	if(tcgetattr(term->termFD, config) < 0){
		printf("ERROR GETTING ATTR");
		exit(1);
	}
	memcpy(original, config, sizeof(*config));
	config->c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
	//config->c_cc[VMIN]  = 1;
	//config->c_cc[VTIME] = 0;
	config->c_cc[VMIN]  = 0;
	config->c_cc[VTIME] = 0;
	if(tcsetattr(term->termFD, TCSAFLUSH, config) < 0) {
		printf("ERROR SETTING ATTR");
		exit(1);
	}
	
	inputInformation->line = (char*)malloc(sizeof(char) * 65);
	inputInformation->lineAllocated = 64;
	strcpy(inputInformation->line, PREFIX);
	inputInformation->linePosition = strlen(inputInformation->line);
	printf("%s", inputInformation->line);
	
	inputInformation->cursorX = strlen(inputInformation->line);
	inputInformation->cursorY = 0;
	
	//ioctl(term->termFD, TIOCSWINSZ, &(inputInformation->windowSize));
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &(inputInformation->windowSize));
	
	inputInformation->prefix = (char*)malloc(sizeof(char) * (strlen(PREFIX) + 1));
	strcpy(inputInformation->prefix, PREFIX);
}

int calculateCursorStringPosition(struct lineInformation* inputInformation){
	int location = inputInformation->cursorX;
	location += (inputInformation->cursorY * inputInformation->windowSize.ws_col);
	//location += strlen(inputInformation->prefix);
	return location;
}

void calculateXY(int* x, int* y, int position, struct lineInformation* inputInformation){
	int ws_col = inputInformation->windowSize.ws_col;
	*x = position % ws_col;
	*y = ((position - *x) / ws_col);
	if((position % ws_col) == 0){
		(*y)--;
		*x = ws_col;
	}
}

void wrapCursorUp(struct lineInformation* inputInformation){
	inputInformation->cursorY--;
	inputInformation->cursorX = (inputInformation->windowSize.ws_col - 1);
	printf("%s", UPCHAR);
	printf("\033[%iC", inputInformation->windowSize.ws_col - 1);
}

void wrapCursorDown(struct lineInformation* inputInformation){
	inputInformation->cursorY++;
	inputInformation->cursorX = 0;
	printf("%s", DOWNCHAR);
	printf("\033[%iD", inputInformation->windowSize.ws_col - 1);
}

void moveCursorToPosition(int positionX, int positionY, int currentPositionX, int currentPositionY/*, struct lineInformation* inputInformation*/){
	//int x,y;
	//calculateXY(&x, &y, currentPosition, inputInformation);
	//int toMoveX = inputInformation->cursorX - x;
	//int toMoveY = y - inputInformation->cursorY;
	int toMoveX = positionX - currentPositionX;
	int toMoveY = currentPositionY - positionY;
	if(toMoveX > 0) printf("\033[%iC", toMoveX);
	else if(toMoveX < 0) printf("\033[%iD", -toMoveX);
	if(toMoveY > 0) printf("\033[%iA", toMoveY);
	else if(toMoveY < 0) printf("\033[%iB", -toMoveY);
}

void handleInput(struct lineInformation* inputInformation){
	char input[4];
	memset(input, 0, 4);
	
	if(read(STDIN_FILENO, &input, 4) > 0){
		if(strcmp(input, UPCHAR) == 0){
		} else if(strcmp(input, LEFTCHAR) == 0){
			inputInformation->cursorX--;
			
			int location = calculateCursorStringPosition(inputInformation);
			if(location < strlen(inputInformation->prefix)) inputInformation->cursorX++;
			else if(inputInformation->cursorX < 0){
				if(inputInformation->cursorY > 0){
					wrapCursorUp(inputInformation);
				}
			}
			else printf("%s", LEFTCHAR);
		} else if(strcmp(input, RIGHTCHAR) == 0){
			inputInformation->cursorX++;
			int location = calculateCursorStringPosition(inputInformation);
			if(location > inputInformation->linePosition) inputInformation->cursorX--;
			else if(inputInformation->cursorX >= inputInformation->windowSize.ws_col){
				wrapCursorDown(inputInformation);
			}
			else printf("%s", RIGHTCHAR);
		} else if(input[0] == 127){
			int location = calculateCursorStringPosition(inputInformation);
			location--;
			if(location >= strlen(inputInformation->prefix)){
				deleteCharacter(inputInformation->line, location);
				inputInformation->cursorX--;
				if(inputInformation->cursorX < 0){
					wrapCursorUp(inputInformation);
				} else printf("%s", LEFTCHAR);
				printf("%s ", &(inputInformation->line[location]));
				int x,y;
				calculateXY(&x, &y, inputInformation->linePosition, inputInformation);
				moveCursorToPosition(inputInformation->cursorX, inputInformation->cursorY, x, y);
				inputInformation->linePosition--;
			}
		} else if(strncmp(input, DELCHAR, strlen(DELCHAR)) == 0){
			int location = calculateCursorStringPosition(inputInformation);
			if(location > strlen(inputInformation->prefix) && location < inputInformation->linePosition){
				deleteCharacter(inputInformation->line, location);
				printf("%s ", &(inputInformation->line[location]));
				int x,y;
				calculateXY(&x, &y, inputInformation->linePosition, inputInformation);
				moveCursorToPosition(inputInformation->cursorX, inputInformation->cursorY, x, y);
				inputInformation->linePosition--;
			}
		} else if(input[0] == '\033'){
		} else{
			int location = calculateCursorStringPosition(inputInformation);
			
			if(location == (inputInformation->linePosition/* + strlen(inputInformation->prefix)*/)){
				if((inputInformation->linePosition + strlen(input)) > inputInformation->lineAllocated){
					inputInformation->line = (char*)realloc(inputInformation->line, sizeof(char) * (inputInformation->lineAllocated * 2 + 1));
					inputInformation->lineAllocated *= 2;
				}
				strcat(inputInformation->line, input);
				inputInformation->linePosition += strlen(input);
				printf("%s", input);
				inputInformation->cursorX += strlen(input);
			} else{
				inputInformation->line = insertCharacters(inputInformation->line, input, &(inputInformation->lineAllocated), location/* - strlen(inputInformation->prefix)*/);
				inputInformation->linePosition += strlen(input);
				inputInformation->cursorX += strlen(input);
				if(inputInformation->cursorX > inputInformation->windowSize.ws_col){
					inputInformation->cursorX %= inputInformation->windowSize.ws_col;
					inputInformation->cursorY++;
				}
				
				printf("\r%s", &(inputInformation->line[inputInformation->cursorY * inputInformation->windowSize.ws_col]));
				int x,y;
				calculateXY(&x, &y, inputInformation->linePosition, inputInformation);
				moveCursorToPosition(inputInformation->cursorX, inputInformation->cursorY, x, y);
			}
			if(inputInformation->cursorX >= inputInformation->windowSize.ws_col){
				printf("\n");
				inputInformation->cursorX = 0;
				inputInformation->cursorY++;
			}
		}
	}
}

void closeTermEdit(struct lineInformation* inputInformation, struct terminalInformation* term){
	int x,y;
	calculateXY(&x, &y, inputInformation->linePosition, inputInformation);
	moveCursorToPosition(x, y, inputInformation->cursorX, inputInformation->cursorY);
	
	tcsetattr(term->termFD, TCSAFLUSH, &(term->original));
	
	free(inputInformation->line);
	inputInformation->line = NULL;
	free(inputInformation->prefix);
	inputInformation->prefix = NULL;
	printf("\n");
	
	close(term->termFD);
}
