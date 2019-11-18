//parsing xml files
//-I/usr/include/libxml2 -lxml2

#pragma once

#include <libxml/tree.h>

#include "xr_log.h"
#include "xr_util.h"

namespace xr
{
struct xml_t
{
public:
	xmlDocPtr doc_ptr;   //定义解析文档指针
	xmlNodePtr node_ptr; //定义结点指针(你需要它为了在各个结点间移动)

	typedef std::ios_base &(&manip_t)(std::ios_base &);

public:
	xml_t();
	virtual ~xml_t();
	int open(const char *name);
	void move2children_node();
	void move2next_node();
	int strcmp(const xmlNodePtr node_ptr, const char *name);
	//把某个xml属性的值读取出来
	//val xml的属性值将被读取到val中
	//prop xml属性名
	//manip 以什么形式解释xml属性值，默认是dec十进制，还可用oct八进制和hex十六进制
	//注意！不要用这个函数来读取xml属性值到字符数组中，否则有越界的危险！如果需要读取字符串，\n
	//      可以用这个函数把字符串读到string对象里，或者用get_xml_prop_raw_str把字符串读到字符数组里。
	template <typename T>
	static void get_prop(xmlNodePtr cur, const char *prop, T &val, manip_t manip = std::dec)
	{
		xmlChar *str;
		if (NULL == cur || NULL == (str = xmlGetProp(cur, reinterpret_cast<const xmlChar *>(prop))))
		{
			ALERT_LOG("[%s]", prop);
			assert(0);
		}
		else
		{
			std::istringstream iss(reinterpret_cast<const char *>(str));
			if (!iss.good())
			{
				ALERT_LOG("[%s]", prop);
				assert(0);
			}
			iss >> manip >> val;
			xmlFree(str);
		}
	}
	static uint32_t get_prop(xmlNodePtr cur, const char *prop, manip_t manip = std::dec);

	//同get_prop，但如果prop属性不存在的话，会把默认值def赋给val
	template <typename T1>
	static void get_prop_def(xmlNodePtr cur, const char *prop, T1 &val, const T1 &def, manip_t manip = std::dec)
	{
		if (NULL == cur)
		{
			val = def;
			return;
		}

		xmlChar *str = xmlGetProp(cur, reinterpret_cast<const xmlChar *>(prop));
		if (NULL == str)
		{
			val = def;
			return;
		}

		const char *cc = reinterpret_cast<const char *>(str);
		if (0 == ::strcmp(cc, ""))
		{
			val = def;
		}
		else
		{
			std::istringstream iss(cc);
			if (!iss.good())
			{
				ALERT_LOG("[%s]", prop);
				assert(0);
			}
			iss >> manip >> val;
			xmlFree(str);
		}
	}

	static uint32_t get_prop_def(xmlNodePtr cur, const char *prop, const uint32_t def, manip_t manip = std::dec);

	//把某个xml属性的值读取到字符数组中
	//val xml的属性值将被读取到val中
	//prop xml属性名
	template <size_t len>
	static void get_prop_raw_str(xmlNodePtr cur, const char *prop, char (&val)[len])
	{
		xmlChar *str;
		if (NULL == cur || NULL == (str = xmlGetProp(cur, reinterpret_cast<const xmlChar *>(prop))))
		{
			ALERT_LOG("[%s]", prop);
			assert(0);
		}
		else
		{
			::strncpy(val, reinterpret_cast<char *>(str), len - 1);
			val[len - 1] = '\0';
			xmlFree(str);
		}
	}

	//同get_prop_raw_str，但如果prop属性不存在的话，会把默认值def赋给val
	template <size_t len>
	static void get_prop_raw_str_def(xmlNodePtr cur, const char *prop, char (&val)[len], const char *def)
	{
		xmlChar *str;
		if (NULL == cur || NULL == (str = xmlGetProp(cur, reinterpret_cast<const xmlChar *>(prop))))
		{
			::strncpy(val, def, len - 1);
			val[len - 1] = '\0';
		}
		else
		{
			::strncpy(val, reinterpret_cast<char *>(str), len - 1);
			val[len - 1] = '\0';
			xmlFree(str);
		}
	}
	//得到一个节点的内容
	template <typename T>
	static void get_content(xmlNodePtr cur, T &val)
	{
		xmlChar *str;
		if (NULL == cur || NULL == (str = xmlNodeGetContent(cur)))
		{
			ALERT_LOG("");
			assert(0);
		}
		else
		{
			std::istringstream iss(reinterpret_cast<const char *>(str));
			if (!iss.good())
			{
				ALERT_LOG("");
				assert(0);
			}
			iss >> val;
			xmlFree(str);
		}
	}

	//把某个xml属性的值读取到数组中
	//arr xml的属性值将被读取到arr中
	//prop xml属性名
	//manip 以什么形式解释xml属性值，默认是dec十进制，还可用oct八进制和hex十六进制
	//return 读取到数组arr中的值的个数
	template <typename T1, size_t len>
	static size_t get_xml_prop_arr(xmlNodePtr cur, T1 (&arr)[len], const void *prop, manip_t manip = std::dec)
	{
		xmlChar *str;
		if (NULL == cur || NULL == (str = xmlGetProp(cur, reinterpret_cast<const xmlChar *>(prop))))
		{
			ALERT_LOG("[%s]", (char *)prop);
			assert(0);
		}
		else
		{
			size_t i = 0;
			std::istringstream iss(reinterpret_cast<const char *>(str));
			if (!iss.good())
			{
				ALERT_LOG("[%s]", (char *)prop);
				assert(0);
			}
			while ((i != len) && (iss >> manip >> arr[i]))
			{
				++i;
			}

			xmlFree(str);
			return i;
		}
	}

	//同get_xml_prop_arr，但如果prop属性不存在的话，会把默认值def赋给arr
	//return 读取到数组arr中的值的个数，如果prop不存在，则返回0
	template <typename T1, typename T2, size_t len>
	static size_t get_xml_prop_arr_def(xmlNodePtr cur, T1 (&arr)[len], const void *prop, const T2 &def, manip_t manip = std::dec)
	{
		xmlChar *str;
		if (NULL == cur || NULL == (str = xmlGetProp(cur, reinterpret_cast<const xmlChar *>(prop))))
		{
			for (size_t i = 0; i != len; ++i)
			{
				arr[i] = def;
			}
			return 0;
		}
		else
		{
			size_t i = 0;
			std::istringstream iss(reinterpret_cast<const char *>(str));
			if (!iss.good())
			{
				ALERT_LOG("[%s]", (char *)prop);
				assert(0);
			}
			while ((i != len) && (iss >> manip >> arr[i]))
			{
				++i;
			}

			xmlFree(str);
			return i;
		}
	}

private:
	void close();
};
} // namespace xr
