///
/// @file    pageLibPreProcessor.cc
/// @author  
///

#include "webPage.h"
#include "pageLibPreProcessor.h"
#include "cppLog.h"
#include "redisPool.h"
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <string>
#include <mysql/mysql.h>
#include <sstream>

using std::cout;
using std::endl;
using std::ifstream;
using std::regex_search;
using std::log;
using std::sqrt;
using std::string;

namespace cc
{
	PageLibPreProcessor::PageLibPreProcessor(Configuration & conf,WordSegmentation & jieba)
		:_conf(conf)
		 ,_jieba(jieba)
	{
		readInfoFromDatabase();
		//readInfoFromFile();
		cutRedundantPages();
		buildInvertIndexTable();
		storeOnRedis();
	}

#if 0
	void PageLibPreProcessor::readInfoFromFile(){//根据配置信息读取网页库和网页偏移库的内容
		//通过配置信息的到网页库的路径
		auto confMap = _conf.getConfMap();
		auto itlib = confMap.find("PageLibDir");
		if(itlib == confMap.end()){
			logErrorLoc("configuration error");
		}
		string pageLibDir = itlib->second;
		//读取网页库
		ifstream ifsPageLib(pageLibDir + "pageLib.dat");
		ifstream ifsOffset(pageLibDir + "offsetLib.dat");
		if(!ifsPageLib.good() || !ifsOffset.good()){
			logErrorLoc("ifstream error");
		}
		string line;
		long docId,startPos,len;
		while(std::getline(ifsOffset,line)){
			//将line转化成istringstream来读取配置信息
			std::istringstream iss(line);
			iss >> docId >> startPos >> len;
			//从ifs读取给定长度的字节
			ifsPageLib.seekg(startPos);//定位到指定的开头
			string doc(len,'0');//构造len长的string
			ifsPageLib.read(&doc[0],len);//读len长字符到string种
			WebPage wp;
			wp.processDoc(doc,_conf,_jieba);
			_pageLib.push_back(wp);
		}
	}
#endif

	void PageLibPreProcessor::readInfoFromDatabase(){//从数据库中读取文件
		auto  confMap = Configuration::getInstance().getConfMap();
		string ip = confMap["MysqlServerIp"];
		string user = confMap["MysqlServerUser"];
		string password = confMap["MysqlServerPassword"];
		string database = confMap["MysqlSeverDatabase"];
		string table = confMap["WebPageTable"];
		if(ip.empty() || user.empty() || password.empty() || database.empty() || table.empty()){
			logErrorLoc("database argu error");
			exit(1);
		}
		MYSQL *conn;
		MYSQL_RES *res;
		MYSQL_ROW row;
		conn = mysql_init(NULL);

		//设置utf8格式
		if(mysql_set_character_set( conn, "utf8" )){
			std::ostringstream oss;
			oss << "Error making set character: " << mysql_error(conn);
			logErrorLoc(oss.str());
        } 

		if(!mysql_real_connect(conn,ip.c_str(),user.c_str(),password.c_str(),database.c_str(),0,NULL,0)) {
			std::ostringstream oss;
			oss << "Error connecting to database: " << mysql_error(conn);
			logErrorLoc(oss.str());
			exit(1);
		}else{
			//printf("Connected...\n");
		}
		string query = string("select * from ") + table;
		int t = mysql_query(conn,query.c_str());
		if(t) {
			std::ostringstream oss;
			oss << "Error making query: " << mysql_error(conn);
			logErrorLoc(oss.str());
			exit(1);
		}else{
			//printf("Query made...\n");
			res = mysql_use_result(conn);
			if(res) {
				while((row=mysql_fetch_row(res))!=NULL) {
					//printf("num=%d\n",mysql_num_fields(res));//列数
					int docId = stoi(row[0]);
					string title = row[1];
					string url = row[2];
					string content = row[3];
					WebPage wp;
					wp.processWebData(docId,title,url,content);
					_pageLib.push_back(wp);
				}
			}
			mysql_free_result(res);
		}
		mysql_close(conn);
	}

	void PageLibPreProcessor::cutRedundantPages(){//对冗余的网页进行去重
		vector<WebPage>::iterator it1 = _pageLib.begin();
		for(;it1 != _pageLib.end();){
			for(auto it2 = std::next(it1);it2 != _pageLib.end();){
				if(*it1 == *it2){
					it2 = _pageLib.erase(it2);
				}else{
					++it2;
				}
			}
			++it1;
		}
		std::sort(_pageLib.begin(),_pageLib.end());//按照docid排序
	}

