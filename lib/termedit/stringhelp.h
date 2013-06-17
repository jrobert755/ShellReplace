#ifndef _STRINGHELP_H_
#define _STRINGHELP_H_

/* Will insert the given character into the
 * c-string at the location of the second
 * integer. If there is not enough space,
 * as measured by the first input integer, 
 * the string will be reallocated. The 
 * resulting string is returned.
 */
 #include <stdlib.h>
 #include <string.h>
 
char* insertCharacters(char*, char*, int*, int);

void deleteCharacter(char*, int);

#endif
