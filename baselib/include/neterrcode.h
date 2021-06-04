/**
 * @author:   周龄
 * @desc:     网络错误值定义, 便于模块间调用等统计
 */

#ifndef __NETERRCODE_H__
#define __NETERRCODE_H__


enum ENetErrCode
{
    NE_GETFL            = -10000,
    NE_SETFL            = -9999,
    NE_SOCKET           = -9998,
    NE_CONNECT          = -9997,
    NE_SELECT_TIMEOUT   = -9996,
    NE_UNKNOWN          = -9995,
    NE_SDNOTSET         = -9994,
    NE_RECV             = -9993,
    NE_SEND             = -9992,
    NE_GETSOCKOPT       = -9991,
    NE_SETSOCKOPT       = -9990,
    NE_SETRCVBUF        = -9989,
    NE_SETSNDBUF        = -9988,
    NE_SETRCVTIMEO      = -9987,
    NE_SETSNDTIMEO      = -9986,
    NE_INVALID_SD       = -9985,
    NE_INVALID_IP       = -9984,
    NE_INVALID_PORT     = -9983,
    NE_INPUT            = -9982,
    NE_OUTPUT           = -9981,
};

#endif

