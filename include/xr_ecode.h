//错误码,都为负数
#pragma once

namespace xr{
    //[-0xE000,-0xEFFF]
    enum ECODE_SYS{
        DISCONNECT_PEER     = -0xE000,//需要断开对方链接
        CONNECT_FAIL        = -0xE001,//连接失败
        TIME_OUT            = -0xE002,//超时
        UNDEFINED_CMD       = -0xE003,//未定义的命令号
    };
}