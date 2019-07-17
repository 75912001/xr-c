//错误码,都为负数
#pragma once

namespace xr{
    //[-0xffff,-0xf000]
    enum ECODE_SYS{
        DISCONNECT_PEER     = -0xFFFF,//需要断开对方链接
        CONNECT_FAIL        = -0xFFFE,//连接失败
        TIME_OUT            = -0xFFFD,//超时
        UNDEFINED_CMD       = -0xFFFC,//未定义的命令号
    };
}