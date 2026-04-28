/*
 *
 * (C) 2014 David Lettier.
 *
 * http://www.lettier.com/
 *
 * NTP client.
 *
 * Compiled with gcc version 4.7.2 20121109 (Red Hat 4.7.2-8) (GCC).
 *
 * Tested on Linux 3.8.11-200.fc18.x86_64 #1 SMP Wed May 1 19:44:27 UTC 2013 x86_64 x86_64 x86_64 GNU/Linux.
 *
 * To compile: $ gcc main.c -o ntpClient.out
 *
 * Usage: $ ./ntpClient.out
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <math.h>

#include "ini.h"

#define EMU2000_BUSLINE_INI_NAME "/mynand/config/BusLine.ini"
#define NTP_TIMESTAMP_DELTA 2208988800ull

#define LI(packet)                                                             \
    (uint8_t)((packet.li_vn_mode & 0xC0) >> 6) // (li   & 11 000 000) >> 6
#define VN(packet)                                                             \
    (uint8_t)((packet.li_vn_mode & 0x38) >> 3) // (vn   & 00 111 000) >> 3
#define MODE(packet)                                                           \
    (uint8_t)((packet.li_vn_mode & 0x07) >> 0) // (mode & 00 000 111) >> 0

void error(char *msg)
{
    perror(msg); // Print the error message to stderr.

    exit(0); // Quit the process.
}

#define NTPCLIENT_INI_MATCH(s, n)                                              \
    strcmp(section, s) == 0 && strcmp(name, n) == 0

/*  */
typedef struct _NTPCLIENT_INI_INFO {
    char host[128];
    int port;
    int timezone;
} NTPCLIENT_INI_INFO;

/**
 *  \brief busline 的读取处理
 *  \param
 *  \return 1成功
 */
static int ntpclient_info_handle(void *user, const char *section,
                                 const char *name, const char *value)
{
    NTPCLIENT_INI_INFO *p = (NTPCLIENT_INI_INFO *)user;
    if (NTPCLIENT_INI_MATCH("NtpClient", "ServerIP")) {
        strncpy(p->host, value, sizeof(p->host) - 1);
        p->host[sizeof(p->host) - 1] = '\0';
    }
    else if (NTPCLIENT_INI_MATCH("NtpClient", "ServerPort")) {
        p->port = atoi(value);
    }
    else if (NTPCLIENT_INI_MATCH("NtpClient", "timezone")) {
        p->timezone = atoi(value);
    }
    else {
        return 0; /* unknown section/name, error */
    }

    return 1;
}

