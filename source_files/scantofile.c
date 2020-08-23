/******************************
 * main()
 *
 * Acquires blocks of analog input data for a user-specified group
 * of channels. The acquisition is stopped when the specified
 * number of samples is acquired for each channel.
 ****************************/
#include <math.h>
#include "daqhats_utils.h"
#include "scantofile.h"
#include "utils.h"
#include "mcc172.h"
#include "daqhats.h"

/* local function declarations */
void get_log_file(char*, int);
void add_to_errorlog_quit(char*);
double utils_gettag_errchk_d(char*, char*);
int    utils_gettag_errchk_i(char*, char*);
void stop_if_error(int);
char* get_err_str(int);
void iepe_power_off();
void close_mcc172();

// folowing variables are global so error functions can close down the hardware
uint8_t address = 0;    
int channel_array[2 * 8];  //max of 8 boards with 2 channels each
int num_channels = 0;

int main(void)
{
    int result = RESULT_SUCCESS;
    uint8_t address = 0;
    char channel_string[512];
    char options_str[512];
    char date_time[MAX_ARRAY_SIZE] = {0};       //used to add date and time to log file
    int i;
    
    /* actual scan rate read from mcc172, as loaded scan rate is internally converted to
     * to the nearest valid rate of 51.2 kHz divided by an integer between 1 and 256. */
    double actual_scan_rate = 0.0;    
    double sample_time = 0.0;       //track time of sample displayed in log file
    double sample_time_inc = 0.0;

    uint32_t options = OPTS_DEFAULT;
    uint16_t read_status = 0;
    uint32_t samples_read_per_channel = 0;
    uint8_t synced;
    uint8_t clock_source;
    uint8_t iepe_enable;
    
    int32_t read_request_size = -1;     // '-1' means read all available samples
    double timeout = 5.0;


    /* set up file names */
    char config_file[MAX_ARRAY_SIZE] = {0};     //parameters from xml file to config the MCC172
    char log_file[MAX_ARRAY_SIZE] = {0};        //log of data collected from mcc172
    FILE *fp_logfile;
    utils_get_date_time(date_time, sizeof(date_time));

    char tmp[MAX_ARRAY_SIZE * 2] = {0};

    /* get name and path of configuration file
     * if errors, add to error log file, and quit.
     */ 
    if (!utils_getfilepath(config_file, sizeof(tmp), SUBD_CONFIG, FILE_VIB_CONFIG))
    {
        sprintf(tmp, "%s%s\n", ERROR_XMLFILE, config_file);
        add_to_errorlog_quit(tmp);
    }

    /* get options from xml parameters file, if error will return zero
     * zero is the default value for options, so no error checking
     */
    options =  (uint32_t) utils_getxmltag_d(config_file, PAR_OPTIONS);
  
    // Set the channel mask which is used by the library function
    // mcc172_a_in_scan_start to specify the channels to acquire.
    // The functions below, will parse the channel mask

    //get number of channelsfrom xml parameters file and check for errors
    int no_chs = utils_gettag_errchk_i(config_file, PAR_NOCHANNELS);
    #ifdef DEBUG_MAIN
    printf ("main() - no of channels %i\n",no_chs);
    #endif
    if ( (no_chs < 1) || (no_chs > 2) )
    {
         //put a message on error log file
        sprintf(tmp, "%s%i\n", ERROR_NO_CHANNELS, no_chs);
        add_to_errorlog_quit(tmp);
    }
    
    //set up mask dependant on number of channels
    uint8_t channel_mask =  ((no_chs == 1) ? CHAN0 :   (CHAN0 | CHAN1));
    
    convert_chan_mask_to_string(channel_mask, channel_string);

    num_channels = convert_chan_mask_to_array(channel_mask,
        channel_array);

    //get sensitivity from xml parameters file and check for errors
    double sensitivity = utils_gettag_errchk_d(config_file, PAR_SENSITIVITY);

    //get samples per channel from xml parameters file and check for errors
    double sampl_per_chan =  utils_gettag_errchk_d(config_file, PAR_SAMPLES_CHANNEL);
           
    uint32_t samples_per_channel = (uint32_t)sampl_per_chan;
    uint32_t buffer_size = samples_per_channel * num_channels;

    #ifdef DEBUG_MAIN
    printf("main() - buffer size: %i\n", buffer_size);
    #endif

    double read_buf[buffer_size];
    int total_samples_read = 0;

    //get scanrate from xml parameters file and check for errors
    double scan_rate = utils_gettag_errchk_d(config_file, PAR_SCANRATE);
    
    // Select an MCC172 HAT device to use.
    if (select_hat_device(HAT_ID_MCC_172, &address))
    {
        // Error getting device.
        add_to_errorlog_quit(ERROR_HAT_SELECT);
    }
    
    #ifdef DEBUG_MAIN
    printf ("main() - selected MCC 172 device at address %d\n", address);
    #endif

    // Open a connection to the device.
    result = mcc172_open(address);
    stop_if_error(result);

    //get IEPE power state from xml parameters file and check for errors
    result =  utils_getxmltag(config_file, PAR_IEPE_POWER, tmp);
    //check we read tag from file
    if (!result)
    { 
        //put a message on error log file
        sprintf(tmp, "%s%s\n", ERROR_XMLTAG, PAR_IEPE_POWER);
        utils_appendtofile(FILE_ERROR_LOG, SUBD_RESULTS, tmp);
    }

    // Check if power on or off
    if (strcmp(tmp, "on"))
    { 
        iepe_enable = 1;
    }
    else if (strcmp(tmp, "off"))
    { 
        iepe_enable = 0;
    }
    else
    {
        utils_appendtofile(FILE_ERROR_LOG, SUBD_RESULTS, ERROR_IEPE_POWER);
        mcc172_close(address);
        return -1;
    }
   
    for (i = 0; i < num_channels; i++)
    {
        result = mcc172_iepe_config_write(address, channel_array[i],
            iepe_enable);
        stop_if_error(result);

        result = mcc172_a_in_sensitivity_write(address, channel_array[i], 
            sensitivity);
        stop_if_error(result);
    }

    // Set the ADC clock to the desired rate.
    result = mcc172_a_in_clock_config_write(address, SOURCE_LOCAL, scan_rate);
    stop_if_error(result);
  
    // Wait for the ADCs to synchronize.
    do
    {
        result = mcc172_a_in_clock_config_read(address, &clock_source,
            &actual_scan_rate, &synced);
           
        stop_if_error(result);
        usleep(5000);
    } while (synced == 0);

    sample_time = 0.0;
    sample_time_inc = 1.0 / actual_scan_rate;

    #ifdef DEBUG_MAIN
    printf ("main() - Actual scan rate: %6.0f\n", actual_scan_rate);
    printf ("main() - Sample time inc: %11.9f\n", sample_time_inc);
    printf ("main() - Samples per channel: %i\n", samples_per_channel);
    #endif

    convert_options_to_string(options, options_str);

    // Configure and start the scan.
    result = mcc172_a_in_scan_start(address, channel_mask, samples_per_channel,
        options);
    stop_if_error(result);
    
    #ifdef DEBUG_MAIN
    uint32_t buffer_size_samples = 0;
    result = mcc172_a_in_scan_buffer_size(address, &buffer_size_samples);
    printf ("main() - Scan buffer size: %d\n", buffer_size_samples);
    #endif

    //get the name of log file that scanned data is saved in
    get_log_file(log_file, sizeof(log_file));
    
    fp_logfile = fopen(log_file, "w");
    if (fp_logfile == NULL)
    {
        sprintf(tmp, "%s%s\n", ERROR_LOGFILE, log_file);
        add_to_errorlog_quit(tmp);        
    }

     // Read the specified number of samples.
    do
    {
        result = mcc172_a_in_scan_read(address, &read_status, read_request_size,
            timeout, read_buf, buffer_size, &samples_read_per_channel);
           
        stop_if_error(result);
        #ifdef DEBUG_MAIN
        printf ("main() - Samples read per channel: %i\n", samples_read_per_channel);
        #endif

        if (read_status & STATUS_HW_OVERRUN)
        {
            #ifdef DEBUG_MAIN           
            printf(ERROR_HW_OVERRUN);
            #endif
            add_to_errorlog_quit(ERROR_HW_OVERRUN);
            break;
        }
        else if (read_status & STATUS_BUFFER_OVERRUN)
        {
            #ifdef DEBUG_MAIN  
            printf(ERROR_SCAN_OVERRUN);
            #endif
            add_to_errorlog_quit(ERROR_SCAN_OVERRUN);
            break;
        }

        total_samples_read += samples_read_per_channel;

        //add data to log file
        char tmp_str [MAX_ARRAY_SIZE];
        
        if (samples_read_per_channel > 0)
        {
            int index = 0;
            for (i = 0; i < samples_read_per_channel; i++)
            {
                index = i * 2;
                /*************************************
                 * Write scanned data to the log file
                 * Check if one or two channels
                *************************************/

                //remove zero to left of decimal point
                sprintf(tmp_str, "%.9f", sample_time);
                char* s = strstr(tmp_str, ".");
                if (num_channels == 1)
                {
                    fprintf(fp_logfile, "%s%s, %12.7f\n",
                        date_time, s, read_buf[index]);
                }
                else
                {
                    fprintf(fp_logfile, "%s%s, %12.7f, %12.7f\n",
                        date_time, s, read_buf[index], read_buf[index+1]);   
                }
                //increment time for a sample
                sample_time += sample_time_inc;    
            }
        }
        usleep(100000);
    }
    while ( (result == RESULT_SUCCESS) &&
           ((read_status & STATUS_RUNNING) == STATUS_RUNNING) );

     //now tidy up       
    result = mcc172_a_in_scan_stop(address);
    if (result != RESULT_SUCCESS)
    {
        #ifdef DEBUG_MAIN
        print_error(result);
        #endif
        utils_appendtofile(FILE_ERROR_LOG, SUBD_RESULTS, get_err_str(result));
    }
    
    result = mcc172_a_in_scan_cleanup(address);
    if (result != RESULT_SUCCESS)
    {
        #ifdef DEBUG_MAIN
        print_error(result);
        #endif
        utils_appendtofile(FILE_ERROR_LOG, SUBD_RESULTS, get_err_str(result));
    }
    
    // Turn off IEPE supply
    iepe_power_off();

    close_mcc172(address);

    return 0;   
}

