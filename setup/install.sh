#! /bin/sh
#Set up Raspberry Pi 4B for vibration monitoring
#all errors are appended to the file "err.log",
#this log file is deleted each time the script starts

#Once finished reboot  the Raspberry Pi

#Set up colours toutput messages
YELLOW='\033[1;33m'
# No Color
NC='\033[0m' 

#** Delete log file **
#
logfile=err.log
echo "\n${YELLOW}Removing old error log file - $logfile ${NC}"
rm -f $logfile

#** Ensure all software is up to date **
#
echo "\n${YELLOW}Ensuring all software is up to date ${NC}"
sudo apt update 2>>err.log


#** Change the name of this Raspberry Pi 4 **

#NOTE: if name has been previously changed, then it will have to be edited manually
#use sed command - stream editor 
#use sed -i s/word1/word2/I file
#replace word1 with word2 ignoring case 
# the -i says makes changes in the file rather than print changes to terminal
#get name from keyboard, timeout after 60 seconds
#
echo "\n${YELLOW}Changing hostname of raspberry pi ${NC}"
read -t 60 -p "Enter new hostname: " hostname >>err.log
#sudo sed -i s/raspberrypi/$hostnamelade/I /etc/hostname  2>>err.log
#sudo sed -i s/raspberrypi/$hostname/I     /etc/hosts     2>>err.log



#** Set up working directory for MCC 172IEPE Hat
#
mydir=mcc172
echo "\n${YELLOW}Setting up working directoryies for vibration analysis - $mydir ${NC}"
mkdir mcc172/ 2>>err.log
mkdir mcc172/configuration/ 2>>err.log
mkdir mcc172/results/ 2>>err.log

#Set up cron jobs to send results of vibration analysis up to the server
#
echo "\n${YELLOW}Setting up cron job to transfer vibration analysis results to server ${NC}"
crontab -u pi  /home/pi/mcc172/cron_daily_schedule  2>>err.log

#** Finished, reboot Raspberry Pi **
#
echo "\n${YELLOW}Rebooting system now ${NC}"
# sudo reboot  2>>err.log
