#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "utils.h"
#include <stdbool.h>
#include <string.h>

/*****************************
 * utils_getnamedate() appends hostname and date and time to a string
 *
 * Appends the host name and date time in the form
 * hostname_yymmdd_hhmmss
 * This is typically used to generate file names
 * which are identified with this host.
 * 
 * If buffer is too small or any errors function returns zero
 * and the contents of buffer are undefined.
 * 
 * param buffer - is a char array to store the results in
 * param buf_size - is the size of the buffer char array
 * returns - the size of the string returned in buffer or 0 if error  
****************************/

int
utils_getnamedate(char* buffer, int buf_size)
{
    char date_time[MAX_ARRAY_SIZE] = "";  
    int result = 0;

    //get first part of the name, ie hostname
    result = gethostname(buffer, buf_size);     
    if (result == -1)                    
    {
        return 0;
    }

    //get second part of the name, ie date and time
    result = utils_get_date_time(date_time, sizeof(date_time));
    if (!result)
    {
        return 0;                         //strftime() returns 0 on error
    }

    //check if buffer big enough to hold result
    if ((strlen(buffer) + strlen(date_time) +1) > buf_size)
    {
        return 0;                         //buffer too small
    }

    //combine the 2 parts
    strcat(buffer, "_");
    strcat(buffer, date_time);

    #ifdef DEBUG_UTILS
    printf("utils_getnamedate: %s\n", buffer);
    #endif

    return strlen(buffer);
}


/****************************
 * utils_get_date_time () gets date and time in format specified in code
 *
 * Format is yy-mm-dd hh:mm:ss.
 * This is used in generation of file names
 * 
 * If buffer is too small or any errors function returns zero
 * and the contents of buffer are undefined.
 * 
 * param buffer - is a char array to store the results in
 * param size - is the size of the buffer char array
 * returns - the size of the string returned in buffer or 0 if error  
****************************/   
int
utils_get_date_time(char* buffer, int size) {
    struct tm* time_now;
    time_t now;
    int result;
    
    now = time(NULL);
    time_now = localtime(&now);
    result = strftime(buffer, size, "%Y-%m-%d %H:%M:%S", time_now);
    #ifdef DEBUG_UTILS
    printf("utils_get_date_time() - %s\n", buffer);
    #endif

    return result;                         //strftime() returns 0 on error
}


/****************************
 * utils_getfilepath() gets path to file in specified directory
 *
 * Returns the complete path including filename in buffer
 * If buffer is too small or any errors function returns zero
 * and the contents of buffer are undefined.
 * 
 * param buffer - is a char array to store the results in
 * param buf_size - is the size of the buffer char array
 * param sub_dir - sub directory where file is stored, may be blank
 * param filename - is the name of the file
 * returns - the size of the string returned in buffer or 0 if error  
****************************/

int
utils_getfilepath(char* buffer, int buf_size, char* sub_dir, char* file_name)
{
    int result;
  
    // sub directory, with filename
    //found snprintf() buggy, so no check on overwriting the buffer
         result = sprintf(buffer, "%s%s", sub_dir, file_name);
    if  (result == -1)
    {
        return 0;                   // snprintf() returns -1 on error
    }

    if (result == buf_size)
    {
        return 0;                   //buffer was too small and path truncated
    }
#ifdef DEBUG_UTILS
    printf("utils_getfilepath: %s\n", buffer);
#endif
    return sizeof(buffer);
}




/****************************
 * utils_appendto file() appends a message to a file
 *
 * If the file does not exist it is created.
 * Date and time are pre-appended to the filename
 *
 * Any errors return false, otherwise return true
 *  
 * param filename - name of file
 * param subdir - sub directory where file is stored
 * param message - message to write
 * returns - false if error in appending to the file
****************************/

bool
utils_appendtofile(char* filename, char* subdir, char* message)
{
    char buffer[MAX_ARRAY_SIZE] = {0};
    char tmpfilename[MAX_ARRAY_SIZE] = {0};
    
    //pre-append date and time to file name
    utils_getnamedate(tmpfilename, sizeof(tmpfilename));
    strcat(tmpfilename, "_");
    strcat(tmpfilename, filename);

    //get absolute path to the file
    int result = 0;
    result = utils_getfilepath(buffer, MAX_ARRAY_SIZE, subdir, tmpfilename);
    if (result ==0 )
    {        
        return false;
    }
    
    FILE *fp;
    fp = fopen(buffer, "a");
    if (fp == NULL)
    {
        return false;
    }
     
    #ifdef DEBUG_UTILS
    printf("utils_appendtofile - message: %s\n", message);
    #endif
  
    result = fprintf(fp, message);
    fclose(fp);
    if (result <= 0 )
    {
        return false;
    }
    
    return true;    
}