	void PageLibPreProcessor::buildInvertIndexTable(){//创建倒排索引表 
		int n = _pageLib.size();//文档总数
		//以下容器参数说明:unordered_map<word,vector<pair<docid,tf> > >;
		unordered_map<string,vector<pair<int,int> > > dfMap;//用于计算df，map的key为单词
		//以下容器参数说明:unordered_map<docid,vector<pair<word,tf_idf> > >;
		unordered_map<int,vector<pair<string,double> > > normalMap;//用于每篇文档的计算每篇文档的tfidf并归一化,map的key为docid
		//统计df
		for(auto & web:_pageLib){
			map<string,int> & wordMap = web.getWordMap();
			int docId = web.getDocId();
			for(auto & p:wordMap){//p的格式pair<单词，词频>
				std::pair<int,int> dfPair = std::make_pair(docId,p.second);//dicId,词频
				//dfMap的key是单词，value是存放pair对动态数组，pair的first是文档ID，第二项是词频
				string word = p.first;
				if(dfMap.count(word) == 0){//第一次
					dfMap.insert(std::make_pair(word,vector<pair<int,int> >(1,dfPair)));
				}else{
					dfMap[word].push_back(dfPair);
				}
			}
		}

		//计算tfidf
		for(auto & p:dfMap){//dfMap的格式unordered_map<word,vector<pair<docid,tfidf> > >;
			string word = p.first;
			int df = p.second.size();//文档频率df
			for(auto & f:p.second){//f的格式:pair<docid,tfidf>
				int docId = f.first;
				double tf = f.second;
				double tf_idf = (tf * 1.0) * log((n * 1.0)/(df *1.0 + 1.0));
				vector<pair<string,double> > vecWfPair(1,make_pair(word,tf_idf));//单词，tfidf值
				if(normalMap.count(docId) == 0){//normalMap的格式:unordered_map<docid,vector<pair<word,tf_idf> > >;
					normalMap.insert(make_pair(docId,vecWfPair));
				}else{
					normalMap[docId].push_back(make_pair(word,tf_idf));
				}
			}
		}

		//归一化
		for(auto & p:normalMap){//p的格式：pair<docId,vector<word,tfidf>>
			double sum = 0;
			for(auto & wt:p.second){
				sum += (wt.second)*(wt.second);
			}
			sum = sqrt(sum);
			for(auto & wt:p.second){
				wt.second = wt.second /sum;
			}
		}

		//写入倒排索引表
		for(auto & p:normalMap){//p的格式：pair<docId,vector<word,tfidf>>
			int docId = p.first;
			for(auto & wt:p.second){
				string word = wt.first;
				double tfidf = wt.second;
				pair<int,double> dfPair = make_pair(docId,tfidf);
				if(_invertIndexTable.count(word) == 0){//_invertIndexTable的格式：unordered_map<word,vector<pair<docid,tfidf> > >

					_invertIndexTable.insert(make_pair(word,vector<pair<int,double> >(1,dfPair)));
				}else{
					_invertIndexTable[word].push_back(dfPair);
				}
			}
		}
	}

	void PageLibPreProcessor::storeOnDisk(){
		//将倒排索引信息写入文件
		//找到文件路径
		auto confMap = _conf.getConfMap();
		auto it  = confMap.find("PageLibDir");
		if(it == confMap.end()){
			logErrorLoc("find pageLibDir error");
			return ;
		}
		string pageLibDir = it->second;
		//打开文件
		std::ofstream ofs(pageLibDir + "invertIndex.dat");
		if(!ofs.good()){
			logErrorLoc("open File error");
			return ;
		}
		//写入文件
		for(auto & p:_invertIndexTable){
			ofs << p.first << "\t";
			for(auto & dt:p.second){
				ofs <<  dt.first << " " <<  dt.second << "\t";
			}
			ofs << endl;
		}
		ofs.close();
	}

	void PageLibPreProcessor::storeOnRedis(){
		//获得redis连接的各个参数
		auto confMap = Configuration::getInstance().getConfMap();
		string serverIp = confMap["RedisServerIp"];
		string serverPort = confMap["RedisServerPort"];
		string invertIndexKey = confMap["RedisInvertIndexKey"];
		if(serverIp.empty() || serverPort.empty() || invertIndexKey.empty()){
			logErrorLoc("get conf of redis error");
		}
		int port = stoi(serverPort);
		//实例化redis
		RedisPool redis(serverIp,port);
		
#if 0
		for(auto & p:_invertIndexTable){
			string word = p.first;
			Json::Value value;
			Json::Value table;
			for(auto & dt:p.second){
				value["docid"] = dt.first;
				value["tfidf"] = dt.second;
				table.append(value);
			}
			Json::StyledWriter writer;
			cout << word << ":" << writer.write(table) << endl;
		}
#endif
		//hash表名
		string invertKey = confMap["RedisInvertIndexKey"];
		if(invertKey.empty()){
			logErrorLoc("get conf of redisInvertKey error");
			return;
		}
		//先将旧的倒排索引表删除
		string result;//执行redis命令的返回信息
		redis.ExecuteCmdBy_2_InputArgs("DEL",invertKey,result);
		
		//将倒排索引信息写入redis的hash表
		for(auto & p:_invertIndexTable){
			string word = p.first;
			Json::Value value;
			Json::Value table;
			for(auto & dt:p.second){
				value[to_string(dt.first)] = dt.second;
				table[word] = value;
			}

			//写入redis
			Json::StyledWriter writer;
			string redisField = word;//field为单词
			string redisValue = writer.write(value);//将value转为json字符串:  docid：tfidf 的keyValue对
			redis.ExecuteCmdBy_4_InputArgs("HSET",invertKey,redisField,redisValue,result);

#if 0	
			//测试解析字符串
			string invertStr = writer.write(value);
			Json::Value root;
			Json::Reader reader;
			if(!reader.parse(invertStr,root)){
				logErrorLoc("parse string to json error");
				return;
			}


			Json::Value::Members members;
			members = root.getMemberNames();
			for(auto it = members.begin();it != members.end();++it){
				string docIdStr = *it;
				double tfidf = root[docIdStr].asDouble();
				cout << docIdStr << "|" << tfidf << "\t";
			}
			cout << endl;
#endif
		}

	}

};//end of namespace
