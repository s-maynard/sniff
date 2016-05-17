#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <fcntl.h>
#include <netdb.h>
#include <net/if.h>



//Way 1
//~ char cmd[128];
//~ 
//~ FILE * log;
//~ 
//~ 
//~ char * location = "foobar";
//~ 
//~ //malloc?
//~ 
//~ log = fopen("/var/log/sentlog", "r+");
//~ 
//~ char location_1[256];
//~ 
//~ snprintf(location_1, sizeof(location1_1), "==%s==\n\n", location);
//~ 
//~ int locationStrLen = strlen(location_1);
//~ 
//~ fseek(log, 00, SEEK_SET);
//~ fwrite(location_1, sizeof(char), locationStrLen, log);
//~ fclose(log);
//~ 
//~ snprintf(cmd, sizeof(cmd), "cat /var/log/syslog >> /var/log/sentlog");
//~ system(cmd);
//~ snprintf(cmd, sizeof(cmd), "mv /var/log/messages >> /var/log/lastlog");
//~ system(cmd);

int main(int argc, char **argv)
{
	char cmd[128];
	char * location = "foobar";
	//Way 2
	FILE * log = fopen("/var/log/sentlog", "w");
	fprintf(log, "==%s==\n\n", location);
	fclose(log);

	system("cat /var/log/syslog >> /var/log/sentlog");
	system("mv /var/log/messages /var/log/lastlog");
	
	return 0;
}


void writeHeader(char *location)
{
	FILE * log = fopen("/var/log/sentlog", "w");
	fprintf(log, "==%s==\n\n", location);
	fclose(log);
}


//~ snprintf(cmd, sizeof(cmd), "cat /var/log/syslog >> /var/log/sentlog");
//~ system(cmd);
//~ snprintf(cmd, sizeof(cmd), "mv /var/log/messages /var/log/lastlog");
//~ system(cmd);
//~ 
//~ 
//~ system("mv /var/log/messages /var/log/lastlog");


//Way 3
