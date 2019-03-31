#define _AFXDLL 
#define _CRT_SECURE_NO_WARNINGS

#include "stdio.h"
#include "stdlib.h"
#include "afxwin.h"
#include "stdint.h"
#include "iostream"



FILE *inputFile, *outputFile;
const char* inputPath = "encode.mjpeg";
const char* outputPath = "";

int main() {
	inputFile = fopen(inputPath, "rb+");
	uint8_t *BufferLong = (uint8_t*)malloc(1024 * 1024);
	while (1) {
		uint8_t *Buffer=(uint8_t*)malloc(1);
		fread(Buffer, sizeof(uint8_t), 1, inputFile);
		//printf("%hd\n", Buffer);
		//getchar();
		std::cout << atoi((const char*)Buffer)<<std::endl;
		free(Buffer);
	}
	return 0;
}
