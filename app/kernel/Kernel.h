//Kernel.h
//20110506 dada create

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <stdexcept>

#include "Common.h"

//#include "str4i.h"

#include "AppDefine.h"
#include "KernelUtil.h"
#include "ErrorUtil.h"
#include "DataLog.h"
#include "WhiteList.h"
#include "DataBaseDBC.h"
#include "Session.h"
#include "BusinessLog.h"
//#include "Compress.h"

#include "FirePacket.h"

#include "ActivitySingleton.h"
#include "BattleSingleton.h"
#include "DataSingleton.h"
#include "DBCBase.h"
#include "ProtobufDispatcher.h"
#include "ConfigManager.h"
#include "ConfigPB.h"
#include "ConfigInc.h"

using std::runtime_error;
using std::deque;
using std::map;
using std::string;
using std::vector;
using std::set;
using std::list;
using std::min;
using std::max;
using std::bitset;


#endif //__KERNEL_H__
