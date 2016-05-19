/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

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

#include <libwit.h>
#include <witutil.h>
#include <wiringPi.h>
#include <PCD8544.h>

#ifdef __APPLE__
#define PID_FILE_NAME "/tmp/sniff.pid"
#else
#define PID_FILE_NAME "/var/run/sniff.pid"
#endif
#define MAX_HTTP_BUF_SZ (1024*1024)
#define MAX_NAME_BUF_SZ 128
#define INTF "wlan0"
#define _2GHz_IP "0.0.0.0"
#define _5GHz_IP "0.0.0.0"
#define Canary_IP "8.8.8.8"
#define SSID "ATGWiFi"
#define PASS "mystrotv"
#define CONFIG_PATH_EXT "/media/usb"
#define CONFIG_PATH_INT "/home/pi"
#define CONFIG_PATH_SIZE 128
#define CONFIG_NAME "wit"
#define CONFIG_NAME_SIZE 32
#define GET_TIMEOUT 20


static char interface[16];
static char ssid[32];
static char pass[128];
static char _2GHz_ip[128];
static char _5GHz_ip[128];
static char location[1024];
static char canary[128];
static int outages = 0;
static unsigned long outsecs = 0;
static int current_freq = 0;
static int current_chan = 0;
static int current_signal = 0;

static bool use_lcd = true;
static bool nm_running = false;
static bool daemonize = false;
static bool run_for_gdb = false;
static bool done = false;

static char cpu_arch[MAX_NAME_BUF_SZ];
static char srvr_name[MAX_NAME_BUF_SZ];
static int client_app_version = CLIENT_APP_VERSION;
static int client_lib_version = 0;
static char ID[64];
static char MAC_buf[64];
static char MAC[16];
static char IP[64];
static char rx_buf[MAX_HTTP_BUF_SZ];


/*****************************************************************************
 * SIGNALS
 */
static void
handle_signal(int signo)
{
    switch (signo) {
    case SIGINT:
        LOG(NOTICE, "signal (%d) exiting...", signo);
        done = true;
        break;
    default:
        LOG(WARN, "signal (%d) not handled!", signo);
        break;
    }
}

static void
setup_signals(void)
{
    struct sigaction s;

    memset(&s, 0, sizeof(s));
    s.sa_handler = handle_signal;
    s.sa_flags = 0;
    sigaction(SIGINT, &s, NULL);
    sigaction(SIGTERM, &s, NULL);
    sigaction(SIGUSR1, &s, NULL);
    sigaction(SIGUSR2, &s, NULL);

    s.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &s, NULL);
    sleep(4);
}

/*****************************************************************************
 * HELPERS
 */
// Print a string to a LCD line constrained to prevent overflow
static void
pr_lcd_line(int line, char* str, int flush)
{
    char lcdbuf[16];
    int x = 0;
    int y = line == 0? 0 : (line * 8) + 4;

    if (use_lcd) {
        strncpy(lcdbuf, str, 15);
        LCDdrawstring(0, y, lcdbuf);

        if (flush) {
            LCDdisplay();
        }
    }
}

// Print the header to the top LCD line followed by a black line
static void
pr_lcd_header(long int tick)
{
    if (use_lcd) {
        if (tick == -1)
            pr_lcd_line(0, "Configuration:", false);
    else if (tick & 0x1)
            pr_lcd_line(0, IP, false);
        else
            pr_lcd_line(0, MAC, false);

        LCDdrawline(0, 10, 83, 10, BLACK);
        LCDdisplay();
    }
}

// Print a time in seconds, minutes, hours, or days
static char*
pr_s_m_h_d(char* buf, int bufsz, char* str, unsigned long secs)
{
    if (secs < 60)
        snprintf(buf, bufsz, "%s %lds", str, secs);
    else if (secs < 3600)
        snprintf(buf, bufsz, "%s %ldm", str, secs/60);
    else if (secs < 86400)
        snprintf(buf, bufsz, "%s %ldh", str, secs/3600);
    else
        snprintf(buf, bufsz, "%s %ldd", str, secs/86400);

    return buf;
}

static char *
get_clientID(char *id)
{
    sprintf(MAC, "%s", get_mac_str(MAC_buf));
    sprintf(id, "HOUND_%s", MAC);
    return id;
}

