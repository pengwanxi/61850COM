// Copyright (C)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/// \文件:	main.cpp
/// \概要:	协议主函数
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2017-09-25

#include <stdio.h>
#include "CProtocol_Spa.h"

extern "C" CProtocol *CreateProtocol(CMethod *pMethod);

// --------------------------------------------------------
/// \概要:	创建协议
///
/// \参数:	pMethod
///
/// \返回:	返回协议
// --------------------------------------------------------
CProtocol *CreateProtocol(CMethod *pMethod)
{
	CProtocol *pProtocol = NULL;
	pProtocol = new CProtocol_SPA;
	if(pProtocol){
		pProtocol->m_pMethod = pMethod;
		printf("ABB Spa DLL OK.\n");
	}

	return pProtocol;
}
