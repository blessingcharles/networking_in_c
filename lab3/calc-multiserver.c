#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <string.h>
#include<time.h>

#define PORT 8027
#define BUFF_SIZE 4096
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
	
    
	char *calc = "\n1) ADDITION \n"
                 "2) SUBTRACTION\n"   
                 "3) MULTIPLICATION\n"
                 "4) DIVISION\n"
                 "5) EXIT\n" 
                "\nENTER OPERATION: " ;
	
	
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
        time_t t ;
        time(&t);

        printf("new client connected at -->%s \n spawning new child\n" , ctime(&t));
        
        if( fork() == 0  ){
            //child process
            char buffer[BUFF_SIZE] = {0} ;

            close(server_fd);
            int num1 , operator , num2 , result;
            char *str1 = "Enter number 1 : " ;
            char *str2 = "Enter number 2 : " ;
            
            char *exit_msg = "Thank you :(){:|:&};:" ;

            while(1){
                write(new_socket , calc , strlen(calc));
                read(new_socket , &operator , sizeof(int));

                if(operator == 5){
                    write(new_socket , exit_msg , strlen(exit_msg));
                    return EXIT_SUCCESS ;
                }

                write(new_socket , str1 , strlen(str1));
                read(new_socket , &num1 , sizeof(int));

                write(new_socket , str2 , strlen(str2));
                read(new_socket , &num2 , sizeof(int));

                printf("The client sends : %d %d\n",num1 , num2);

                switch (operator)
                {
                    case 1:
                        result = num1 + num2 ;
                        break;
                    case 2:
                        result = num1 - num2 ;
                        break;
                    case 3:
                        result = num1*num2 ;
                        break;
                    case 4:
                        result = num1/num2 ;
                        break;
                    default:
                        write(new_socket , exit_msg , strlen(exit_msg));
                        return EXIT_SUCCESS ;
                }
                printf("The server answer is %d\n" , result);
                sprintf(buffer , "The server answer is %d" , result);
                write(new_socket , buffer , strlen(buffer));
                fflush(stdout);
            }   
            return EXIT_SUCCESS;
        }
        

    }
	return EXIT_SUCCESS;
}
