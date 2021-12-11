#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netinet/in.h>
#include<string.h>

#define SA struct sockaddr_in
#define BUFFER_SIZE 1024

int create_client(char *hostname , int port){

    int sock_fd ;
    if( (sock_fd = socket(AF_INET , SOCK_STREAM , 0) )< 0){
        fprintf(stderr , "failed to open socket");
        exit(EXIT_FAILURE);
    }

    SA address ;
    address.sin_family = AF_INET ;
    address.sin_port = htons(port);

    if(inet_pton(AF_INET , hostname ,&address.sin_addr) <= 0){
        fprintf(stderr , "failed to convert host text to binary form\n");
        exit(EXIT_FAILURE);
    }
    
    if(connect(sock_fd , (struct sockaddr *)&address , sizeof(address) ) < 0){
        fprintf(stderr , "failed to connect to %s\n",hostname);
        exit(EXIT_FAILURE);
    }
    return sock_fd ;
}

int main(int argc ,  char **argv){

    if(argc < 3){
        fprintf(stderr , "USAGE %s hostname port" , argv[0]);
        exit(EXIT_FAILURE);
    }

    char *request = "GET / http/1.1\r\nUser-Agent: th3h04x-client\r\n\r\n";

    int client_fd = create_client(argv[1] , atoi(argv[2]));

    printf("succesfully connected to host via opened fd : %d\n",client_fd);

    send(client_fd , request , strlen(request) , 0);
    
    char buffer[1024] = {0};

    if(recv(client_fd , buffer , BUFFER_SIZE , 0) < 0){
        fprintf(stderr , "failed to receive from server\n");
        exit(EXIT_FAILURE);
    }

    printf("SERVER RESPONSE \n%s" , buffer);

    return EXIT_SUCCESS ;
}