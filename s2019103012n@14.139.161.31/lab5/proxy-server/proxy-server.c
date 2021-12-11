#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 8022
#define SERVER_PORT 8015

#define BUFF_SIZE 4096
#define PROXY_LOG_FILENAME "proxylog.txt"
#define LAST_MODIFIED_HEADER "Last-Modified: "
#define PROXY_LOG_FILE_DELIMETER "#"
#define SA struct sockaddr_in


void get_key_value_pair(char* headers , char* key , char* value){
    
    char temp_headers[1024] = {0};
    strcpy(temp_headers , headers);

    char *ptr = strstr(temp_headers , key);

    char *token = strtok(ptr+strlen(key) , "\r\n");
    
    strcpy(value , token);
   
}
void get_contents_from_response(char *response , char *content_buffer){
    char temp_response[1024] = {0};
    strcpy(temp_response , response);
   
    char *ptr = strstr(temp_response , "contents\r\n");

    strcpy(content_buffer , ptr+strlen("contents\r\n"));

}
char* get_file_name(char* buffer ){

    int i = 0 ;
    char *delimiter = "\r\n\r\n";

    char *filename = (strstr(buffer , delimiter)+strlen(delimiter)) ;

    return filename ;

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
		fprintf(stderr , "failed to bind");
		exit(EXIT_FAILURE);
	}
	if (listen(sock_fd, 3) < 0)
	{
		fprintf(stderr , "failed to listen");
		exit(EXIT_FAILURE);
	}

	return sock_fd ;
}

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
void get_file_contents(FILE *fileptr , char *file_contents){
    char line[256] = {0};

    while(fgets(line , sizeof(line) , fileptr)){
        strcat(file_contents , line);
        strcat(file_contents,"\n");
    }
}
void modify_logfile(char *filename , char *timestamp){
    
    
    FILE *fileptr = fopen(PROXY_LOG_FILENAME , "a+");
    char line[256] = {0};
    char file_contents[BUFF_SIZE] = {0};

    while(fgets(line , sizeof(line) , fileptr)){

        if(strstr(line,filename)){
            strcat(file_contents , filename);
            strcat(file_contents,PROXY_LOG_FILE_DELIMETER);
            strcat(file_contents,timestamp);
          
        }
        else{
            strcat(file_contents , line);
        }
    }
    fileptr = fopen(PROXY_LOG_FILENAME , "w+");
    fprintf(fileptr , file_contents);
    fclose(fileptr);

}
void get_file_timestamp(char *filename , char *timestamp){
    FILE *fd = fopen(PROXY_LOG_FILENAME , "r");
    char temp_buff[BUFF_SIZE] = {0};
    char temp_file_name[BUFF_SIZE/8] = {0};

    strcpy(temp_file_name , filename);
    strcat(temp_file_name,PROXY_LOG_FILE_DELIMETER);
    get_file_contents(fd , temp_buff);
    
    char *ptr = strstr(temp_buff , temp_file_name);
    char *token = strtok(ptr+strlen(temp_file_name),"\n");
    strcpy(timestamp , token);
    
    fclose(fd);
}
void proxy_server_handler(int clientfd , char *filename){
    char temp_buff[BUFF_SIZE/4] = {0};
    char buff[BUFF_SIZE] = {0};
    char response_buffer[BUFF_SIZE] = {0};

    char default_response[BUFF_SIZE] =  "HTTP/1.1 200 OK\r\n"
                                        "Server: th3h04x-proxy-server\r\n"
                                        "Connection: close\r\n"
                                        "\r\n";
	
    char not_found_response[BUFF_SIZE] = "HTTP/1.1 404 NOT FOUND\r\n"
                                         "Server: th3h04x-proxy-server\r\n"
                                         "Connection: close\r\n"
                                         "\r\n";

    char modified_since_request[BUFF_SIZE] = {0};
    char get_file_contents_request[BUFF_SIZE] = {0};
    char response_header[BUFF_SIZE] = {0};
    char response_contents[BUFF_SIZE] = {0};
    char lastmodified_buffer[BUFF_SIZE] = {0};

    int server_fd = create_client("127.0.0.1",SERVER_PORT);

    FILE *fileptr =  fopen(filename , "r");
    char timestamp[64] = {0};
    if(fileptr == NULL){
        fprintf(stdout , "\nFILE NOT FOUND IN CACHE\n");
        //check if it present in the server
        strcat(get_file_contents_request,"GET /");
        strcat(get_file_contents_request,filename);

        send(server_fd ,get_file_contents_request , strlen(get_file_contents_request)+1 ,0 );
        recv(server_fd , response_buffer , BUFF_SIZE-1 , 0);
        printf("\nMAIN SERVER RESPONSE : \n%s\n" ,response_buffer );
        if(strstr(response_buffer , "404")){
        
        // [+] CONDITION 1 if file even not present in the server send 404 response status
            printf("Not found");
            send(clientfd , not_found_response , strlen(not_found_response)+1 , 0);
            return ;
        }
        
        // [+] CONDITION 2 if file not present in proxy server but present in main server

        //get the last modified buffer and file contents
        get_key_value_pair(response_buffer , "Last-Modified: ",lastmodified_buffer);
        get_contents_from_response(response_buffer , response_contents);
        strcat(default_response , response_contents);
        
        // writing to the new file which doesnot exist in the proxy
        fileptr = fopen(filename , "w");
        fputs(response_contents , fileptr);
        fclose(fileptr);

        fileptr = fopen(PROXY_LOG_FILENAME , "a+");

        //writing the new entry to log file
        fprintf(fileptr ,"\n");
        fprintf(fileptr , filename);
        fprintf(fileptr,PROXY_LOG_FILE_DELIMETER);
        fprintf(fileptr,lastmodified_buffer);

        fclose(fileptr);
        send(clientfd , default_response , strlen(default_response)+1 , 0);
        return ;
    }
    else{

        printf("\nFILE PRESENT IN THE PROXY SERVER\n");

        //check for last modification date with the main server
        strcat(get_file_contents_request,"GET /");
        strcat(get_file_contents_request,filename);
        strcat(get_file_contents_request , "\r\n");
        strcat(get_file_contents_request , "If-Modified-Since: ");
        
        get_file_timestamp(filename,timestamp);
        strcat(get_file_contents_request , timestamp);
        strcat(get_file_contents_request , "\r\n\r\n");

        send(server_fd ,get_file_contents_request , strlen(get_file_contents_request)+1 ,0 );
        recv(server_fd , response_buffer , BUFF_SIZE-1 , 0);
        printf("\nMAIN SERVER RESPONSE : \n%s\n" ,response_buffer );
        
        if(strstr(response_buffer , "304")){
            // [+] condition 3 304 STATUS CODE [NOT MODIFIED]
            // send the contents from the proxy server
            while(fgets(temp_buff , sizeof(temp_buff) , fileptr)){
                strcat(buff , temp_buff);
                strcat(buff , "\n");
            }
            strcat(default_response , buff);
            send(clientfd , default_response , strlen(default_response)+1 , 0);
        }
        else{
            // [+] condition 4 200 STATUS
            // file modified update the file content in the proxy server
            printf("\n[+] FILE MODIFIED\n");

            char file_contents[BUFF_SIZE] = {0};
            get_key_value_pair(response_buffer , LAST_MODIFIED_HEADER , timestamp);
            get_contents_from_response(response_buffer , file_contents);

            //writing to the modified file
            FILE *fd = fopen(filename , "w+");
            fprintf(fd , file_contents);
            fclose(fd);

            modify_logfile(filename , timestamp);

            strcat(default_response , file_contents);
            send(clientfd , default_response , strlen(default_response)+1 , 0);
        }
        
    }

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

    printf("Proxy server file descriptor : %d \n",server_fd);
    printf("Proxy Server listening on : %d\n",PORT);

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

            printf("Request from client : \n%s\n",buffer );

            char* filename = get_file_name(buffer);

            printf("FILE NAME REQUESTED : %s" , filename);

            proxy_server_handler(new_socket ,filename);
            // send(new_socket , not_found_response , strlen(not_found_response) , 0 );

            close(new_socket);
            return 0 ;
        }
        

    }
	return 0;
}