/****************************
 * add_to_errorlog_quit() - add a message to the error log file, then quit
 * 
 * param message - message to be written to the log file
 ****************************/
void
add_to_errorlog_quit(char* message)
{
        utils_appendtofile(FILE_ERROR_LOG, SUBD_RESULTS, message);

        exit(-1);
}


/****************************
 * get_log_file() - get the name and absolute path to the log file
 *
 * Get the hostname, date and time which forms the filename.
 * Add absolute path to the filename.
 * If errors, add to error log, and quit as no point in proceeding
 *
 * param filename - holds complete file name on return
 *****************************/
void
get_log_file(char* filename, int size)
{
    char tmp[MAX_ARRAY_SIZE] = {0};
    char tmp1[MAX_ARRAY_SIZE * 2] = {0};
    
   //Get the hostname, date and time 
    if ( !utils_getnamedate(tmp, sizeof(tmp)) )
    {
        sprintf(tmp1, "%s%s\n", ERROR_LOGFILE, tmp);  
        add_to_errorlog_quit(tmp1);
    }
    //take hostname, date and time and add to absolute path 
    if ( !utils_getfilepath(filename, size, SUBD_RESULTS, tmp) )
    { 
        sprintf(tmp1, "%s%s\n", ERROR_LOGFILE, filename);
        add_to_errorlog_quit(tmp1);
    }     
    #ifdef DEBUG_MAIN
    printf ("main() - get_log_file() - filename: %s\n", filename);
    #endif
}


