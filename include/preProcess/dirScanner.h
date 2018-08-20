///
/// @file    dirScanner.h
/// @author  
///

#ifndef __CC_DIRSCANNER_H__
#define __CC_DIRSCANNER_H__
#include "configuration.h"
#include <iostream>
#include <vector>
#include <string.h>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>

using std::cout;
using std::endl;
using std::vector;
using std::string;

namespace cc
{
	class DirScanner
	{
		public:
			DirScanner();
			vector<string> files();//返回_vecFileNames的引用
			vector<string> operator()(const string &);//传入路径，得到路径下的文件名
		private:
			void traverse(const string &);//获取某一目录下的所有文件
		private:
			vector<string> _vecFileNames;//vector中存放每个源网页文件的绝对路径
	};
};
#endif
