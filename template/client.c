#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netinet/in.h>
#include<string.h>
#include<time.h>

#define SA struct sockaddr_in
#define BUFFER_SIZE 4096

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

void read_from_socket(char *buffer , int client_fd){

    bzero(buffer , 0);
    int n = read(client_fd , buffer , BUFFER_SIZE-1);
    if(n < 0){
        fprintf(stderr , "failed to receive message from server");
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0' ;

}

int main(int argc ,  char **argv){

    if(argc < 3){
        fprintf(stderr , "USAGE %s <hostname> <port>" , argv[0]);
        exit(EXIT_FAILURE);
    }
    int client_fd = create_client(argv[1] , atoi(argv[2]));

    printf("succesfully connected to host via opened fd : %d\n",client_fd);
    time_t t ;
    time(&t);

    printf("Connected At : %s\n" , ctime(&t));

    char buffer[BUFFER_SIZE] = {0};
    int n = 0 , operator , num1 , num2;
    while(1){
        
        read_from_socket(buffer , client_fd);

        printf("\n[SERVER MESSAGE] \n%s",buffer);
        scanf("%d" , &operator);
        write(client_fd , &operator , sizeof(int));
        fflush(stdout);

        read_from_socket(buffer , client_fd);
        printf("%s\n",buffer);
        if(operator == 5) break;

        scanf("%d",&num1);
        write(client_fd , &num1 , sizeof(int));

        read_from_socket(buffer , client_fd);
        printf("%s\n" , buffer);
        scanf("%d",&num2);
        write(client_fd , &num2 , sizeof(int));

        //answer
        int answer ;
        read_from_socket(buffer , client_fd);
        printf("%s\n" ,buffer);
        fflush(stdout);
    }

    return EXIT_SUCCESS ;
}