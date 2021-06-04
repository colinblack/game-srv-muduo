/*
 * KernelUtil.h
 *
 *  Created on: 2011-5-26
 *      Author: dada
 */

#ifndef KERNELUTIL_H_
#define KERNELUTIL_H_

#include "Kernel.h"
using namespace std;

namespace Kernel
{
	bool InitLog(const string &logName);
	bool Init(const string &configPath = APP_CONFIG_PATH);
}

#endif /* KERNELUTIL_H_ */
