/*******************************************************************************
 * 文件名:iec103_simenzi_ym.cpp
 * 文件描述:见头文件
 * 创建日期:2018/05/14 14:47:01
 * 版本：Ver1.0
 *
 * Copyright © 2018 - 2018 mengqp.
 *
 * 历史版本：
 * 修改人                修改时间                修改版本        修改内容
 *
 *
 ******************************************************************************/
#include "iec103_simenzi_ym.h"
#include "../../share/global.h"

/*******************************************************************************
 * 功能描述:构造函数
 ******************************************************************************/
Ciec103_simenzi_ym::Ciec103_simenzi_ym (void) : CIEC103()
{

}   /*-------- end 构造函数 -------- */

/*******************************************************************************
 * 功能描述:析构函数
 ******************************************************************************/
Ciec103_simenzi_ym::~Ciec103_simenzi_ym (void)
{

}   /*-------- end 析构函数 -------- */

/*******************************************************************************
 * 类:Ciec103_simenzi_ym
 * 函数名:M_IT_TA_3_SIPROTEC_Frame( BYTE *buf, int len )
 * 功能描述: 单独处理西门子的遥脉
 * 参数:void
 * 被调用:
 * 返回值:BOOL
 ******************************************************************************/
BOOL Ciec103_simenzi_ym::M_IT_TA_3_SIPROTEC_Frame( BYTE *buf, int len )
{
    BYTE byYmNum = buf[7] & 0x7f;
    BYTE byFunType;
    BYTE byInfoIndex;
    WORD wPnt;
    DWORD dwYmValue;
    QWORD qYmValue;
    BYTE byYmBuf[4];
    int i = 0;
    bool s = false;
    // inf0 val0
    // head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+fun:1+inf:1+ymnum*8+cs:1+0x16:1
    if( ( 14 + byYmNum * 8) != len )
        return FALSE;

    DisplayCot( buf[8] );
    byFunType = buf[10];
    byInfoIndex = buf[11];

    for ( i=0; i<byYmNum; i++)
    {
        if( !GetModulePnt( IEC103_YM_DATATYPE, byFunType, byInfoIndex+i, wPnt ) )
        {
            continue;
        }

        if ( 0 != (buf[15+8*i] & 0x10) )
        {
            s = 1;
        }
        buf[15+8*i] &= 0x0f;
        GlobalCopyByEndian(byYmBuf,&buf[12 + 8 * i], 4);

        memcpy( &dwYmValue, byYmBuf, 4 );
        if ( 1 == s )
        {
            dwYmValue = ~dwYmValue + 1;
        }

        qYmValue = (QWORD)dwYmValue;

        m_pMethod->SetYmData( m_SerialNo, wPnt, qYmValue );
        sprintf( DebugBuf, "YmUpdate:dev%d pnt%d=%lu ", m_wDevAddr, wPnt, dwYmValue);
        print( DebugBuf );
    }

    return TRUE;

}   /*-------- end class Ciec103_simenzi_ym method M_IT_TA_3_SIPROTEC_Frame( BYTE *buf, int len ) -------- */
