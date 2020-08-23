/** Include file for vibration scans **/


//header guard

#ifndef SCANTOFILE_H
#define SCANTOFILE_H

/*
 * if DEBUG_UTILS defined, enables simple debugging in source file
 * in production "#define DEBUG_MAIN" should be commented out
 */ 
//#define DEBUG_MAIN


/* define filenames and sub directories */
#define SUBD_RESULTS "./results/"
#define SUBD_CONFIG  "./"
#define FILE_VIB_CONFIG "vib_params"
#define FILE_ERROR_LOG "errorlog"

/* define error Messages */
#define ERROR_LOGFILE "Error creating log file: "
#define ERROR_XMLFILE "Error accessing xml file: "
#define ERROR_XMLTAG "Error accessing xml tag: "
#define ERROR_FILE_OPEN "Error opening file: "
#define ERROR_FILE_READ "Error reading to file: "
#define ERROR_FILE_WRITE "Error writing to file: "
#define ERROR_HAT_SELECT "Error selecting hat device"
#define ERROR_IEPE_POWER "Error IEPE power, invalid selection"
#define ERROR_IN_ADDR "Error in channel: "
#define ERROR_HW_OVERRUN "Error hardware overrun\n"
#define ERROR_SCAN_OVERRUN "Error scan buffer overrun\n"
#define ERROR_NO_CHANNELS "Error incorrect number of channels: "


/* define tags for xml parameters file */

#define PAR_SENSITIVITY "sensitivity"
#define PAR_SCANRATE "scan_rate"
#define PAR_IEPE_POWER "iepe_supply"
#define PAR_SAMPLES_CHANNEL "samples_per_channel"
#define PAR_OPTIONS "options"
#define PAR_NOCHANNELS "number_of_channels"

#endif
