#ifndef SLIDINGWINDOW_H
#define SLIDINGWINDOW_H

#include<stdint.h>
#define SYN_FLAG 0
#define ACK_FLAG 1
#define SYN_ACK_FLAG 2
#define FIN -9999

#pragma pack(1) 
typedef struct{
    uint64_t window_size ;
    uint8_t flag ;
    uint32_t seq_num;
    uint32_t ack_num;
    int data ;

} Packtets ;
typedef struct{
    int *unacked_packets ;
    int serverfd ;
} AckReceiver ;

void printline(){printf("\n-------------------------------------------\n");}
#pragma pack(0) 

#endif // SLIDINGWINDOW_H