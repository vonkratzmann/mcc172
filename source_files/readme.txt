Introduction
The program "scantofile" acquires blocks of analog input data for a user-specified group of channels. The acquisition is stopped when the specified number of samples is acquired for each channel.

The system is based on the Measurement Computing (MC) MCC 172 for sound and vibration measurement which plugs into a Raspberry Pi 4B. The software is based on the MC supplied open sourced library and examples.

The MCC 172 is a two channel DAQ HAT for making sound and vibration measurements from IEPE sensors like accelerometers and microphones. It features a 24-bit A/D per channel and a maximum sample rate of  51.2 kS/s/Ch. Up to eight MCC HATs can be stacked onto one Raspberry Pi but this software only supports one board, ie 2 channels.

The program “scantofile”  is designed to run standalone on a Raspberry Pi without a monitor. The collected data is written to a log file or an error log file if there are any errors. 

Configuration
The following parameters in the  XML file: “vib_params” allow the user to make changes to the data collection parameters:

    1. hardware options
    2. scan_rate 
    3. samples per channel
    4. number of channels
    5. sensitivity
    6. IEPE power supply

A description of each parameter is provided in the xml file with the parameters.

Operation
To run the program, go to the directory mcc172 and type “./scantofile”. 

If successful the program will generate a log file in the directory “results” with a filename consisting of the Raspberry Pi’s host name, the date, and the time. If unsuccessful an error file will be generated with a file name of  host name, the date,  the time, and the  chars “errolog” appended to the end of the filename.

Software Files
The MC software libraries and include files have not been changed. The following files have been developed or  used for this program and are stored in the directory “mcc172”:

source_files/scantofile.c	- based on the MC example program finite_scan.c
source_files/scantofile.h	- codes for XML configuration , error messages , file names  
source_files/utils.c		- a collection of utilities to support the scantofile.c
source_files/utils.h		- function declarations for utils.c
source_files/makefile		- to compile the source files

vib-params 			- XML file containing configuration parameters
source_files/readme		- this file

results/*			- log files and error files generated when the application is run

Files provided by MC which have not been changed:

source_files/daqhats.h	-  declaration of constants, errors and commands
source_files/daqhats_utils.h	- helper functions used in MC supplied examples
source_files/mcc172.h	- function declarations for mcc172
source_files/daqhats.so.1.3.0.5	- library


Software Functions


Functions in “scantofile.c”:

/******************************
 * main()
 *
 * Acquires blocks of analog input data for a user-specified group
 * of channels. The acquisition is stopped when the specified
 * number of samples is acquired for each channel.
 ****************************/

/****************************
 * add_to_errorlog_quit() - add a message to the error log file, then quit
 * 
 * param message - message to be written to the log file
 ****************************/

/****************************
 * get_log_file() - get the name and absolute path to the log file
 *
 * Get the hostname, date and time which forms the filename.
 * Add absolute path to the filename.
 * If errors, add to error log, and quit as no point in proceeding
 *
 * param filename - holds complete file name on return 
 * ****************************/

/****************************
 * utils_gettag_errchk_d gets a tag value as a double from an xml file
 *
 * utils_getxmltag_d()  - get the double
 * If not available, adds message to log file,
 * which will also terminate the program.
 *
 * param filename - name of xml file
 * param parameter - tag we are looking for
 * returns - the double value from xml file
*****************************/

/****************************
 * utils_gettag_errchk_i () - gets a tag value as an int from an xml file
 *
 * Uses utils_getxmltag_i() to get the int
 * If not available, adds message to log file,
 * which will also terminate the program.
 *
 * param filename - name of xml file
 * param parameter - tag we are looking for
 * returns - the int value from xml file
*****************************/

/****************************
 * stop_if_error() - called to check for errors during scanning
 *
 * If no error, do nothing, return.
 * If error, appends the error message to the error log file,
 * shuts down the hardware,
 * and terminates the program.
*****************************/

/****************************
 * iepe_power_off()  - shutdown the hardware
 * 
 * If error appends it to the error log file.
*****************************/

/****************************
 * close_mcc172() - shutdown the hardware
 * 
 * If error appends it to the error log file.
*****************************/

/****************************
 * get_err_str() - generates a string from an error or result code
 * 
 * It appends the error message to the error log file rather
 * rather than the console and will only output
 * the error message to the console if debugging is enabled.
 * param result - result code to be converted to a string
 * return - string for the result code
*****************************/

Functions in “utils.c”:

/****************************
 * utils_getnamedate() - appends hostname and date and time to a string
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

/****************************
 * utils_get_date_time () - gets date and time in format specified in code
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

/****************************
 * utils_getfilepath() - gets absolute path to file
 *
 * Gets the home directory from the OS for the user,
 * gets the path for the working dir defined in PATHRESULTS
 * returns the complete path including filename in buffer
 * 
 * If buffer is too small or any errors function returns zero
 * and the contents of buffer are undefined.
 * 
 * param buffer - is a char array to store the results in
 * param buf_size - is the size of the buffer char array
 * param sub_dir - sub directory where file is stored, may be blank
 * param filename - is the name of the file
 * returns - the size of the string returned in buffer or 0 if error  
****************************/

/****************************
 * utils_appendto file() - appends a message to a file
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
  
/****************************
 * utils_getxmltag() - gets a tag value as a string from an xml file
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

/****************************
 * utils_getxmltag_d() - gets a tag value as a double from an xml file
 *
 * Uses utils_getxmltag() to get the string and then converts it to a double
 * Default value is 0, so if error returns 0.0
 *
 * param filename - name of xml file
 * param tag - tag we are looking for
 * returns - double
*****************************/

/****************************
 * utils_getxmltag_i() - gets a tag value as an integer from an xml file
 *
 * Uses utils_getxmltag() to get the string and then converts it to a integer
 * Default value is 0, so if error returns 0
 *
 * param filename - name of xml file
 * param tag - tag we are looking for
 * returns - integer
*****************************/

