#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <string.h>
#include<time.h>
#include<math.h>

#define BUFF_SIZE 4096
#define SA struct sockaddr_in

int create_server(SA address){
	
	int sock_fd ;
	if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
	{
		fprintf(stderr , "failed to accept .");
		exit(EXIT_FAILURE);
	}

	if (bind(sock_fd, (struct sockaddr *)&address,
								sizeof(address))<0)
	{
		fprintf(stderr , "failed to bind");
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

    int PORT = atoi(argv[1]);

	int server_fd, new_socket;
	SA address;
	int addrlen = sizeof(address);
	
    
	char *calc = "\n1) ADDITION \n"
                 "2) MULTIPLICATION\n"   
                 "3) SQUARE ROOT\n"
                 "4) DIVISION\n"
                 "5) EXIT\n" 
                "\nENTER OPERATION: " ;
	
	
	address.sin_family = AF_INET;
	address.sin_port = htons( PORT );
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
	
	server_fd = create_server(address);

    printf("server file descriptor : %d \n",server_fd);
    printf("Multithreaded calculated server listening on : %d\n",PORT);
    char buffer[BUFF_SIZE] = {0} ;

    SA host_addr, client_addr;
    socklen_t length = sizeof(SA);

    int recvbytes = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &length);
    int sentbytes = sendto(server_fd, calc, strlen(calc) + 1, 0, (struct sockaddr*)&client_addr, length);

    while(1){

            bzero(buffer , BUFF_SIZE);
            recvbytes = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &length);
            int operator = atoi(buffer) ;


            bzero(buffer , BUFF_SIZE);
            recvbytes = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &length);
            int num1 = atoi(buffer) ;

            bzero(buffer , BUFF_SIZE);
            recvbytes = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &length);
            int num2 = atoi(buffer) ;
            double n = num1 ;
            
            int result = 0;
            switch (operator)
                {
                    case 1:
                        result = num1 + num2 ;
                        break;
                    case 2:
                        result = num1 * num2 ;
                        break;
                    case 3:
                        result = sqrt(n) ;
                        break;
                    case 4:
                        result = num1/num2 ;
                        break;
                    default:
                        result = 0 ;
                }
            printf("The server answer is %d\n" , result);
            bzero(buffer , BUFF_SIZE);
            sprintf(buffer , "The server answer is %d" , result);
            sentbytes = sendto(server_fd, buffer, strlen(buffer) + 1, 0, (struct sockaddr*)&client_addr, length);
            
            fflush(stdout);

            
    }

    
	return EXIT_SUCCESS;
}
