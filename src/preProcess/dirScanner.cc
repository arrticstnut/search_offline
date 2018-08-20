///
/// @file    dirScanner.cc
/// @author  
///

#include "dirScanner.h"
#include "cppLog.h"
#include <iostream>
using std::cout;
using std::endl;
namespace cc
{
	DirScanner::DirScanner(){}

	vector<string> DirScanner::files(){//返回扫描到的文件名
		return _vecFileNames;
	}

	void DirScanner::traverse(const string & dirname){//扫描路径
		struct dirent *pDirInfo; 
		struct stat statbuf;//文件状态
		DIR *pDir=opendir(dirname.c_str());
		if(NULL==pDir){
			logErrorLoc("opendir error");
			return;
		}
		while((pDirInfo=readdir(pDir))!=NULL){
			if(!strcmp(pDirInfo->d_name,".")||!strcmp(pDirInfo->d_name,"..")){
				continue;
			}
			string filename=dirname+"/"+pDirInfo->d_name;
			stat(filename.c_str(),&statbuf);//获取文件状态,一定要加上dirname,否则程序所在的目录和dirname不相同时，获取不到
			if(S_ISDIR(statbuf.st_mode)){//跳过目录
				continue;
			}
			_vecFileNames.push_back(filename);
		}
		closedir(pDir);
	}

	vector<string> DirScanner::operator()(const string & dirname){//传入路径，得到路径下的文件名
		traverse(dirname);
		return _vecFileNames;
	}
};
