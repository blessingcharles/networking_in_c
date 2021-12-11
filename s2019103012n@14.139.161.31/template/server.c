#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <string.h>
#include<ctype.h>
#include<time.h>

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

    if(argc < 2){
        fprintf(stdout , "USAGE ./%s <port>",argv[0]);
        exit(EXIT_FAILURE);
    }

	int server_fd, new_socket;
	SA address;
	int addrlen = sizeof(address);
	
	char *html =    "HTTP/1.1 200 OK\n"
                    "Server: th3h04x-server\n"
                    "Connection: keep-alive\n"
                    "\n"
                    "Send Messages to this persistent Connection\r\n\r\n";
	
	int PORT = atoi(argv[1]);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
	
	server_fd = create_server(address);

    printf("server file descriptor : %d \n",server_fd);
    printf("Multithreaded th3h04x webserver listening on : %d\n",PORT);

    
	return 0;
}
