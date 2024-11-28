#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>



extern "C"
{
	void sayHello();
}

//extern void sayHello();
void main()
{
	printf("Hello, what is your name?\n");
	sayHello();
	while (getchar() != '\n');
}
extern "C"
void* readName()
{
	char* name = (char*)calloc(1, 255);
	scanf("%s", name);
	while (getchar() != '\n');
	return name;
}