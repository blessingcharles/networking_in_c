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

int create_client(){

    int sock_fd ;
    if( (sock_fd = socket(AF_INET , SOCK_DGRAM , 0) )< 0){
        fprintf(stderr , "failed to open socket");
        exit(EXIT_FAILURE);
    }
    return sock_fd ;
}

int main(int argc ,  char **argv){

    if(argc < 2){
        fprintf(stderr , "USAGE %s port" , argv[0]);
        exit(EXIT_FAILURE);
    }
    int socketfd = create_client();
    int PORT = atoi(argv[1]);

    char buffer[BUFFER_SIZE] = {0};
    int n = 0 , operator , num1 , num2;

    struct sockaddr_in host_addr;
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &host_addr.sin_addr);

    

    int sentbytes = sendto(socketfd,"send options", strlen("send options") + 1, 0, (struct sockaddr*)&host_addr, sizeof(host_addr));
    int recvbytes = recvfrom(socketfd, buffer, sizeof(buffer), 0, NULL, NULL);
    
    printf("%s",buffer);
    scanf("%d",&operator);

    if(operator == 3){
        printf("[+]ENTER NUMBER : ");
        scanf("%d",&num1);
        num2 = 0 ;
    }
    else{
        printf("[+]ENTER NUMBER1 : ");
        scanf("%d",&num1);

        printf("[+]ENTER NUMBER2 : ");
        scanf("%d",&num2);
    }

    bzero(buffer , BUFFER_SIZE);
    sprintf(buffer ,"%d", operator);

    sendto(socketfd,buffer, strlen(buffer) + 1, 0, (struct sockaddr*)&host_addr, sizeof(host_addr));

    bzero(buffer , BUFFER_SIZE);
    sprintf(buffer ,"%d" ,num1);

    sendto(socketfd,buffer, strlen(buffer) + 1, 0, (struct sockaddr*)&host_addr, sizeof(host_addr));

    bzero(buffer , BUFFER_SIZE);
    sprintf(buffer ,"%d" ,num2);

    sendto(socketfd,buffer, strlen(buffer) + 1, 0, (struct sockaddr*)&host_addr, sizeof(host_addr));

    bzero(buffer , BUFFER_SIZE);

    recvbytes = recvfrom(socketfd, buffer, sizeof(buffer), 0, NULL, NULL);

    printf("%s\n",buffer);
    return EXIT_SUCCESS ;
}