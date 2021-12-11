#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SERVERPORT 8040
#define COMMAND_NOT_IMPLEMENTED "502"
#define FILE_NOT_FOUND "404"
#define HELP_COMMAND "help"

int get_connected_socket(long port) {
        int socketfd = 0, n = 0;
        struct sockaddr_in server_addr;
        
        socketfd = socket(AF_INET, SOCK_STREAM, 0);
        if(socketfd < 0) {
                fprintf(stderr, "Error in socket creation.\n");
                return -1;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        if(connect(socketfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                fprintf(stderr, "Error in connection.\n");
                return -1;
        }
        fprintf(stdout, "Connection %s:%d.\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
        return socketfd;
}

void file_transfer(int file_port , int control_fd) {
        int socketfd = 0;
        char command[20], buffer[512];
        command[0] = '\0';
        char cnfm;
        printf("[+] Data ");
        socketfd = get_connected_socket(file_port);
        if(socketfd < 0) {
                return ;
        }

        fprintf(stdout, "Enter exit to close the connection.\n\n");
        while(1) {
                fprintf(stdout, "\nftp> ");
                fgets(command,100,stdin);
                if(strncmp(command, "exit", strlen("exit")) == 0) {
                        send(control_fd, "exit", strlen("exit") + 1, 0);
                        break;
                }
                else if(strncmp(command ,HELP_COMMAND , strlen(HELP_COMMAND)) == 0){
                    send(control_fd , HELP_COMMAND , strlen(HELP_COMMAND)+1 , 0);
                    recv(socketfd , buffer , sizeof(buffer) , 0);
                    printf("\n[HELP]\n%s",buffer);
                    continue ;
                }
        }
        close(socketfd);
        return;
}
int main(int argc , char **argv) {

        if(argc < 3){
            fprintf(stderr , "USAGE %s <username> <password>\n" , argv[0]);
            return -1 ;
        }

        int socketfd;
        struct sockaddr_in server_addr;
        char buffer[512], *username = argv[1] , *password = argv[2];
        printf("\n[+] Authenticating With %s : %s\n",username , password );
        printf("[+] Conntrol ");
        socketfd = get_connected_socket(SERVERPORT);
        if(socketfd < 0) {
                return -1;
        }

        buffer[0] = '\0';
        strcat(buffer, "req_auth$");
        strcat(buffer, username);
        strcat(buffer, "$");
        strcat(buffer, password);
        strcat(buffer, "$");
        send(socketfd, buffer, strlen(buffer) + 1, 0);
        
        recv(socketfd, buffer, sizeof(buffer), 0);
        if(strncmp(buffer, "INVALID_CRED", strlen("INVALID_CRED")) == 0) {
                fprintf(stdout, "\nInvalid Credentials. Try Again.\n");
                exit(1);
        }
        if(strncmp(buffer, "ERROR", strlen("ERROR")) == 0) {
                fprintf(stdout, "Couldn't authorize credentials now. Try Again.\n");
                return -1;
        }
        int file_port = atoi(buffer);
        fprintf(stdout,"[+] Logged in as %s\n",username);
        file_transfer(file_port , socketfd);
        close(socketfd);
        return 0;
}