static void
read_configuration(void)
{
    char cmd[128];
    char result[1024];
    char url[CONFIG_PATH_SIZE + CONFIG_NAME_SIZE];
    char line[1024];
    int lineNum = 0;
    char *lf = NULL;
    FILE* fp = NULL;

    snprintf(url, sizeof(url), "%s/%s.cfg", CONFIG_PATH_EXT, CONFIG_NAME);
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", CONFIG_PATH_EXT);
    execute_command(cmd, result, sizeof(result));
    LOG(INFO, "mkdir result: %s", result);
    snprintf(cmd, sizeof(cmd), "mount -L WIT %s", CONFIG_PATH_EXT);
    execute_command(cmd, result, sizeof(result));
    LOG(INFO, "mount result: %s", result);

    if(NULL == (fp = fopen(url, "r"))) {
        LOG(WARN, "could not open %s, trying last config...", url);
        snprintf(url, sizeof(url), "%s/%s.cfg", CONFIG_PATH_INT, CONFIG_NAME);

        if(NULL == (fp = fopen(url, "r"))) {
            LOG(ERROR, "could not open %s?!", url);
        }
    }

    if(NULL != fp) {
        while(NULL != fgets(line, sizeof(line), fp)) {
            lineNum++;

            // special case empty lines and comments
            if(('\n' == line[0]) || ('\r' == line[0]) || ('#' == line[0])) {
                LOG(DEBUG, "non-TLV line %d: %s", lineNum, line);
            } else {
                char* tag = NULL;
                char* val = NULL;

                // suppress the LF before we log it...
                if((lf = strchr(line, '\n')))
                    *lf = 0;

                LOG(DEBUG, "line %d: %s", lineNum, line);
                tag = strtok(line, ":");
                val = strtok(NULL, ":");

                if ((NULL == tag) || (NULL == val)) {
                    LOG(ERROR, "NULL parsing for tag or val");
                } else {
                    if (*val == ' ')
                        val++;

                    if (strstr(tag, "SSID")) {
                        LOG(INFO, "change %s %s to %s", tag, ssid, val);
                        strcpy(ssid, val);
                    } else if (strstr(tag, "PASS")) {
                        LOG(INFO, "change %s %s to %s", tag, pass, val);
                        strcpy(pass, val);
                    } else if (strstr(tag, "LOCATION_DESC")) {
                        LOG(INFO, "change %s '%s' to '%s'", tag, location, val);
                        strcpy(location, val);
                    } else if (strstr(tag, "CANARY_IP")) {
                        LOG(INFO, "change %s %s to %s", tag, canary, val);
                        strcpy(canary, val);
                    } else {
                        LOG(ERROR, "unknown tag: %s", tag);
                    }
                }
            }
        }

        (void)fclose(fp);

        if (strstr(url, "usb")) {
            snprintf(cmd, sizeof(cmd), "cp %s /home/pi/wit.cfg", url);
            execute_command(cmd, result, sizeof(result));
            LOG(INFO, "cp result: %s", result);
        }
    }

    if (use_lcd) {
        LCDclear();
        pr_lcd_header(-1);
        pr_lcd_line(1, ssid, false);
        pr_lcd_line(2, pass, false);
        pr_lcd_line(3, canary, false);
        pr_lcd_line(4, location, true);
        sleep(3);
    }
}

static void
check_wifi(char* interface_str)
{
    char cmd[128];
    char result[1024];

    if (!nm_running) {
        snprintf(cmd, sizeof(cmd), "NetworkManager");
        execute_command(cmd, result, sizeof(result));
        LOG(INFO, "start NetworkManager result: %s", result);
        nm_running = true;
    }

    snprintf(cmd, sizeof(cmd),
            "nmcli -f FREQ,SIGNAL,ACTIVE dev wifi list | grep yes");
    execute_command(cmd, result, sizeof(result));
    LOG(INFO, "check %s result: %s", interface_str, result);
    current_freq = atoi(result);
    current_chan = get_frequency_channel(current_freq);
    current_signal = atoi(&result[9]);
    LOG(INFO, "freq:%d, chan:%d, signal:%d",
        current_freq, current_chan, current_signal);
    get_ip_str(interface_str, IP);
}

static void
connect_wifi(char* interface_str)
{
    char cmd[128];
    char result[1024];

    if (!nm_running) {
        snprintf(cmd, sizeof(cmd), "NetworkManager");
        execute_command(cmd, result, sizeof(result));
        LOG(INFO, "start NetworkManager result: %s", result);
        nm_running = true;
    }

    snprintf(cmd, sizeof(cmd), "rm /etc/NetworkManager/system-connections/*");
    execute_command(cmd, result, sizeof(result));
    LOG(INFO, "clear %s connections result: %s", interface_str, result);

    snprintf(cmd, sizeof(cmd), "nmcli dev wifi con \"%s\" password \"%s\"", ssid, pass);
    execute_command(cmd, result, sizeof(result));
    LOG(INFO, "connect %s result: %s", interface_str, result);
    check_wifi(interface_str);
}

