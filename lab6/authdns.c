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
#define IPLOOKUP_TABLE_COUNT 4
#define IP_FOR_EACH_DNS_RECORDS 3
#define AUTHPORT 8043

extern int errno;

typedef struct {char *key ; int value ;} keyValuePairs;

keyValuePairs ip_lookuptable[] = {
            {"zomato.co.in" , 0} ,
            {"customer.zomato.co.in" , 0} ,

        };

int rotate_dns_ip(char *domain_name){
    for(int i = 0 ; i < IPLOOKUP_TABLE_COUNT ; i++){
        if(strcmp(domain_name , ip_lookuptable[i].key) == 0){
            int value = ip_lookuptable[i].value ;
            ip_lookuptable[i].value++ ;
            return value ;
        }
    }
    return -1 ;
}

int main() {
        int socketfd = 0, clientfd = 0, sentbytes, recvbytes;
        socklen_t length = sizeof(struct sockaddr_in);
        struct sockaddr_in host_addr, client_addr;
        char buffer[20];

        socketfd = socket(AF_INET, SOCK_DGRAM, 0);
        if(socketfd < 0) {
                fprintf(stderr, "Error in socket creation.\n");
                return -1;
        }

        host_addr.sin_family = AF_INET;
        host_addr.sin_port = htons(AUTHPORT);
        inet_pton(AF_INET, "127.0.0.1", &host_addr.sin_addr);

        if(bind(socketfd, (struct sockaddr*)&host_addr, sizeof(host_addr)) < 0) {
                fprintf(stderr, "Error in binding port to socket.\n");
                return -1;
        }
        printf("AUTHORITATIVE DNS SERVER FOR CO.IN STARTED AT PORT : %d\n",AUTHPORT);
        while(1) {
                recvbytes = recvfrom(socketfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &length);

                fprintf(stdout, "DNS QUERY : %s\n", buffer);
                FILE* fd = fopen("authdns.txt", "r");
                if(!fd) {
                        fprintf(stderr, "Could not access DNS records.\n");
                        sendto(socketfd, "ERROR", strlen("ERROR") + 1, 0, (struct sockaddr*)&client_addr, length);
                        continue;
                }
                char linebuff[80], filebuff[400], ip[40], tempbuff[80], lastbuff[80];
                char *temp, *iptemp;
                int flag = 0, i;
                linebuff[0] = '\0';
                lastbuff[0] = '\0';
                filebuff[0] = '\0';
                ip[0] = '\0';
                while( fgets(linebuff, sizeof(linebuff), fd) ) {
                        strcpy(tempbuff, linebuff);
                        temp = strtok(tempbuff, " ");
                        if(flag == 0 && strncmp(temp, buffer, strlen(temp)) == 0) {
                                flag = 1;
                                strcpy(lastbuff, linebuff);
                                iptemp = strtok(NULL, " ");
                                int counter = 0 ;
                                int curr_pointer = rotate_dns_ip(buffer)%IP_FOR_EACH_DNS_RECORDS;
                                int i = 0;
                                while(1){
                                    for(i = 0; *iptemp != ' ' && *iptemp != '\0'; i++, iptemp++)
                                        ip[i] = *iptemp;
                                    if(*iptemp == '\n' || counter == curr_pointer) break ;
                                    counter++ ;
                                    iptemp = strtok(NULL, " ");

                                }
                               
                                ip[i] = '\0';
                        }
                        else{
                                strcat(filebuff, linebuff);
                        }
                }
                fclose(fd);
                if(flag == 0) {
                        sentbytes = sendto(socketfd, "404", strlen("404") + 1, 0, (struct sockaddr*)&client_addr, length);
                }
                else {
                        int fdes = open("authdns.txt", O_WRONLY);
                        strcat(filebuff, lastbuff);
                        write(fdes, filebuff, strlen(filebuff));
                        close(fdes);
                        fprintf(stdout, "AUTHORITATIVE SERVER IP : %s\n\n", ip);
                        sentbytes = sendto(socketfd, ip, strlen(ip) + 1, 0, (struct sockaddr*)&client_addr, length);
                }
        }
        close(socketfd);
        return 0;
}