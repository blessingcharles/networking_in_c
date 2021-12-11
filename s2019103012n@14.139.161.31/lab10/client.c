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
#include "slidingwindow.h"

#define HOST "127.0.0.1"
#define INITIAL_SEQUENCE_NUMBER 1
#define MSS 5
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

void slidingwindow_algo(int serverfd){

        int sliding_window_size = 0 , count = 0;
        printf("[*] ENTER SLIDING WINDOW SIZE : ");
        scanf("%d",&sliding_window_size);
        printf("[*]Enter no. of packets : ");
        scanf("%d",&count);

        int frames[count];
        printf("[*] Enter data : ");
        for(int i = 0 ; i < count ; i++){
            scanf("%d",&frames[i]);
        } 
        printline();
        Packtets syn ;
        Packtets ack ;
        Packtets synack;
        uint64_t receiver_window = 0 ;
        syn.window_size = sliding_window_size ;
        syn.flag = SYN_FLAG ;
        synack.flag = SYN_ACK_FLAG ;
        synack.window_size = sliding_window_size;
        printf("[*] sending syn flag\n");
        send(serverfd ,(void *)&syn , sizeof(syn)+1 , 0 );
        recv(serverfd ,&ack , sizeof(ack)+1, 0 );
        receiver_window = ack.window_size ;
        printf("[+] Received Ack \n" );
        printf("[*] sending synack flag\n");
        send(serverfd ,(void *)&synack , sizeof(synack)+1 , 0);
        int available_data = 0 ;
        char buffer[BUFF_SIZE] = {0};

        char temp ;
      
        int squence_number = INITIAL_SEQUENCE_NUMBER ;
        fflush(NULL);
       
        int start = 0 ;
        int finished = 1 ;
        printf("Threeway handshake finished\n");
        printline();
        while(start < count){
            printf("\n[ Sending data ]\n ");

            for(int i = 0 ; i < sliding_window_size ; i++ ){
                Packtets df ;
                df.seq_num = squence_number++ ;

                if(start >= count ){
                    df.data = FIN ;
                    printf("[+]closing connection\n");
                    send(serverfd ,(void *)&df , sizeof(df)+1 , 0);
                    break ;
                };
                df.data = frames[start];      
                printf("%d [with seq num %d] \n",df.data , df.seq_num);
                send(serverfd ,(void *)&df , sizeof(df)+1 , 0);
                start++ ;
            }
            Packtets acked ;

            recv(serverfd ,&acked , sizeof(acked)+1 , 0 );
            printf("Acknowledgement received with %d \n" , acked.ack_num);
        }
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
        slidingwindow_algo(serverfd);
        return 0;
}