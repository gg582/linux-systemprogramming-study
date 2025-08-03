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

#define PORT       16384
#define MSG_LEN    8192  
#define HEADER_LEN 11
#define IOV_LEN    2

typedef struct sockaddr saddr;
typedef struct sockaddr_in saddr_in;

#define msleep(x) { \
    usleep(x*1000); \
}

const char* family_to_string(sa_family_t family) {
    switch (family) {
        case AF_INET : return "AF_INET"   ;
        case AF_INET6: return "AF_INET6"  ;
        case AF_UNIX : return "AF_UNIX"   ;
        default      : return "AF_UNKNOWN";
    }
}


char * pretty_print_saddr_in(saddr_in *info, char *msg) {
    char ip_address_as_string[17]; // add null padding. 16-length address + 1(null)
    inet_ntop(AF_INET, &(info->sin_addr), ip_address_as_string, sizeof(ip_address_as_string));
    sprintf(msg, "{\n");
    sprintf(msg + strlen(msg), "\tsin_family = %s,\n" , family_to_string(info->sin_family));
    sprintf(msg + strlen(msg), "\tsin_addr   = %s,\n" , ip_address_as_string);
    sprintf(msg + strlen(msg), "\tsin_port   = %d\n"  , ntohs(info->sin_port));
    sprintf(msg + strlen(msg), "}\n\0");
    return msg;
}

void server(int sockfd, saddr * cli_addr, socklen_t *cli_len) {
    // iovec of the server. this specifies buffer location
    struct iovec srv_iov[IOV_LEN];
    // this is a raw message header
    struct msghdr srv_msghdr;
    memset(&srv_msghdr, 0, sizeof(srv_msghdr));
    int srv_flags;
    int recvmsg_status;
    static socklen_t len = sizeof(saddr_in);
    char msg[MSG_LEN];
    char cli_msg[MSG_LEN];
    int sendmsg_status;
    while(true) {

        memset(msg, 0, sizeof(msg));
        memset(cli_msg, 0, sizeof(cli_msg));
        int recvlen = recvfrom(sockfd, cli_msg, MSG_LEN, 0, (saddr *)cli_addr, &len);
        if (recvlen < 0) {
            fprintf(stderr, "[RECEIVED] message length <= 0 (msg == %d)\n", recvlen);
        }  

        saddr_in * request_info = (saddr_in *)cli_addr;
        pretty_print_saddr_in(request_info, msg);
        if(strcmp(cli_msg, "ipv4") != 0) {

            char header[HEADER_LEN] = "CLIENT    \0";
            srv_iov[0].iov_base = header; //set a header
            srv_iov[0].iov_len  = strlen(header);  
            srv_iov[1].iov_base = cli_msg;
            srv_iov[1].iov_len = strlen(cli_msg);
        } else {
            char header[HEADER_LEN] = "ECHO_SRV  \0";
            srv_iov[0].iov_base = header; //set a header
            srv_iov[0].iov_len  = strlen(header);  
            srv_iov[1].iov_base = msg;
            srv_iov[1].iov_len  = strlen(msg);
        }
        // setup reply address to client origin
        srv_msghdr.msg_name    = cli_addr;
        srv_msghdr.msg_namelen = *cli_len;
        srv_msghdr.msg_iov     = srv_iov;
        srv_msghdr.msg_iovlen  = IOV_LEN;
        srv_msghdr.msg_flags   = 0;
        srv_flags = 0;
        sendmsg_status = sendmsg(sockfd, &srv_msghdr, srv_flags);
        if(sendmsg_status <= 0) {
            fprintf(stderr, "[SENT] message length <= 0 (msg == %d), errno:(%d)\n", sendmsg_status, errno);
        } else {
            fprintf(stderr, "sent message is: %s", msg);
        }
        msleep(10);
    }
}


int main(int argc, char *argv[]) {
    int sockfd;
    saddr_in srv_addr,
             cli_addr;
    int bind_status;
    if(argc != 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return 1;
    }

    // create udp socket(server)
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        fprintf(stderr, "error on socket(); errno:(%d)", errno);
        return errno;
    }

    // fill the structures to zero to prevent dirty values
    memset(&srv_addr, 0, sizeof(srv_addr));
    memset(&cli_addr, 0, sizeof(cli_addr));

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(PORT);
    // bind address to socket/
    int err = bind(sockfd, (saddr *)&srv_addr, sizeof(srv_addr));
    if(err<0) {
        fprintf(stderr, "error on bind(), errno(function): (%d), errno(main):(%d)", err, errno
);
        return(errno);
    }
     
    int cli_len = sizeof(cli_addr); // explicit length due to forced casting on saddr -> saddr_in
    server(sockfd, (saddr *)&cli_addr, &cli_len);
}
