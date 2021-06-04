/*
 * IBase.cpp
 *
 *  Created on: 2012-2-15
 *      Author: dada
 */

#include "IBase.h"

//GCC内联虚函数不自动生成函数体，需要在子类中显式定义
//否则出现链接错误：undefined reference to `vtable for ~???'
//因此超类虚析构函数改为非内联
IBase::~IBase()
{
}
