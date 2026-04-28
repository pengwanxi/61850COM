#ifndef GroupBroad_H
#define GroupBroad_H

#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */
int GroupBroadInit(void);
int GroupBroadSend(unsigned char* pBuf,int Len);
unsigned short GroupBroadRecv(unsigned char* pBuf,int* pLen);


#ifdef	__cplusplus
}
#endif	/* __cplusplus */




#endif