static void
disconnect_wifi(char* interface_str)
{
    char cmd[128];
    char result[1024];

    if (!nm_running) {
        snprintf(cmd, sizeof(cmd), "NetworkManager");
        execute_command(cmd, result, sizeof(result));
        LOG(INFO, "start NetworkManager result: %s", result);
        nm_running = true;
    }

    snprintf(cmd, sizeof(cmd), "nmcli dev disconnect iface %s", interface_str);
    execute_command(cmd, result, sizeof(result));
    LOG(INFO, "disconnect %s result: %s", interface_str, result);
}

static void
scan_APs(char* interface_str)
{
    char lcdbuf[16];
    AccessPoint *ap_list = NULL;
    AccessPoint *app = NULL;
    APChannel* ap_chan2 = NULL;
    APChannel* ap_chan5 = NULL;
    APChannel* ap_chan = NULL;
    int ret = 0;
    int aps = 0;

    if (use_lcd) {
        LCDclear();
        pr_lcd_header(0);
        pr_lcd_line(1, ssid, false);
        pr_lcd_line(2, "Scanning...", true);
    }

    ap_list = wifi_scan(interface_str);
    ap_chan2 = get_suggested_AP_settings(2, ap_list);
    ap_chan5 = get_suggested_AP_settings(5, ap_list);
    ap_chan = wifi_survey(interface_str);
    ret = save_WIT_settings(ap_chan2, ap_chan5);

    if (use_lcd) {
        app = ap_list;

        while(app) {
            app = app->next;
            aps++;
        }

        snprintf(lcdbuf, sizeof(lcdbuf), "found %d", aps);
        pr_lcd_line(3, lcdbuf, true);
    }

    free_apchannel(ap_chan2);
    free_apchannel(ap_chan5);
    free_apchannel(ap_chan);
    free_accesspoint(ap_list);
    sleep(3);
}

static int
ping_canary(char* ip, char* canary, int timeout, long int tick)
{
    char cmd[128];
    char result[1024];
    int ret = 0;

    snprintf(cmd, sizeof(cmd), "ping -w%d -c1 -I%s %s", timeout, ip, canary);
    execute_command(cmd, result, sizeof(result));
    LOG(DEBUG, "result: %s", result);

    if(strstr(result, "1 received")) {
        LOG(INFO, "intf addr %s succeeded at tick %ld", ip, tick);
    } else {
        LOG(WARN, "ping on intf addr %s at tick %ld failed after %d seconds",
            ip, tick, timeout);
        ret = -ENODATA;
    }

    return ret;
}

static void
connection_failed(char* interface, char* ip, char* canary, long int tick)
{
    char lcdbuf[16];
    int retry = 0;
    char* alt = (NULL == strstr(_2GHz_ip, ip))? _2GHz_ip : _5GHz_ip;

    LOG(INFO, "attempting longer timeout on ping...");

    if(0 != ping_canary(ip, canary, 10, tick)) {
        if(0 != ping_canary(alt, canary, 10, tick)) {
            LOG(ERROR, "connection lost on both interfaces at t:%ld", tick);
        } else {
            LOG(WARN, "ping alt addr %s succeeded at t:%ld", alt, tick);
        }
    } else {
        LOG(WARN, "longer ping from addr %s succeeded at t:%ld", ip, tick);
    }
}

static int
usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [options]\n"
        "Options:\n"
        " -d daemonize\n"
        " -g run for gdb or valgrind\n"
        " -l <level>:  Log output level (default: %d)\n"
        "                                  ALERT: %d\n"
        "                               CRITICAL: %d\n"
        "                                  ERROR: %d\n"
        "                                WARNING: %d\n"
        "                                 NOTICE: %d\n"
        "                                   INFO: %d\n"
        "                                  DEBUG: %d\n"
        " -s:          Use stderr for log messages\n"
        " -i: <wlanX>: Wireless Interface to use. wlan0 if unspecified \n"
        " -c: <value>: Contrast value (30 to 90) \n"
        "\n", progname, DEFAULT_LOG_LEVEL, LOG_ALERT, LOG_CRIT, LOG_ERR,
        LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG);
    fflush(stderr);

    return 1;
}

// pin setup
int _din = 1;
int _sclk = 0;
int _dc = 2;
int _rst = 4;
int _cs = 3;

// lcd contrast
// may need to modify for your screen!  normal: 30-90, default is:55
int contrast = 55;

/*****************************************************************************
 * MAIN
 */