/****************************
 * utils_getxmltag() gets a tag value as a string from an xml file
 *
 * Only supports a very simple xml file.
 * Assumes file size is less than MAX_FILE_SIZE.
 * File has to be small as file contents, excluding comments
 * are read into memory.
 *
 * Any error or cannot find the tag, returns zero
 * and the contents of tagvalue are undefined.
 * Only MAX_FILE_SIZE chars are read, the rest are ignored.
 *
 * param filename - name of xml file
 * param tag - tag we are looking for
 * param tagvalue - tag value from the file
 * returns - size of tagvalue or 0 if error
*****************************/

int
utils_getxmltag(char *filename, char *tag, char *tagvalue)
{
   
    #ifdef DEBUG_UTILS
    printf("utils_getxmltagvalue - filename: %s\n", filename);
    #endif
    
    int len = 0;
    char openingtag[MAX_ARRAY_SIZE] = {0}; //store tags here
    char closingtag[MAX_ARRAY_SIZE] = {0};
    
    //get data from file
    FILE* fp;
    fp = fopen(filename, "r");
      if (fp == NULL)
    {
        return 0;
    }
    
    char buffer[MAX_ARRAY_SIZE] = {0};
    char filebuffer[MAX_FILE_SIZE] = {0};
    int charcount = 0;
    
    while (fgets(buffer, MAX_ARRAY_SIZE, fp) != NULL)
    {  
        //strip out comments and check file not too large
       
        if ( strncmp(buffer, "<!", 2) != 0 )
        {
            if ( (charcount += strlen(buffer)) < MAX_FILE_SIZE)
                  strcat(filebuffer, buffer);
        }
    }
    if (strlen(filebuffer) <= 0)
    { 
        return 0;
    }
    fclose(fp);
    
    #ifdef DEBUG_UTILS
    sscanf(filebuffer,"%20s", buffer);
    printf("utils_getxmltagvalue - start of file: %s\n",buffer);
    #endif

    //Create opening tag
    strcpy(openingtag, "<");
    strcat(openingtag, tag);
    strcat(openingtag, ">");

    //Create closing tag
    strcpy(closingtag, "</");
    strcat(closingtag, tag);
    strcat(closingtag, ">");

    int pos = 0;
    int posopeningtag = 0;
    int posclosingtag = 0;
    bool openingtagfound = false;
    bool closingtagfound = false;
    len = sizeof(filebuffer);
   
    //Get opening tag position in data read from file
    
    for (pos = 0; pos < len; pos++)
    {
        if ( !memcmp(openingtag, filebuffer+pos, strlen(openingtag)) )
        {
            posopeningtag = pos;
            openingtagfound = true;
            break;
        }
    }

    //Get closing tag position in data read from file
    for (pos = 0; pos < len; pos++)
    {
        if ( !memcmp(closingtag, filebuffer+pos, strlen(closingtag)) )
        {
            posclosingtag = pos;
            closingtagfound = true;
            break;
        }
    }

    if (!openingtagfound || !closingtagfound)
    {
        return 0;
    }

    /* Note no check if number of chars about to read from filebuffer,
     * will fit into the char array, tagvalue */
    int charsintag = posclosingtag-posopeningtag-strlen(openingtag);
      
    //Get tag value
    memcpy(
        tagvalue,
        filebuffer+posopeningtag+strlen(openingtag),
        posclosingtag-posopeningtag-strlen(openingtag)
        );

    //ensure string is terminated with a null character
    tagvalue[charsintag] = '\0';
    
    return strlen(tagvalue);
}


/****************************
 * utils_getxmltag_d() gets a tag value as a double from an xml file
 *
 * Uses utils_getxmltag() to get the string and then converts it to a double
 * Default value is 0, so if error returns 0.0
 *
 * param filename - name of xml file
 * param tag - tag we are looking for
 * returns - double
*****************************/

double
utils_getxmltag_d(char *filename, char *tag)
{
    char buffer[MAX_ARRAY_SIZE] = {0};
    
    if ( utils_getxmltag(filename, tag, buffer) )
    { 
        return  strtod(buffer, NULL);
    }
    else
    {   //error getting tag
        return 0.0;
    }
}


/****************************
 * utils_getxmltag_i() gets a tag value as an integer from an xml file
 *
 * Uses utils_getxmltag() to get the string and then converts it to a integer
 * Default value is 0, so if error returns 0
 *
 * param filename - name of xml file
 * param tag - tag we are looking for
 * returns - integer
*****************************/

int
utils_getxmltag_i(char *filename, char *tag)
{
    char buffer[MAX_ARRAY_SIZE] = {0};
    
    if ( utils_getxmltag(filename, tag, buffer) )
    { 
        return  atoi(buffer);
    }
    else
    {   //error getting tag
        return 0;
    }
}







