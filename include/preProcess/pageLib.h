///
/// @file    pageLib.h
/// @author  
///
#ifndef __CC_PAGELIB_H__
#define __CC_PAGELIB_H__

#include "configuration.h"
#include "dirScanner.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>
using std::cout;
using std::endl;
using std::map;
using std::vector;
using std::string;

namespace cc
{
	class PageLib{
	public:
		PageLib(Configuration & conf,DirScanner & dirScanner);
		void create();//创建网页库
		void store();//存储网页库和位置偏移库
	private:
		Configuration & _conf;
		DirScanner & _dirScanner;//目录扫描对象的引用
		vector<string> _vecPages;//存放格式化之后的网页的容器
		map<int, std::pair<int, int> > _offsetLib;//存放每篇文档在网页库的位置信息
	};
};

#endif