int main(int argc, char *argv[])
{
    int sockfd,
        n; // Socket file descriptor and the n return result from writing/reading from the socket.

    int portno = 123; // NTP UDP port number.
    char *host_name = "us.pool.ntp.org"; // NTP server host-name.
    int interval = 120; // Polling interval in seconds
    int timezone = 0; // Timezone offset in hours (0 for UTC)

    if (argc == 1 || argc == 2) {
        NTPCLIENT_INI_INFO devinfo;
        int ret = ini_parse(EMU2000_BUSLINE_INI_NAME, ntpclient_info_handle,
                            &devinfo);
        if (ret > 0) {
            host_name = devinfo.host;
            portno = devinfo.port;
            timezone = devinfo.timezone;
        }
    }
    else if (argc == 3) {
        host_name = argv[1]; // NTP server host-name from the command-line.
        portno = atoi(argv[2]); // NTP server port number from the command-line.
    }
    else if (argc == 4) {
        host_name = argv[1]; // NTP server host-name from the command-line.
        portno = atoi(argv[2]); // NTP server port number from the command-line.
        timezone = atoi(argv[3]); // Timezone offset in hours
    }

    printf("Using NTP server: %s:%d timezone=%d\n", host_name, portno,
           timezone);

    // Structure that defines the 48 byte NTP packet protocol.

    typedef struct {
        uint8_t li_vn_mode; // Eight bits. li, vn, and mode.
        // li.   Two bits.   Leap indicator.
        // vn.   Three bits. Version number of the protocol.
        // mode. Three bits. Client will pick mode 3 for client.

        uint8_t stratum; // Eight bits. Stratum level of the local clock.
        uint8_t poll; // Eight bits. Maximum interval between successive messages.
        uint8_t precision; // Eight bits. Precision of the local clock.

        uint32_t rootDelay; // 32 bits. Total round trip delay time.
        uint32_t
            rootDispersion; // 32 bits. Max error aloud from primary clock source.
        uint32_t refId; // 32 bits. Reference clock identifier.

        uint32_t refTm_s; // 32 bits. Reference time-stamp seconds.
        uint32_t refTm_f; // 32 bits. Reference time-stamp fraction of a second.

        uint32_t origTm_s; // 32 bits. Originate time-stamp seconds.
        uint32_t origTm_f; // 32 bits. Originate time-stamp fraction of a second.

        uint32_t rxTm_s; // 32 bits. Received time-stamp seconds.
        uint32_t rxTm_f; // 32 bits. Received time-stamp fraction of a second.

        uint32_t
            txTm_s; // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
        uint32_t txTm_f; // 32 bits. Transmit time-stamp fraction of a second.

    } ntp_packet; // Total: 384 bits or 48 bytes.

    // Create and zero out the packet. All 48 bytes worth.

    ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    memset(&packet, 0, sizeof(ntp_packet));

    // Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.

    *((char *)&packet + 0) =
        0x1b; // Represents 27 in base 10 or 00011011 in base 2.

    // Create a UDP socket, convert the host-name to an IP address, set the port number,
    // connect to the server, send the packet, and then read in the return packet.

    struct sockaddr_in serv_addr; // Server address data structure.
    struct hostent *server; // Server data structure.

    /* sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // Create a UDP socket. */

    /* if (sockfd < 0) */
    /*     error("ERROR opening socket"); */

    server = gethostbyname(host_name); // Convert URL to IP.

    if (server == NULL)
        error("ERROR, no such host");

    // Call up the server using its IP address and port number.

    // Zero out the server address structure.

    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    // Copy the server's IP address to the server address structure.

    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);

    // Convert the port number integer to network big-endian style and save it to the server address structure.

    serv_addr.sin_port = htons(portno);

    /* if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) */
    /*     error("ERROR connecting"); */

    /* // Send it the NTP packet it wants. If n == -1, it failed. */

    /* n = write(sockfd, (char *)&packet, sizeof(ntp_packet)); */

    /* if (n < 0) */
    /*     error("ERROR writing to socket"); */

    /* // Wait and receive the packet back from the server. If n == -1, it failed. */

    /* n = read(sockfd, (char *)&packet, sizeof(ntp_packet)); */

    /* if (n < 0) */
    /*     error("ERROR reading from socket"); */

    /* // These two fields contain the time-stamp seconds as the packet left the NTP server. */
    /* // The number of seconds correspond to the seconds passed since 1900. */
    /* // ntohl() converts the bit/byte order from the network's to host's "endianness". */

    /* packet.txTm_s = ntohl(packet.txTm_s); // Time-stamp seconds. */
    /* packet.txTm_f = ntohl(packet.txTm_f); // Time-stamp fraction of a second. */

    /* // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server. */
    /* // Subtract 70 years worth of seconds from the seconds since 1900. */
    /* // This leaves the seconds since the UNIX epoch of 1970. */
    /* // (1900)------------------(1970)**************************************(Time Packet Left the Server) */

    /* time_t txTm = (time_t)(packet.txTm_s - NTP_TIMESTAMP_DELTA); */

    /* // Get current system time */
    /* time_t current_time; */
    /* time(&current_time); */
    /* printf("System Time: %s", ctime(&current_time)); */
    /* printf("NTP Time: %s", ctime((const time_t *)&txTm)); */

    /* // Calculate time difference */
    /* double diff = difftime(txTm, current_time); */
    /* printf("Time difference: %.2f seconds\n", diff); */

    /* // Only sync if difference is more than 3 seconds */
    /* if (fabs(diff) > 3) { */
    /*     char cmd[128]; */
    /*     snprintf(cmd, sizeof(cmd), "date -s \"%s\"", ctime((const time_t *)&txTm)); */
    /*     printf("Syncing system time: %s\n", cmd); */
    /*     system(cmd); */
    /*     system("hwclock -w"); */
    /* } */

    // Close socket and reopen every interval seconds
    while (1) {
        if (sockfd >= 0) {
            close(sockfd);
            sockfd = -1;
        }
        sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sockfd < 0)
            error("ERROR opening socket");

        bzero((char *)&serv_addr, sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;

        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
              server->h_length);

        // Copy the server's IP address to the server address structure.

        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
              server->h_length);

        // Convert the port number integer to network big-endian style and save it to the server address structure.

        serv_addr.sin_port = htons(portno);
        printf("ntp serv=%s port=%d tz=%d\n", server->h_addr, portno, timezone);

        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
            0)
            error("ERROR connecting");

        memset(&packet, 0, sizeof(ntp_packet));
        *((char *)&packet + 0) =
            0x1b; // Represents 27 in base 10 or 00011011 in base 2.
        n = write(sockfd, (char *)&packet, sizeof(ntp_packet));
        if (n < 0)
            error("ERROR writing to socket");

        n = read(sockfd, (char *)&packet, sizeof(ntp_packet));
        if (n < 0)
            error("ERROR reading from socket");

        packet.txTm_s = ntohl(packet.txTm_s);
        packet.txTm_f = ntohl(packet.txTm_f);
        time_t txTm = (time_t)(packet.txTm_s - NTP_TIMESTAMP_DELTA);

        // Get current system time
        time_t current_time;
        time(&current_time);

        // Calculate time difference
        // Calculate difference considering timezone
        double diff = difftime(txTm + (timezone * 3600), current_time);
        /* double diff = difftime(txTm, current_time); */

        printf("System Time: %s", ctime(&current_time));
        printf("NTP Time: %s", ctime((const time_t *)&txTm));
        printf("Time difference: %.2f seconds\n", diff);
        // Only sync if difference is more than 3 seconds
        if (fabs(diff) > 3) {
            char cmd[128];
            // Manually adjust time components for timezone
            // Apply timezone offset
            time_t adjusted_time = txTm + (timezone * 3600);
            /* time_t adjusted_time = txTm; */
            struct tm tm_info;
            gmtime_r(&adjusted_time, &tm_info); // Use UTC-based time functions

            // Format the time string manually
            snprintf(cmd, sizeof(cmd),
                     "date -s \"%04d-%02d-%02d %02d:%02d:%02d\"",
                     tm_info.tm_year + 1900, tm_info.tm_mon + 1,
                     tm_info.tm_mday, tm_info.tm_hour, tm_info.tm_min,
                     tm_info.tm_sec);
            printf("Syncing system time: %s\n", cmd);
            system(cmd);
            system("hwclock -w -u");
        }

        close(sockfd);
        sockfd = -1;
        sleep(interval);
    }

    return 0;
}
