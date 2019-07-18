//杂项

#pragma once

#include "xr_include.h"

//////////////////////////////////////////////////////////////////////////

#define	FAIL (-2)
#define	ERR  (-1)
#define	INVALID_FD (-1)
#define SUCC (0)

//判断	
#define	IS_NULL(__p__)	(NULL == (__p__))
#define	IS_ZERO(__p__)	(0 == (__p__))
#define IS_SUCC(__p__) (SUCC == (__p__))
#define IS_ERR(__p__) (ERR == (__p__))
#define IS_FAIL(__p__) (FAIL == (__p__))
//协助	
#define NOTE//需要注意	
#define TODO//需要完成	
#define DEBUG//为了调试	
#define OUT//输出参数	
#define IN//输入参数	
#define TEST//测试中

//////////////////////////////////////////////////////////////////////////
#ifndef likely
	#define likely(x)  __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
	#define unlikely(x)  __builtin_expect(!!(x), 0)
#endif

//////////////////////////////////////////////////////////////////////////
#define STRUCT_MEMBER_SIZEOF(__struct__, __member__) \
	sizeof(((__struct__*)0)->__member__)

//////////////////////////////////////////////////////////////////////////
//内存删除	
#define SAFE_DELETE(__p__){\
		delete (__p__);\
		(__p__) = NULL;\
	}

#define SAFE_DELETE_ARR(__p__){\
		delete [](__p__);\
		(__p__) = NULL;\
	}

#define SAFE_FREE(__p__){\
		free(__p__);\
		(__p__) = NULL;\
	}

//////////////////////////////////////////////////////////////////////////
//处理可被系统中断的函数返回EINTR时循环处理.(函数返回值必须为int)	
//example:int nRes = HANDLE_EINTR(::close(s));	
#define HANDLE_EINTR(x) ({\
	decltype(x) __eintr_code__;\
	do {\
		__eintr_code__ = x;\
	} while(unlikely(__eintr_code__ < 0 && EINTR == errno));\
	__eintr_code__;\
	})

//////////////////////////////////////////////////////////////////////////
#define FOREACH(__container__, it) \
	for(auto it = (__container__).begin(); (it) != (__container__).end(); ++(it))

#define FOREACH_P(__container__, it) \
	for(auto it = (*__container__).begin(); (it) != (*__container__).end(); ++(it))

#define FOREACH_R(__container__, it) \
	for(auto it = (__container__).rbegin(); (it) != (__container__).rend(); ++(it))

#define FOREACH_PB(__container__, index) \
	auto __cnt__ = __container__##_size(); \
	for(auto index = 0; index < __cnt__; index++)

//返回std::map中的val(指针类型),按照KEY来查找	
#define FIND_MAP_VAL_P(__map__, __key__) ({\
		auto __it__ = (__map__).find(__key__);\
		(__map__).end() != __it__ ? __it__->second : NULL;\
	})

//返回std::map中的val对象的指针,按照KEY来查找	
#define FIND_MAP_VAL(__map__, __key__) ({\
		auto __it__ = (__map__).find(__key__);\
		(__map__).end() != __it__ ? &__it__->second : NULL;\
	)}

//return 1  :f1 > f2	
//return 0  :f1 == f2	
//retrun -1 :f1 < f2		
#define FLOAT_COMPARE(__f1__, __f2__) ({\
	(__f1__) - (__f2__) < -FLT_EPSILON ? -1 : (FLT_EPSILON < (__f1__) - (__f2__) ? 1 : 0)\
})

namespace xr{
	//由字符串转换为所需类型	
	//value	所需类型	
	//s	待转换的字符串	
	// notice:s(中间有空格的话,它会用空格分割,只取空格前的部分)	
	//string2T("", x);	空字符串,转出来是0		
	//************************************
	template <typename T> 
	inline void string2T(const std::string& s, T &value) {
		if (s.empty()){
			std::string ts = "0";
			std::stringstream ss(ts);
			ss >> value;
		} else {
			std::stringstream ss(s);
			ss >> value;
		}
	}
	template <typename T> 
	std::string T2string(T value) {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	//切割字符(以单个符号为间断)like: 1,2,3 or 1;2;3 or aa/b/cc ...	
	//src	待切割的字符串
	//tag	切割依据符号
	//result 切割后的结果
	template <typename T>
	inline void cat_string(const std::string& src, char tag,  std::vector<T>& result){
		std::stringstream ss(src);        
		std::string sub_str;        
		while(std::getline(ss, sub_str, tag)){
			//以tag为间隔分割str的内容	
			T t;
			string2T(sub_str, t);
			result.push_back(t);
		}
	};
}

//linux version code 大于等于 内核版本号
#define XR_IS_LINUX_VERSION_CODE_GE(__code1__, __code2__, __code3__) \
	(LINUX_VERSION_CODE >= KERNEL_VERSION((__code1__), (__code2__), (__code3__)))
	