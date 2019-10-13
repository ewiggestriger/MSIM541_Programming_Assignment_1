#include "utility.h"

void printString(string str)
{
	for (int i = 0; i < str.length(); i++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, str[i]);
}

