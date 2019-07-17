//读取*.ini文件
//[section]
//key=val

#pragma once

#include "xr_util.h"

namespace xr{
	struct file_ini_t{
		//加载*.ini文件数据到内存
		void load(const char* file_path);
		//获取ini文件中对应的数据
		template <typename T>
		inline T get_def(const char* section, const char* name, T def){
			T ret;
			return SUCC == this->get(section, name, ret) ? ret : def;
		}
		//获取指定组中key的值
		//返回:true:成功.false:失败
		template <typename T>
		inline int get(const char* section, const char* name, T& data){
			std::string val = this->get_str(section, name);
			if (val.empty()){
				return FAIL;
			} else {
				string2T(val, data);
			}
			return SUCC;
		}
	private:
		typedef std::map<std::string, std::string> NAME_MAP;//key:名字, val:值
		std::map<std::string, NAME_MAP> section_map;//key:节, val:NAME_MAP
		std::string get_str(const char* section, const char* name);
	};
}//end namespace xr