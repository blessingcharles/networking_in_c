#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <string.h>
#include<ctype.h>
#include<time.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 8015
#define BUFF_SIZE 4096
#define LAST_MODIFIED_HEADER "Last-Modified: "
#define SA struct sockaddr_in

void get_key_value_pair(char* headers , char* key , char* value){
    
    char temp_headers[1024] = {0};
    strcpy(temp_headers , headers);

    char *ptr = strstr(temp_headers , key);

    char *token = strtok(ptr+strlen(key) , "\r\n");
    
    strcpy(value , token);
   
}

void get_file_contents(FILE *fileptr , char *file_contents){
    char line[256] = {0};

    while(fgets(line , sizeof(line) , fileptr)){
        strcat(file_contents , line);
        strcat(file_contents,"\n");
    }
}

void handle_last_modified_request(int clientfd ,char* filename, char* req_timestamp){
    
    char not_modified_response[BUFF_SIZE] = "HTTP/1.1 304 NOT MODIFIED\r\n"
                                            "Server: th3h04x-server\r\n"
                                            "Connection: close\r\n\r\n";
    char response[BUFF_SIZE] =  "HTTP/1.1 200 OK\r\n"
                                "Server: th3h04x-server\r\n"
                                "Connection: close\r\n\r\n";

    char last_modified[BUFF_SIZE/8] = {0};
    char file_contents[BUFF_SIZE] = {0};

    struct stat file_info ;
    stat(filename , &file_info);
    strcpy(last_modified , ctime(&file_info.st_mtime));
    last_modified[24] = 0 ;

    printf("PROXY TIMESTAMP [[%s]] | LAST UPDATED TimeStamp : [[%s]]",req_timestamp , last_modified);
   
    if(strcmp(req_timestamp , last_modified) == 0){
        // if not modified reply with 304
        printf("\n[+] NOT MODIFIED 304\n\n");
        send(clientfd , not_modified_response , strlen(not_modified_response)+1 , 0);
        return;
    }
    else{
        //MODIFIED
        printf("\n[+] MODIFIED 200\n");

        strcat(response,LAST_MODIFIED_HEADER);
        strcat(response,last_modified);
        strcat(response,"\r\n\r\n");
        FILE* fileptr = fopen(filename , "r");
        get_file_contents(fileptr , file_contents);
        strcat(response,"contents\r\n");
        strcat(response,file_contents);
        send(clientfd , response , strlen(response)+1 , 0);
    }

}
void handle_connection(int clientfd , char* filename){
    	
	char response[BUFF_SIZE] =   "HTTP/1.1 200 OK\r\n"
                                "Server: th3h04x-server\r\n"
                                "Connection: close\r\n";

    char not_found_response[BUFF_SIZE] = "HTTP/1.1 404 NOT FOUND\r\n"
                                         "Server: th3h04x-server\r\n"
                                         "Connection: close\r\n"
                                         "\r\n";

    struct stat file_info ;
    char last_modified[BUFF_SIZE/8];
    char file_contents[BUFF_SIZE];

    FILE* fileptr = fopen(filename , "r");

    if(fileptr == NULL){
        printf("FILE NOT FOUND IN SERVER %s\n" , filename);
        send(clientfd , not_found_response , strlen(not_found_response)+1 , 0);
        return;
    }
    int n = stat(filename , &file_info);
    if(n < 0){
        printf("FILE NOT FOUND IN SERVER %s\n" , filename);
        send(clientfd , not_found_response , strlen(not_found_response)+1 , 0);
        return;
    }

   
    strcpy(last_modified , ctime(&file_info.st_mtime));
    printf("Filename %s\n | TimeStamp : %s",filename , last_modified);

    strcat(response,"Last-Modified: ");
    strcat(response,last_modified);
    strcat(response,"\r\n\r\ncontents\r\n");
    get_file_contents(fileptr , file_contents);

    strcat(response,file_contents);
    send(clientfd , response , strlen(response)+1 , 0);
    return ;
}
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
        if( fork() == 0  ){
            //child process
            close(server_fd);
            time_t t;
            time(&t);  
            printf("Client Connected successfully  at %s\n",ctime(&t));

            char buffer[BUFF_SIZE] = {0};
            int read_bytes_count = read( new_socket , buffer, BUFF_SIZE);

            printf("Request from Proxy server : \n\n%s\n",buffer );
            if(strstr(buffer , "If-Modified-Since: ")){
                char filename[BUFF_SIZE/8] = {0};
                char timestamp_buff[BUFF_SIZE/8] = {0};

                get_key_value_pair(buffer , "GET /",filename);
                get_key_value_pair(buffer , "If-Modified-Since: " , timestamp_buff);
                handle_last_modified_request(new_socket ,filename, timestamp_buff);
            }
            else{
                // proxy server requesting if the proxy server donot have the file
                printf("--------------\n");
                handle_connection(new_socket , buffer+5);
            }
         
            close(new_socket);
            return 0 ;
        }
        

    }
	return 0;
}