/****************************
 * utils_gettag_errchk_d() - gets a tag value as a double from an xml file
 *
 * Uses utils_getxmltag_d() to get the double
 * If not available, adds message to log file,
 * which will also terminate the program.
 *
 * param filename - name of xml file
 * param parameter - tag we are looking for
 * returns - the double value from xml file
*****************************/
double
utils_gettag_errchk_d(char* filename, char* parameter)
{
    double result = 0;
    char buffer[MAX_ARRAY_SIZE] = {0};
    
    result = utils_getxmltag_d(filename, parameter);
    //check we read tag from file
    if (!result)
    { 
        sprintf(buffer, "%s%s\n", ERROR_XMLTAG, parameter);
        add_to_errorlog_quit(buffer);
    }
    return result;
}


/****************************
 * utils_gettag_errchk_i() - gets a tag value as an int from an xml file
 *
 * Uses utils_getxmltag_i() to get the int
 * If not available, adds message to log file,
 * which will also terminate the program.
 *
 * param filename - name of xml file
 * param parameter - tag we are looking for
 * returns - the int value from xml file
*****************************/
int
utils_gettag_errchk_i(char* filename, char* parameter)
{
    int result = 0;
    char buffer[MAX_ARRAY_SIZE] = {0};
    
    result = utils_getxmltag_d(filename, parameter);
    //check we read tag from file
    if (!result)
    { 
        sprintf(buffer, "%s%s\n", ERROR_XMLTAG, parameter);
        add_to_errorlog_quit(buffer);
    }
    return result;
}


