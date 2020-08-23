/*****************************************
 * utils.h
 *
 *****************************************/
#include <stdbool.h>

//header guard

#ifndef UTILS_H
#define UTILS_H

/*
 * if DEBUG_UTILS defined, enables simple debugging in source file
 * in production "#define DEBUG_UTILS" should be commented out
 */   
//#define DEBUG_UTILS

/* define various maximum size of buffers used */ 
#define MAX_ARRAY_SIZE 200
#define MAX_FILE_SIZE 2000

/* function declarations */   
int utils_getnamedate(char*, int);
int utils_getfilepath(char*, int, char*, char*);
int utils_getxmltag(char*, char*, char*);
double utils_getxmltag_d(char*, char*);
int utils_getxmltag_i(char*, char*);
long utils_getxmltag_l(char*, char*);
bool utils_appendtofile(char*, char*, char*);
int utils_get_date_time(char*, int);


#endif


