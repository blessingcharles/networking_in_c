#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <string.h>
#include<ctype.h>
#include<time.h>

#define PORT 8023
#define BUFF_SIZE 1024
#define SA struct sockaddr_in

int create_server(SA address){
	
	int sock_fd ;
	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		fprintf(stderr , "failed to accept");
		exit(EXIT_FAILURE);
	}

	if (bind(sock_fd, (struct sockaddr *)&address,
								sizeof(address))<0)
	{
		fprintf(stderr , "failed to accept");
		exit(EXIT_FAILURE);
	}
	if (listen(sock_fd, 3) < 0)
	{
		fprintf(stderr , "failed to accept");
		exit(EXIT_FAILURE);
	}

	return sock_fd ;
}
int main(int argc, char  **argv)
{
	int server_fd, new_socket;
	SA address;
	int addrlen = sizeof(address);
	
	char *html =    "HTTP/1.1 200 OK\n"
                    "Server: th3h04x-server\n"
                    "Connection: keep-alive\n"
                    "\n"
                    "Send Messages to this persistent Connection\r\n\r\n";
	
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
	
	server_fd = create_server(address);

    printf("server file descriptor : %d \n",server_fd);
    printf("Multithreaded th3h04x webserver listening on : %d\n",PORT);

    while(1){
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                        (socklen_t*)&addrlen))<0)
        {
            fprintf(stderr , "failed to accept");
            exit(EXIT_FAILURE);
        }
        printf("new client --> spawning new child\n");

        if( fork() == 0  ){
            //child process
            time_t t;
            time(&t);  
            printf("Client Connected successfully  at %s\n",ctime(&t));
            close(server_fd);

            char buffer[BUFF_SIZE] = {0};
            int read_bytes_count = read( new_socket , buffer, BUFF_SIZE);

            printf("Request from client\n\n%s\n",buffer );
            send(new_socket , html , strlen(html) , 0 );

            char *servr_message = "message received succesfully\n";           

            while(1){
                bzero(buffer , BUFF_SIZE);
                if(read(new_socket , buffer , BUFF_SIZE ) < 0){
                    break ;
                }
                
                if(strcmp(buffer , "exit\n") == 0){

                    printf("\n[-] client closing the connection\n");
                    printf("\n--------------------------------------------------------------------------\n");

                    break;
                
                }
                printf("\nRequest from client:\n%s",buffer );
                
                send(new_socket , servr_message , strlen(servr_message) , 0 );

                fflush(stdout);
                
            }
            close(new_socket);
            return 0 ;
        }
        

    }
	return 0;
}
