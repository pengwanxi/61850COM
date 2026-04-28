/*
 * =====================================================================================
 *
 *       Filename:  IEC101S_1997.cpp
 *
 *    Description:  101 1997
 *
 *        Version:  1.0
 *        Created:  2014年12月23日 15时29分36秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (), 
 *   Organization:  
 *
 *		  history:
 * =====================================================================================
 */

#include "IEC101S_1997.h"


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_1997
 *      Method:  CIEC101S_1997
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CIEC101S_1997::CIEC101S_1997 ()
{
	//设置装置数据起始地址
	m_wYxStartAddr = 0x001;
	m_wYcStartAddr = 0x701;
	m_wYkStartAddr = 0xb01;
	m_wYmStartAddr = 0xc01;
	m_wComStateAddr = 50000;

	//可变化的传送原因 公共地址 和 信息地字节长度
	m_byCotLen = 1;
	m_byAddrLen = 1;
	m_byInfoAddrLen = 2;

	//总召数据类型
	m_byTotalCallYx = 1;//单点遥信
	m_byTotalCallYc = 11;//测量值 标度化值
	m_byTotalCallYm = 15;//累积量

	m_byChangeYx = 1;	//单点信息
	m_bySoeYx = 30;    //cp56time2a 单点信息
	m_byChangeYc = 11; //标度化值
	m_byYkType = IEC101S_2002_YKSINGLE_TYPE;//单点遥信
}  /* -----  end of method CIEC101S_1997::CIEC101S_1997  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_1997
 *      Method:  ~CIEC101S_1997
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CIEC101S_1997::~CIEC101S_1997 ()
{
}  /* -----  end of method CIEC101S_1997::~CIEC101S_1997  (destructor)  ----- */
