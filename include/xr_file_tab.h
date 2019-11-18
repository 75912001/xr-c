//以tab为分隔符的文件行
//id	name	desc	icon
//物品id	名字	描述	iconid资源id
//1	物品1	描述1	7,3,1
//2	物品2	描述2	7,3,2
//3	物品3	描述3	7,3,3
//4	物品4	描述4	7,3,4
//5	物品5	描述5	7,3,5
//6	物品6	描述6	7,3,6

#pragma once

#include "xr_util.h"

namespace xr
{
struct file_tab_t
{
	//加载文件到内存
	int load(const char *file_path);
	//传入 id,name,icon
	//返回 多行,  每行为id,name,icon组成的vector
	std::vector<std::vector<std::string>> get(const std::vector<std::string> &title_vec);

private:
	std::map<std::string, uint32_t> title_map;			  //key:符串, val:字col列[0-n]
	std::vector<std::vector<std::string>> content_vector; //内容 字符串信息

	template <typename T>
	inline T get_val_def(const char *col_name, const std::vector<std::string> &line, T def)
	{
		T ret;
		return SUCC == this->get_val(col_name, line, ret) ? ret : def;
	}
	//获取指定的值
	//returns:SUCC:成功.FAIL:失败
	//data(传出值)
	template <typename T>
	inline int get_val(const char *col_name, const std::vector<std::string> &line, T &data)
	{
		uint32_t col = 0; //要取得值的列数
		{
			auto it = this->title_map.find(col_name);
			if (this->title_map.end() == it)
			{
				return FAIL;
			}
			col = it->second;
		}
		{
			std::string val = line[col];
			if (val.empty())
			{
				return FAIL;
			}

			string2T(val, data);
		}
		return SUCC;
	}
};
} //end namespace xr