int main(int argc, char **argv)
{
    char buf[64];
    char lcdbuf[16];
    struct sysinfo si;
    unsigned long avgCpuLoad = 0;
    unsigned long totalRam = 0;
    long int tick = 0;
    int ch;
    int temp;

    // check wiringPi setup
    if (wiringPiSetup() == -1) {
        fprintf(stderr, "! wiringPiSetup Error\n");
        use_lcd = false;
    }

init_lcd:
    if (use_lcd) {
        // init and clear lcd
        LCDInit(_sclk, _din, _dc, _cs, _rst, contrast);
        LCDclear();

        // show logo
        LCDshowLogo();
    }

    strncpy(cpu_arch, get_cpu_type(), sizeof(cpu_arch));
    strncpy(interface, INTF, sizeof(interface));
    strncpy(ssid, SSID, sizeof(ssid));
    strncpy(pass, PASS, sizeof(pass));
    strncpy(_2GHz_ip, _2GHz_IP, sizeof(_2GHz_ip));
    strncpy(_5GHz_ip, _5GHz_IP, sizeof(_5GHz_ip));
    strncpy(location, "no location set", sizeof(location));
    strncpy(canary, Canary_IP, sizeof(canary));

    while ((ch = getopt(argc, argv, "l:dgsc:")) != -1) {
        switch(ch) {
        case 'l':
            set_log_level(atoi(optarg));
            break;
        case 'c':
            temp = atoi(optarg);
            if (temp != contrast)
            {
                contrast = temp;
                goto init_lcd;
            }
            break;
        case 'd':
            daemonize = true;
            break;
        case 'g':
            run_for_gdb = true;
            daemonize = false;
            break;
        case 's':
            set_log_stderr(true);
            break;
        case 'i':
            strcpy(interface, optarg);
            break;
        default:
            return usage(argv[0]);
        }
    }

    if (!run_for_gdb && is_already_running(PID_FILE_NAME)) {
        fprintf(stderr, "%s already running!", argv[0]);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    if (!run_for_gdb && daemonize) {
        pid_t result = fork();

        if (result == -1)
        {
            fprintf(stderr, "Failed to fork: %s.", strerror(errno));
            fflush(stderr);
            return 1;
        }
        else if (result == 0)
        {
            //Create a session and set the process group id.
            setsid();
            fprintf(stderr, "sniff running as daemon...\n");
            fflush(stderr);
        }
        else
        {
            //parent
            return 0;
        }
    }

    // set module parameters...
    start_logger("sniff");
    setup_signals();
    read_configuration();
    connect_wifi(interface);
    get_clientID(ID);
    LOG(NOTICE, "arch = %s", cpu_arch);

    do {

        // Every hour...
        if ((tick % 3600) == 0) {
        }

        // Every ten minutes...
        if ((tick % 600) == 0) {
        }

        // Every thirty seconds...
        if ((tick % 30) == 0) {
            scan_APs(interface);
#if 0
            if(0 != ping_canary(_2GHz_ip, canary, 2, tick)) {
                connection_failed(interface, _2GHz_ip, canary, tick);
            } else if(0 != ping_canary(_5GHz_ip, canary, 2, tick)) {
                connection_failed(interface, _5GHz_ip, canary, tick);
            }
#endif
            // get system usage / info
            if(sysinfo(&si) != 0) {
                LOG(ERROR, "sysinfo failed (%d)", errno);
                snprintf(lcdbuf, sizeof(lcdbuf), "E:%d", errno);
            } else {
                pr_s_m_h_d(lcdbuf, sizeof(lcdbuf), "Up",  si.uptime);

                if (tick == 0)    // sync tick to uptime
                    tick = si.uptime / 5;
            }

            // uptime
            pr_s_m_h_d(buf, sizeof(lcdbuf), " Out", outsecs);
            strncat(lcdbuf, buf, sizeof(lcdbuf));
            // cpu load
            avgCpuLoad = si.loads[0] / 1000;
            // free mem
            totalRam = si.freeram / 1024 / 1024;
            LOG(INFO, "%s, CPU %ld%%, RAM %ld MB",
                    lcdbuf, avgCpuLoad, totalRam);
            check_wifi(interface);

            if (use_lcd) {
                LCDclear();
                pr_lcd_header(tick);
                pr_lcd_line(1, ssid, false);
                pr_lcd_line(3, lcdbuf, false); // lcdbuf already holds uptime...
                snprintf(lcdbuf, sizeof(lcdbuf), "%dMHz %2d %d%%",
                         current_freq, current_chan, current_signal);
                pr_lcd_line(2, lcdbuf, false);
                snprintf(lcdbuf, sizeof(lcdbuf),
                         "Out %d/%ld", outages, 1+(tick/5));
                pr_lcd_line(4, lcdbuf, true);
            }
        }

        // Every second...
        tick++;

        sleep(1);

    } while (!done);

    LOG(DEBUG, "leaving...\n");
    stop_logger();
    return 0;
}