/****************************
 * stop_if_error() - called to check for errors during scanning
 *
 * If no error, do nothing, return.
 * If error, appends the error message to the error log file,
 * shuts down the hardware,
 * and terminates the program.
*****************************/
void
stop_if_error(int result)
{
    if (result != RESULT_SUCCESS)
    {
        utils_appendtofile(FILE_ERROR_LOG, SUBD_RESULTS, get_err_str(result));
        #ifdef DEBUG_MAIN
        print_error(result);
        #endif
        // Turn off IEPE supply
        iepe_power_off();
        
        close_mcc172();
        exit(-1);
    }
}


/****************************
 * iepe_power_off() - shutdown the hardware
 * 
 * If error appends it to the error log file.
*****************************/
void
iepe_power_off()
{
    int i = 0;
    int result = 0;
    for (i = 0; i < num_channels; i++)
    {
        result = mcc172_iepe_config_write(address, channel_array[i], 0);
        if (result != RESULT_SUCCESS)
        {
            
            #ifdef DEBUG_MAIN
            printf("main() - iepe_power_off()\n");
            print_error(result);
            #endif
        }
    }
}


/****************************
 * close_mcc172() - shutdown the hardware
 * 
 * If error appends it to the error log file.
*****************************/

void
close_mcc172()
{
    int result = 0;
    result = mcc172_close(address);
    if (result != RESULT_SUCCESS)
    {        
        utils_appendtofile(FILE_ERROR_LOG, SUBD_RESULTS, get_err_str(result));
        #ifdef DEBUG_MAIN
        printf("main() - close_mcc172()\n");
        print_error(result);
        #endif
    }
}


/****************************
 * get_err_str() - generates a string from an error or result code
 * 
 * It appends the error message to the error log file rather
 * rather than the console and will only output
 * the error message to the console if debugging is enabled.
 * param result - result code to be converted to a string
 * return - string for the result code
*****************************/
char*
get_err_str(int result)
{
    char * s = {0};
    switch(result)
    {
        // Success, no errors
        case 0:
            s = "Error - result_success";
            break;
    
        // A parameter passed to the function was incorrect.
        case -1:
            s = "Error - result_bad_parameter";
            break;
    
        // The device is busy.
        case -2:
            s = "Error - result_busy";
            break;
        
        // There was a timeout accessing a resource.
        case -3:
            s = "Error - result_timeout";
            break;
        
        // There was a timeout while obtaining a resource lock.
        case -4:
            s = "Error - result_lock_timeout";
            break;
    
        // The device at the specified address is not the correct type.
        case -5:
            s = "Error - result_invalid_device";
            break;
    
        // A needed resource was not available.
        case -6:
            s = "Error - result_resource_unavail";
            break;

        // Could not communicate with the device.
        case -7:
            s = "Error - result_comms_failure";
            break;
        
        // Some other error occurred.
        default:
            s = "Error - result_undefined";
    }
    return s;
}

