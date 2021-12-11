#include<stdio.h>
#include<string.h>

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
void main(){

    char a[1024] = "HTTP/1.1 304 NOT MODIFIED\r\n"
                    "Server: th3h04x-server\r\n"
                    "Connection: close\r\n\r\n"
                    "contents\r\n"
                    "hello\nnaeruifbwuibg\niebfruwefrubeiurfbiuq3fbui3f\n";

    char value[100] = {0};
    // get_key_value_pair(a , "Connection: " ,value );
    get_contents(a , value);

    printf("%s",value);
}