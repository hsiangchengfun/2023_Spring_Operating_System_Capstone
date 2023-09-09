#ifndef _MBR_H
#define _MBR_H

// 0-445 bytes boots code data (446)
// 446-509 partitions (16*4 = 64)
// 510-511 Boot Signature (0x55AA)
//total 512 bytes


// each partition have 16 bytes in MBR  
// total at most 4 partitions
struct mbr_partition_tb {
    unsigned char status_flag;              //0x0
    unsigned char partition_begin_head;     //0x1
    unsigned short partition_begin_sector;  //0x2-0x3
    unsigned char partition_type;           //0x4
    unsigned char partition_end_head;       //0x5
    unsigned short partition_end_sector;    //0x6-0x7
    unsigned int starting_sector;           //0x8-0xB
    unsigned int number_of_sector;          //0xC-0xF
};

#endif