#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "nagles.h"
#define HOST "127.0.0.1"
#define MSS 5
#define WINDOW_SIZE 20
#define BUFF_SIZE 100
int init_client(uint32_t port) {
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

int unacked_packets = 0 ;
void *myThreadFun(void *args){
    char buffer[BUFF_SIZE] = {0};
    AckReceiver *data = (AckReceiver *)args ;
    while(1){
            //listen for ack packets
            recv(data->serverfd ,buffer , BUFF_SIZE-1,0);
            if(strncmp(buffer , "ack" , 3) == 0){
                    unacked_packets-- ;
            }
    }
    return NULL ;
}

void nagle_algo(int serverfd){
        Packtets syn ;
        Packtets ack ;
        Packtets synack;
        uint64_t receiver_window = 0 ;
        syn.window_size = WINDOW_SIZE ;
        syn.flag = SYN_FLAG ;
        synack.flag = SYN_ACK_FLAG ;
        synack.window_size = WINDOW_SIZE;
        printf("[*] sending syn flag\n");
        send(serverfd ,(void *)&syn , sizeof(syn)+1 , 0 );
        recv(serverfd ,&ack , BUFF_SIZE-1 , 0 );
        receiver_window = ack.window_size ;
        printf("[+] Received Ack (receiver window size : %ld )\n" , ack.window_size);
        printf("[*] sending synack flag\n");
        send(serverfd ,(void *)&synack , sizeof(synack)+1 , 0);
        
        int available_data = 0 ;
        char buffer[BUFF_SIZE] = {0};

        char temp ;

        pthread_t thread_id;
        AckReceiver data ;
        data.serverfd = serverfd ;
        data.unacked_packets = &unacked_packets ;

        pthread_create(&thread_id, NULL, myThreadFun, &data);


        while(1){
                printf("\n(ENTER)>%s",buffer);
                scanf("%c",&temp);
                getchar();
                buffer[available_data] = temp;
                available_data++ ;
                if(available_data >= MSS && receiver_window >= MSS){
                        printf("[+] sending %s\n",buffer);
                        send(serverfd,buffer,strlen(buffer)+1 , 0);
                        unacked_packets++ ;
                        available_data = 0;
                        memset(buffer,0,BUFF_SIZE-1);
                }
                else{
                        if(unacked_packets){
                                //buffer the data ;
                                printf("# buffering %s\n",buffer);
                        }
                        else{
                                //send data
                                printf("# sending %s\n",buffer);
                                send(serverfd,buffer,strlen(buffer)+1 , 0);
                                unacked_packets++ ;
                                available_data = 0;
                                memset(buffer,0,BUFF_SIZE-1);
                        }
                }
        }
        pthread_join(thread_id, NULL);

}
int main(int argc , char **argv){
        
        if(argc < 2){
                fprintf(stdout,"%s <PORT>",argv[0]);
                return -1;
        }
        uint32_t port = atoi(argv[1]);
        int serverfd = init_client(port);

        if(serverfd < 0){
                fprintf(stdout,"failed to open port\n");
                return -1;
        }
        printf("[+] successfully initiated connection between host1\n");
        nagle_algo(serverfd);
        return 0;
}