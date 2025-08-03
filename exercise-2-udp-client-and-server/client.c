/* Sockets, Inet library */
// INET
#include <arpa/inet.h>
#include <netinet/in.h>
// Sockets
#include <sys/socket.h>
// Standard headers
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// ERROR NUMBERS
#include <errno.h>
// TYPES
#include <sys/types.h>
// boolean (true/false)
#include<stdbool.h>
// loading this for msleep
#include<unistd.h>

#define PORT    16384
#define MSG_LEN 8192

typedef struct sockaddr saddr;
typedef struct sockaddr_in saddr_in;

#define msleep(x) { \
    usleep(x*1000); \
}

int main(int argc, char *argv[]) {
    int sockfd;
    saddr_in srv_addr;
    // Message buffer to send
    char send_buf[MSG_LEN];
    char recv_buf[MSG_LEN];
    char SRV_ADDR[17]; // 16-length, and a null padding
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <message> <ipv4 address>\n", argv[0]);
        return 1;
    } else {
        if(strlen(argv[2]) <= 16) {
            strncpy(SRV_ADDR, argv[2], 16);
            SRV_ADDR[16] = '\0';
        } else {
            fprintf(stderr, "IPv4 address length should not be over 16\n");
            return 1;
        }
    }

    // create udp socket(client)
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        fprintf(stderr, "socket() failed. errno:(%d)", errno);
        return errno;
    }
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET; //IPv4
    srv_addr.sin_port   = htons(PORT);
    if(inet_pton(AF_INET, SRV_ADDR, &srv_addr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton() failed. errno:(%d)", errno);
        close(sockfd);
        return errno;
    }
    strncpy(send_buf, argv[1], MSG_LEN);
    send_buf[MSG_LEN - 1] = '\0'; // null padding

    int addr_len = sizeof(srv_addr);
    ssize_t sent_bytes = sendto(sockfd, send_buf, strlen(send_buf), 0,
                                (saddr *)&srv_addr, addr_len);
    if (sent_bytes < 0) {
        fprintf(stderr, "sendto() failed");
        close(sockfd);
        return errno;
    }

    ssize_t recv_bytes = recvfrom(sockfd, recv_buf, MSG_LEN - 1, 0, (saddr *)&srv_addr, &addr_len);
    if(recv_bytes <= 0) {
        fprintf(stderr, "recvfrom() failed. errno:(%d)", errno);
        close(sockfd);
        return errno;
    }
    
    recv_buf[recv_bytes] = '\0'; // explicit null-termination
    printf("Received from server:\n%s\n", recv_buf);
    close(sockfd);

    return 0;
}
