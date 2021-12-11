#ifndef NAGLES_H
#define NAGLES_H

#include<stdint.h>
#define SYN_FLAG 0
#define ACK_FLAG 1
#define SYN_ACK_FLAG 2

#pragma pack(1) 
typedef struct{
    uint64_t window_size ;
    uint8_t flag ;
} Packtets ;
typedef struct{
    int *unacked_packets ;
    int serverfd ;
} AckReceiver ;

#pragma pack(0) 

#endif // NAGLES_H