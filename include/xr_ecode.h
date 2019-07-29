//错误码,都为负数
#pragma once

namespace xr{
    //[-0xC000,-0xFFFF]
    enum ECODE{
        DISCONNECT_PEER         = -0xC000,//需要断开对方链接
        CONNECT_FAIL            = -0xC001,//连接失败
        TIME_OUT                = -0xC002,//超时
        UNDEFINED_CMD           = -0xC003,//未定义的命令号
        SYS                     = -0xC004,//系统错误

        DB_SYS                 = -0xD000,//DB系统错误
        DB_AFFECTED_ROWS       = -0xD001,//DB影响了多行数据
        DB_RECORDS_NOT_EXIST   = -0xD002,//DB记录不存在
        DB_RECORDS_EXIST       = -0xD003,//DB记录已存在
    };    
}