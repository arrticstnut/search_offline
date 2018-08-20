INC_DIR:= \
	./include/ \
	./include/wordSegmentation/cppjieba/ \
	./include/wordSegmentation/ \
	./include/wordSegmentation/cppjieba/limonp/ \
	./include/tinyxml2/ \
	./include/configuration/ \
	./include/preProcess/ \
	./include/log \
	./include/redis \
#SRC_DIR:= 
#	./src/ \
#	./src/task/ \
#	./src/tcpSer/ \
#	./src/threadPool/ \
#	./src/cacle/ \
#	./src/timer/ \
#	./src/cppjieba/ \
#	./src/configuration/ \

SRCS:=\
	$(wildcard ./src/*.cc) \
	$(wildcard ./src/tinyxml2/*.cc) \
	$(wildcard ./src/configuration/*.cc) \
	$(wildcard ./src/wordSegmentation/*.cc) \
	$(wildcard ./src/preProcess/*.cc) \
	$(wildcard ./src/log/*.cc) \
	$(wildcard ./src/redis/*.cc) \

OBJS:= $(patsubst %.cc, %.o, $(SRCS))
LIBS:= -llog4cpp -lpthread -lmysqlclient -lhiredis -ljson

CXX:=g++

CXXFLAGS:= -Wall -std=c++11 $(addprefix -I, $(INC_DIR)) $(LIBS) -Wno-deprecated \
	-g

EXE:=./bin/ser

$(EXE):$(OBJS)
	$(CXX) -o $(EXE) $(OBJS) $(CXXFLAGS)

clean:
#	rm -rf $(EXE)
	rm -rf $(OBJS)

