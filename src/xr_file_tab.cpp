#include "xr_file_tab.h"

namespace {
	//解析文件
int __parser( const std::vector<std::string>& line_vector, 
	std::map<std::string, uint32_t>& title_map,
	std::vector<std::vector<std::string>>& content_vector ){
	std::string cur_section_name;//当前章节名称
	uint32_t row = 0;//行
	FOREACH(line_vector, it){
		const std::string& line = *it;
		row++;
 		if (2 == row){
 			continue;
 		}
		std::vector<std::string> val_vector;
		xr::cat_string(line, '	', val_vector);
		if (1 == row){
			uint32_t col_cnt = 0;//列
			FOREACH(val_vector, it_title){
				title_map[*it_title] = col_cnt++;
			}
			continue;
		}
		if (title_map.size() != val_vector.size()){//值的列数和标题的列数不等
			return FAIL;
		}
		content_vector.push_back(val_vector);
	}
	return SUCC;
}
}
namespace xr{
int file_tab_t::load( const char* file_path )
{
	std::ifstream fin(file_path);
	if (!fin.good()){
		fin.close();
		return FAIL;
	}

	std::vector<std::string> line_vector;
	std::string line;

	while(std::getline(fin, line)){
		if (line.empty()){//去除空行
			continue;
		}
		if ('#' == line[0]){//去除注释
			continue;
		}
		line_vector.push_back(line);
	}
	if (FAIL == __parser(line_vector, this->title_map, this->content_vector)){
		fin.close();
		return FAIL;
	}
	fin.close();
	return SUCC;
}

std::vector<std::vector<std::string>> file_tab_t::get(const std::vector<std::string>& title_vec){
	std::vector<std::vector<std::string>> ret;
	FOREACH(this->content_vector, it){
		std::vector<std::string>& line = *it;
		std::vector<std::string> str_vac;
		FOREACH(title_vec, it_title){
			const std::string& col_name = *it_title;
			static std::string str_def;
			std::string val = this->get_val_def(col_name.c_str(), line, str_def);
			str_vac.push_back(val);
		}
		ret.push_back(str_vac);
	}
	return ret;
}
}//end namespace xr