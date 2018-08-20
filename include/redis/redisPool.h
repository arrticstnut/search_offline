///
/// @file    redisPool.h
/// @author  
///
#ifndef __CC_REDISPOOL_H__
#define __CC_REDISPOOL_H__

#include <string>
#include <mutex>
#include <hiredis/hiredis.h>
#include <queue>
#include <memory>
using std::stringstream;
using std::queue;
using std::string;

namespace cc
{

	class RedisPool
	{
		public:
			RedisPool(const string & serverIp,int serverPort,int timeout = 50000);//连接超时默认为5000毫秒
			~RedisPool();
		public:
			void disConnect();//释放所有连接
			bool ExecuteCmd(const string & cmd,string &response);//传入完整命令，结果存储在response中,（不支持返回结果为数组类型的命令）
			bool ExecuteCmdBy_2_InputArgs(const string & arg1,const string & arg2,string & response);//传入的是2个字段+1个返回变量
			bool ExecuteCmdBy_3_InputArgs(const string & arg1,const string & arg2,const string & arg3,string & response);//传入3个字段+1个返回变量
			bool ExecuteCmdBy_4_InputArgs(const string & arg1,const string & arg2,const string & arg3,const string & arg4,string & response);//传入4个字段+1个返回变量
		private:
			void releaseContext(redisContext *ctx,bool active);//释放连接（如果连接可用，则归还到连接池,如果连接不可用，则是真正释放连接)
			redisContext* createContext();//创建连接
		private:
			redisReply* ExecuteCmd(const string & cmd);
			redisReply* ExecuteCmdByInputArgs(const string & arg1,const string & arg2);//执行2个字段
			redisReply* ExecuteCmdByInputArgs(const string & arg1,const string & arg2,const string & arg3);//执行3个字段
			redisReply* ExecuteCmdByInputArgs(const string & arg1,const string & arg2,const string & arg3,const string & arg4);//执行4个字段
			bool getResponse(redisReply *reply,string & response);//根据reply分析结果
		private:
			string _serverIp;
			int _serverPort;
			int _timeout;//连接超时时间
			std::mutex _mutex;//锁
			queue<redisContext *> _contextQue;//连接池
	};
}//end of namespace

#endif
