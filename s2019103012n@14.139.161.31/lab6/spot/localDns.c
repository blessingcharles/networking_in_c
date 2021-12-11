#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#define ROOTPORT 8041      
#define PORT 8044

int main() {
        int socketfd = 0, localfd = 0;
        int rootfd = 0, tldfd = 0, authfd = 0;
        socklen_t length = sizeof(struct sockaddr_in);
        struct sockaddr_in host_addr, root_addr, client_addr;
        char buffer[512];
        char reqip[30];
        int recvbytes, sentbytes;

        socketfd = socket(AF_INET, SOCK_DGRAM, 0);
        if(socketfd < 0) {
                fprintf(stderr, "Error in socket creation.\n");
                return -1;
        }

        host_addr.sin_family = AF_INET;
        host_addr.sin_port = htons(PORT);
        inet_pton(AF_INET, "127.0.0.1", &host_addr.sin_addr);

        if(bind(socketfd, (struct sockaddr*)&host_addr, sizeof(host_addr)) < 0) {
                fprintf(stderr, "Error in binding port to socket.\n");
                return -1;
        }
        fprintf(stdout,"[+] SERVER STARTED AT PORT : %d\n",PORT);

        while(1) {
                printf("\n-------------------------------------------------\n");
                recvbytes = recvfrom(socketfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &length);
                if(strncmp(buffer, "exit", sizeof("exit")) == 0)
                        break;
                fprintf(stdout, "[+] DNS QUERY: %s\n", buffer);

                fprintf(stdout, "##### Querying root DNS ######\n");
                //REQUEST TO ROOT DNS
                rootfd = socket(AF_INET, SOCK_DGRAM, 0);
                if(rootfd < 0) {
                        fprintf(stderr, "Error in socket creation.\n");
                        return -1;
                }      

                root_addr.sin_family = AF_INET;
                root_addr.sin_port = htons(ROOTPORT);
                inet_pton(AF_INET, "127.0.0.1", &root_addr.sin_addr);  
                
                sentbytes = sendto(rootfd, buffer, strlen(buffer) + 1, 0, (struct sockaddr*)&root_addr, length);
                recvbytes = recvfrom(rootfd, reqip, sizeof(reqip), 0, NULL, NULL);
                fprintf(stdout, "Server IP for %s: %s\n(returned by root DNS)\nReturning dns query results to client ...\n\n", buffer, reqip);
                close(rootfd);

                sentbytes = sendto(socketfd, reqip, strlen(reqip) + 1, 0, (struct sockaddr*)&client_addr, length);
        }
        close(socketfd);
        return 0;
}       