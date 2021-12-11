#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "slidingwindow.h"
#define HOST "127.0.0.1"
#define WINDOW_SIZE 20
#define BUFF_SIZE 100
#define RTT 5

void nagle_algo_receiver(int clientfd){
        char buffer[BUFF_SIZE];
        Packtets synpacket ;
        Packtets ackpacket ;
        ackpacket.flag = ACK_FLAG ;
        ackpacket.window_size = WINDOW_SIZE ;
        Packtets synack_packet ;
        int sender_windowsize ;

        recv(clientfd,&synpacket,sizeof(synpacket)+1,0);
        printf("[+]RTT : %d\n",RTT);
        int window_size = synpacket.window_size ;

        ackpacket.window_size = WINDOW_SIZE ;
        printf("[+]sending ack\n");
        send(clientfd ,(void *)&ackpacket , sizeof(ackpacket)+1 , 0 );
        recv(clientfd,&synack_packet,sizeof(synack_packet)+1,0);
        printf("[+]received synack \n");

        printf("[ Receiving data ]\n");
        fflush(NULL);
        int end = FIN ;
        int last_seq ;
        int flag = 1;
        while(flag){
                int t = window_size ;
                printf("\n[Data] : \n");
                while(t--){
                   
                    Packtets df ;
                    if(recv(clientfd,&df,sizeof(df)+1,0) == -1){
                        printf("failed\n");
                        exit(0);
                    }
                    if(df.data == FIN){
                        printf("[-] FIN flag received .\n.[+] closing tcp connection\n");
                        flag = 0;
                        break;
                    }
                    printf("\n%d [seq num : %d] , \n",df.data,df.seq_num);
                    last_seq = df.seq_num ;
                }
                Packtets ack ;
                ack.ack_num = last_seq+1 ;
                printline();
                printf("Sending ack with [%d]\n",ack.ack_num);
                send(clientfd ,(void *)&ack , sizeof(ack)+1 , 0 );
        }
}

int init_server(uint32_t port) {
        int socketfd;
        struct sockaddr_in server_addr;

        socketfd = socket(AF_INET, SOCK_STREAM, 0);
        if(socketfd < 0) {
                fprintf(stderr, "Error in socket creation.\n");
                return -1;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET,HOST, &server_addr.sin_addr);

        if(bind(socketfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                fprintf(stderr, "Error in binding socket to port.\n");
                return -1;
        }

        if(listen(socketfd, 5) < 0) {
                fprintf(stderr, "Error in listening in %s:%d.\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
                return -1;
        }
        fprintf(stderr, "Listening at %s:%d.\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
        return socketfd;
}

int main(int argc , char ** argv){
        if(argc < 2){
                fprintf(stdout,"%s <PORT>",argv[0]);
                return -1 ;
        }
        uint32_t port = atoi(argv[1]);
        int serverfd = init_server(port);

        if(serverfd < 0){
                fprintf(stdout,"failed to open port\n");
                return -1;
        }
        struct sockaddr_in client_addr ;
        int length  = 0 ;
        while(1){
                int clientfd = accept(serverfd , (struct sockaddr *)&client_addr ,&length);
                if(clientfd < 0){
                        fprintf(stderr , "Failed to accept connection");
                }
                struct sockaddr *client_details = (struct sockaddr *)&client_addr ;
                nagle_algo_receiver(clientfd);
        }

        return 0 ;
}