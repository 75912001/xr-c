#include "xr_file_ini.h"

namespace
{
//返回[section]中的 section
//若没有则返回空字符串
std::string __get_section_name(const std::string &line)
{
	if ('[' == line[0] && ']' == line[line.size() - 1])
	{
		std::string section_name(line, 1, line.size() - 2);
		return section_name;
	}
	static std::string section_name;
	return section_name;
}
//从字符串中找出name和val
//return:true:成功.false:失败
bool __get_name_and_val(const std::string &line, std::string &name, std::string &val)
{
	std::string::size_type pos = line.find('=');
	if (std::string::npos == pos)
	{
		return false;
	}
	std::string t_name(line, 0, pos);
	name = t_name;
	std::string t_val(line, pos + 1, line.size() - 1);
	val = t_val;
	return true;
}
//解析所有行
void __parser(const std::vector<std::string> &line_vector, std::map<std::string, std::map<std::string, std::string>> &section_map)
{
	std::string cur_section_name; //当前章节名称
	FOREACH(line_vector, it)
	{
		const std::string &line = *it;
		const std::string section_name = __get_section_name(line);
		if (!section_name.empty())
		{ //章节
			cur_section_name = section_name;
		}
		else
		{ //名字和值, ip=xxxxxx;
			std::string name;
			std::string val;
			if (__get_name_and_val(line, name, val))
			{
				section_map[cur_section_name][name] = val;
			}
		}
	}
}
} // namespace

namespace xr
{
void file_ini_t::load(const char *file_path)
{
	std::ifstream fin(file_path);
	std::vector<std::string> line_vector;
	std::string s;
	while (getline(fin, s))
	{
		if (s.empty())
		{ //去除空行
			continue;
		}
		if ('#' == s[0])
		{ //去除注释
			continue;
		}
		line_vector.push_back(s);
	}
	__parser(line_vector, this->section_map);
	fin.close();
}

std::string file_ini_t::get_str(const char *section, const char *name)
{
	auto it = this->section_map.find(section);
	if (this->section_map.end() != it)
	{
		std::map<std::string, std::string> &r = it->second;
		auto it_name = r.find(name);
		if (r.end() != it_name)
		{
			return it_name->second;
		}
	}
	static std::string str;
	return str;
}
} //end namespace xr