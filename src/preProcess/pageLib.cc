///
/// @file    pageLib.cc
/// @author  
///

#include "pageLib.h"
#include "xmlprase.h"
#include "cppLog.h"
#include <sstream>
#include <fstream>
#include <iostream>
using std::cout;
using std::endl;

namespace cc
{
	PageLib::PageLib(Configuration & conf,DirScanner & dirScanner)
		:_conf(conf)
		 ,_dirScanner(dirScanner)
	{}
	void PageLib::create(){//创建网页库(从爬取的网页数据文件中)
		cc::PageDataReader reader;//读取从网页爬取的数据
		vector<cc::PageItem> vecPageItem;
		//从配置文件中得到pageData的路径
		const map<string,string> confMap = _conf.getConfMap();
		auto itdata = confMap.find("PageDataDir");
		if(itdata == confMap.end()){
			logErrorLoc("configuration error");
			return;
		}
		string pageDataDir = itdata->second;//原始的pagedata路径
		//扫描路径，得到路径下的文件
		vector<string> files = _dirScanner(pageDataDir);
		//解析文件
		for(const auto & file:files){
			reader.parsePageData(file);//读取数据
		}
		vecPageItem = reader.getPageItems();
		//将网页存入到到网页库容器中
		for(size_t idx = 0;idx < vecPageItem.size();++idx){
			std::ostringstream oss;
			oss<<"<doc>"<<endl;
			oss<<'\t'<<"<docid>"<<idx<<"</docid>"<<endl;
			oss<<'\t'<<"<title>"<<vecPageItem[idx].title<<"</title>"<<endl;
			oss<<'\t'<<"<link>"<<vecPageItem[idx].link<<"</link>"<<endl;
			oss<<'\t'<<"<content>"<<vecPageItem[idx].content<<"</content>"<<endl;
			oss<<"</doc>"<<endl;
			_vecPages.push_back(oss.str());
		}
	}

	void PageLib::store(){//存储网页库和偏移库
		//从配置文件中得到pageLib的路径
		const map<string,string> confMap = _conf.getConfMap();
		auto itlib = confMap.find("PageLibDir");
		if(itlib == confMap.end()){
			logErrorLoc("configuration error");
			return;
		}
		string pageLibDir = itlib->second;//pagelib路径
		//得到输出文件的ofs
		std::ofstream ofsPageLib(pageLibDir + "pageLib.dat");
		std::ofstream ofsOffset(pageLibDir + "offsetLib.dat");
		if(!ofsPageLib.good() || !ofsOffset.good())
		{
			logErrorLoc("ofstream open error!");
			return ;
		}
		//将文档输入到网页库中，并记录偏移信息
		for(size_t idx = 0;idx < _vecPages.size();++idx){
			auto startPos = ofsPageLib.tellp();
			ofsPageLib << _vecPages[idx];
			auto endPos = ofsPageLib.tellp();
			_offsetLib[idx] = std::make_pair(startPos,endPos-startPos);
		}
		ofsPageLib.close();
		//将偏移信息写入偏移库
		for(size_t idx = 0;idx < _offsetLib.size();++idx){
			ofsOffset << idx << "\t" << _offsetLib[idx].first << "\t" << _offsetLib[idx].second << endl;
		}
		ofsOffset.close();
	}
};//end of namespace
