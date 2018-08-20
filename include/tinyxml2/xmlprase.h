#ifndef __CC_XMLPRASE_H__
#define __CC_XMLPRASE_H__
#include<string.h>
#include<string>
#include<iostream>
#include<vector>
#include"tinyxml2.h" 
#include<stdio.h>
#include<sys/types.h>
#include<dirent.h>
#include<fstream>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace cc
{
	struct RssItem{
		string title;
		string link;
		string content;
	};

	class RssReader{
		public:
			RssReader();
			void parseRss(const string &);//解析Rss
			void dump(const string &);//输出
		private:
			vector<RssItem> _rss;
	};

	struct PageItem{
		int docid;
		string title;
		string link;
		string content;
	};

	class PageDataReader{
		public:
			PageDataReader();
			void parsePageData(const string & fileName);//解析从网页爬取的data
			void dump(const string & fileName);//输出
			vector<PageItem> getPageItems();
		private:
			vector<PageItem> _vecPageItem;
	};

};//end of namespace

#endif

