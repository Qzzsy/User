/**
 ****************************************************************
 * @file iso14443b.h
 *
 * @brief 
 *
 * @author 
 *
 *
 ****************************************************************
 */
 
#ifndef ISO_14443B_H
#define ISO_14443B_H
/////////////////////////////////////////////////////////////////////
//ISO14443B COMMAND
///////////////////////////////////////////////////////////////////// 
#define	ISO14443B_ANTICOLLISION                  0x05
#define	ISO14443B_ATTRIB                         0x1D
#define	ISO14443B_HLTB                           0x50

#define FSDI 8 //Frame Size for proximity coupling Device, in EMV test. 身份证必须FSDI = 8

/////////////////////////////////////////////////////////////////////
//函数原型
/////////////////////////////////////////////////////////////////////
char pcd_request_b(unsigned char req_code, unsigned char AFI, unsigned char N, unsigned char *ATQB);
char pcd_slot_marker(unsigned char N, unsigned char *ATQB);
char pcd_attri_b(unsigned char *PUPI, unsigned char dsi_dri, unsigned char pro_type, unsigned char CID, unsigned char *answer);
char get_idcard_num(unsigned char *ATQB);
char pcd_halt_b(unsigned char *PUPI);
char select_sr(unsigned char *chip_id);
char read_sr176(unsigned char addr, unsigned char *readdata);
char write_sr176(unsigned char addr, unsigned char *writedata);
char get_prot_sr176(unsigned char lockreg);
char protect_sr176(unsigned char lockreg);
char completion_sr();
char pcd_change_rate_b(unsigned char CID, unsigned char *ATQB);



#endif
