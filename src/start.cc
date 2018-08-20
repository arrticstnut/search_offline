///
/// @file    start.cc
/// @author  
///

#include "configuration.h"
#include "cppLog.h"
#include "wordSegmentation.h"
#include "dirScanner.h"
#include "pageLib.h"
#include "pageLibPreProcessor.h"
#include <fstream>
#include <sstream>
#include <iostream>
using std::cout;
using std::endl;


void start(){
	cc::logInfo("[--offline server start--]");
	cc::Configuration & conf = cc::Configuration::getInstance();
	//cc::DirScanner dirScanner;
	//cc::PageLib pageLib(conf,dirScanner);
	//pageLib.create();
	//pageLib.store();
	cc::WordSegmentation & jieba = cc::WordSegmentation::getInstance();
	cc::PageLibPreProcessor preProcessor(conf,jieba);
	cc::logInfo("[--offline server stop--]");
}

int main(){
	start();
}
