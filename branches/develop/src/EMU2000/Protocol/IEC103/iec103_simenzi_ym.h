/*******************************************************************************
 * 文件名:iec103_simenzi_ym.h
 * 文件描述:现场西门子的遥脉有问题，从IEC.上单独处理ym
 * 创建日期:2018/05/14 14:39:26
 * 版本：Ver1.0
 *
 * Copyright © 2018 - 2018 mengqp.
 *
 * 历史版本：
 * 修改人                修改时间                修改版本        修改内容
 *
 *
 ******************************************************************************/

#ifndef IEC103_SIMENZI_YM_H
#define IEC103_SIMENZI_YM_H
#include "IEC103.h"

/*******************************************************************************
 *功能描述:
 *******************************************************************************/
class Ciec103_simenzi_ym : public CIEC103
{
  public:
    /* constructor */
    explicit Ciec103_simenzi_ym(void);
    /* distructor */
    virtual ~Ciec103_simenzi_ym(void);

  public:
    virtual BOOL M_IT_TA_3_SIPROTEC_Frame( BYTE *buf, int len );

};



#endif /* IEC103_SIMENZI_YM_H */

// This file is set to c + + mode when you set the following content to the bottom and reopen the file
// Local Variables:
// mode: c++
// End:
