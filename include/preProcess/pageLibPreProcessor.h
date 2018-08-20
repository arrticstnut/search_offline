///
/// @file    pageLibPreProcessor.h
/// @author  
///
#ifndef __CC_PAGELIBPREPROCESSOR_H__
#define __CC_PAGELIBPREPROCESSOR_H__
#include "configuration.h"
#include "wordSegmentation.h"
#include "webPage.h"
#include "redisPool.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::unordered_map;
using std::map;
using std::set;
namespace cc
{
	class PageLibPreProcessor
	{
		public:
			PageLibPreProcessor(Configuration & conf,WordSegmentation & jieba);
			void doPreProcess();//执行预处理
			void readInfoFromFile();//根据配置信息读取网页库和网页偏移库
			void readInfoFromDatabase();//从数据库中读取文件
			void cutRedundantPages();//对冗余的网页进行去重
			void buildInvertIndexTable();//创建倒排索引表
			void storeOnDisk();//将经过预处理后的网页库、位置偏移库和倒排索引表写回到磁盘上
			void storeOnRedis();//将经过预处理后的倒排索引信息存放redis数据库
		private:
			Configuration & _conf;
			WordSegmentation & _jieba;//分词对象
			vector<WebPage> _pageLib;//网页库的容器对象
			unordered_map<int,pair<int,int> > _offsetLib;//网页偏移库对象
			unordered_map<string,vector<pair<int,double> > > _invertIndexTable;//倒排索引表对象
	};

};//end of namespace

#endif
