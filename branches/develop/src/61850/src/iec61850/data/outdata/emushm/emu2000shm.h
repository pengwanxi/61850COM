/**
 *   \file emu2000shm.h
 *   \brief emu2000 管理机共享内存
 */
#ifndef _EMU2000SHM_H_
#define _EMU2000SHM_H_

#include "outdata.h"

/* *
 *  \brief 初始化
 *  \param void
 *  \return ERR_OK 成功
 */
int emu2000shm_init(OUTDATA *pdata);
int emu2000shm_exit(OUTDATA *pdata);

#endif /* _EMU2000SHM_H_ */
