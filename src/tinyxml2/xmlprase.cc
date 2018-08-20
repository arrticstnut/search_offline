///
/// @file    xmlparse.cc
/// @author
///
#include "tinyxml2.h" 
#include "xmlprase.h"
#include "cppLog.h"
#include <sstream>
#include <cstdlib>
#include  <regex>

using namespace tinyxml2;
using std::ostringstream;
namespace cc
{
	RssReader::RssReader() {
		_rss.reserve(128);
	}

	void RssReader::parseRss(const string & filename){//解析rss
		XMLDocument doc;
		if(doc.LoadFile(filename.c_str())!=tinyxml2::XMLError::XML_SUCCESS){
			ostringstream oss;
			oss <<"[LoadFile]:[error:id="<<doc.ErrorID()<<"]:["<<filename<<"]"<<endl;
			string logMessage = oss.str();
			logErrorLoc(logMessage.c_str());
			return;
		}else{
			//cout<<"[LoadFile]:[success]:"<<"["<<filename<<"]"<<endl;
		}
		XMLElement *root=doc.RootElement();
		string rootname=root->Name();
		string itemname="item";
		string titlename="title";
		string linkname="link";
		string contentname="description";
		XMLElement *item;
		if(rootname=="feed"){//处理根为feed的xml
			itemname="entry";
			contentname="content";
			item=root->FirstChildElement(itemname.c_str());
		}else{
			XMLElement *channel=root->FirstChildElement();
			item=channel->FirstChildElement(itemname.c_str());
		}
		int itemcnt=0;//记录是否处理了数据
		std::regex reg("<{1}[^<>]*>{1}");
		while(item){
			struct RssItem rssitem;
			XMLElement *title=item->FirstChildElement(titlename.c_str());
			if(title->GetText()==NULL){
				item=item->NextSiblingElement(itemname.c_str());
				continue;
			}
			rssitem.title=title->GetText();
			rssitem.title=regex_replace(rssitem.title,reg,"");
			XMLElement *link=item->FirstChildElement(linkname.c_str());
			if(link->GetText()==NULL){//处理根为feed类型xml的link
				const XMLAttribute *linkAttr=link->FirstAttribute();
				rssitem.link=linkAttr->Value();
				rssitem.link=regex_replace(rssitem.link,reg,"");
			}else{
				rssitem.link=link->GetText();
				rssitem.link=regex_replace(rssitem.link,reg,"");
			}
			XMLElement *content=item->FirstChildElement(contentname.c_str());
			rssitem.content=content->GetText();
			rssitem.content=regex_replace(rssitem.content,reg,"");
			_rss.push_back(rssitem);
			item=item->NextSiblingElement(itemname.c_str());
			++itemcnt;
		}
	}


	void RssReader::dump(const string & resultfile){
		_rss.shrink_to_fit();
		std::ofstream ofs(resultfile,std::ios::app);
		if(!ofs.good()){
			logErrorLoc("ofstream open error");
			return;
		}
		int idx=0;
		int size=_rss.size();
		for(idx=0;idx < size;++idx){
			ofs << "<doc>" <<endl;
			ofs << '\t'<< "<docid>" << idx <<"</docid>"<< endl;
			ofs << '\t'<< "<title>" << _rss[idx].title <<"</title>"<< endl;
			ofs << '\t'<< "<link>" << _rss[idx].link <<"</link>"<< endl;
			ofs << '\t'<< "<content>" << _rss[idx].content << "</content>" << endl;
			ofs << "</doc>" <<endl;
		}
		ofs.close();
	}

	PageDataReader::PageDataReader()
	{
		_vecPageItem.reserve(128);
	}


	void PageDataReader::parsePageData(const string & filename){//解析从网页库
		XMLDocument docs;//docs就是整个网页库
		if(docs.LoadFile(filename.c_str())!=tinyxml2::XMLError::XML_SUCCESS){
			ostringstream oss;
			oss <<"open file \""<< filename << " \"error,erron = "<<docs.ErrorID()<<endl;
			string logMessage = oss.str();
			logErrorLoc(logMessage.c_str());
			return;
		}else{
			//cout<<"[LoadFile]:[success]:"<<"["<<filename<<"]"<<endl;
		}
		XMLElement *doc = docs.RootElement();//得到第1个doc
		XMLElement *docid,*title,*link,*content;
		//XMLElement *description;
		PageItem pageItem;
		while(doc){
			docid = doc->FirstChildElement("docid");
			title = doc->FirstChildElement("title");
			link = doc->FirstChildElement("link");
			content = doc->FirstChildElement("content");
			//description = doc->FirstChildElement("description");
			if(!title || !link || !content){
				logErrorLoc("pagelib attribute name error");
			}
			const char *pDocIdText = docid->GetText();
			const char *pTitleText = title->GetText();
			const char *pLinkText = link->GetText();
			const char *pContentText = content->GetText();
			//要判断指针是否为空，否则赋值给string会出错
			pageItem.docid = pTitleText ? std::atoi(pDocIdText) : 0;
			pageItem.title = pTitleText ? pTitleText : "";
			pageItem.link =  pLinkText ? pLinkText : "";
			pageItem.content = pContentText ? pContentText : "";
			_vecPageItem.push_back(pageItem);
			doc = doc->NextSiblingElement();//继续下一个doc
		}
	}


	vector<PageItem> PageDataReader::getPageItems(){
		return _vecPageItem;
	}

	void PageDataReader::dump(const string & outfileName){
		_vecPageItem.shrink_to_fit();
		std::ofstream ofs(outfileName,std::ios::app);
		if(!ofs.good()){
			logErrorLoc("ofstream open error");
			return;
		}
		int idx=0;
		int size=_vecPageItem.size();
		for(idx=0;idx < size;++idx){
			ofs << "<doc>" << endl;
			ofs << '\t' << "<docid>" << idx << "</docid>" << endl;
			ofs << '\t' << "<title>" << _vecPageItem[idx].title << "</title>" << endl;
			ofs << '\t' << "<link>" << _vecPageItem[idx].link << "</link>" << endl;
			ofs << '\t' << "<content>" << _vecPageItem[idx].content << "</content>" << endl;
			ofs << "</doc>" <<endl;
		}
		ofs.close();
	}

};//end of namespace



