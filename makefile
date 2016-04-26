SRCDIR ?= $(CURDIR)
HEADERDIR = ./include
LIBDIR = ./lib64
APPDIR = $(HOME)/bin
VPATH = ./src/XTransServer

CC = g++
CPPFLAGS += -g -fPIC

INCFLAGS += -I/usr/include -I/usr/include/libxml2 
INCFLAGS += -I$(HEADERDIR) -I$(HEADERDIR)/XTransServer -I$(HEADERDIR)/libuv -I$(HEADERDIR)/log4cpp 
LDFLAGS += -L/usr/lib64 -lxml2 -lssl -lrt -lm -lc -ldl -lpthread

#主程序文件
XTRANSSERVER_XSD = config.xsd
XTRANSSERVER_CFG = XTransServer.config.xml
XTRANSSERVER_OBJ += XXmlDocument.o XClassFactory.o XPublic.o XLogClass.o XTools.o XConfig.o XServer.o 
XTRANSSERVER_OBJ += XSocket.o XListener.o XTcpClient.o XTcpConnection.o XTcpListener.o 
XTRANSSERVER_OBJ += XHttpContext.o XHttpConnection.o XHttpListener.o XHttpsConnection.o XHttpsListener.o 
XTRANSSERVER_OBJ += XDataBase.o XDataTable.o XDataBaseConnection.o 
XTRANSSERVER_OBJ += XTransServer.o 
XTRANSSERVER_LDFLAGS += 

DATABASE_ORACLE_OBJ += XOracleConnection.o
DATABASE_ORACLE_INCFLAGS += -I$(ORACLE_HOME)/sdk/include
#-I$(ORACLE_HOME)/sdk/include
DATABASE_ORACLE_LDFLAGS += -L$(ORACLE_HOME)/lib64 -lclntsh
#-L$(ORACLE_HOME)/lib64 -lclntsh

DATABASE_MYSQL_OBJ += XMySqlConnection.o
DATABASE_MYSQL_INCFLAGS += -I/usr/include/odbc
DATABASE_MYSQL_LDFLAGS += -lmyodbc5

.PHONY : all clean
all: libXTransServer libXTransServerOracle libXTransServerMySql libXTransServerAll clean

$(XTRANSSERVER_OBJ): %.o: %.cpp
	$(CC) -c $< $(CPPFLAGS) $(INCFLAGS) -o $@

$(DATABASE_ORACLE_OBJ): %.o: %.cpp
	$(CC) -c $< $(INCFLAGS) $(CPPFLAGS) $(DATABASE_ORACLE_INCFLAGS) -o $@
	
$(DATABASE_MYSQL_OBJ): %.o: %.cpp
	$(CC) -c $< $(INCFLAGS) $(CPPFLAGS) $(DATABASE_MYSQL_INCFLAGS) -o $@
	
libXTransServer : $(XTRANSSERVER_OBJ)
	$(CC) -o $(LIBDIR)/$@.so $(XTRANSSERVER_OBJ) $(LIBDIR)/libuv.a $(LIBDIR)/liblog4cpp.a $(CPPFLAGS) $(LDFLAGS) $(XTRANSSERVER_LDFLAGS) -shared -fPIC

libXTransServerOracle : $(XTRANSSERVER_OBJ) $(DATABASE_ORACLE_OBJ) 
	$(CC) -o $(LIBDIR)/$@.so $(XTRANSSERVER_OBJ) $(DATABASE_ORACLE_OBJ) $(LIBDIR)/libuv.a $(LIBDIR)/liblog4cpp.a $(CPPFLAGS) $(LDFLAGS) $(XTRANSSERVER_LDFLAGS) $(DATABASE_ORACLE_LDFLAGS) -shared -fPIC

libXTransServerMySql : $(XTRANSSERVER_OBJ) $(DATABASE_MYSQL_OBJ) 
	$(CC) -o $(LIBDIR)/$@.so $(XTRANSSERVER_OBJ) $(DATABASE_MYSQL_OBJ) $(LIBDIR)/libuv.a $(LIBDIR)/liblog4cpp.a $(CPPFLAGS) $(LDFLAGS) $(XTRANSSERVER_LDFLAGS) $(DATABASE_MYSQL_LDFLAGS) -shared -fPIC

libXTransServerAll : $(XTRANSSERVER_OBJ) $(DATABASE_ORACLE_OBJ) $(DATABASE_MYSQL_OBJ) 
	$(CC) -o $(LIBDIR)/$@.so $(XTRANSSERVER_OBJ) $(DATABASE_ORACLE_OBJ) $(DATABASE_MYSQL_OBJ) $(LIBDIR)/libuv.a $(LIBDIR)/liblog4cpp.a $(CPPFLAGS) $(LDFLAGS) $(XTRANSSERVER_LDFLAGS) $(DATABASE_ORACLE_LDFLAGS) $(DATABASE_MYSQL_LDFLAGS) -shared -fPIC
		
createdir:
	$(shell if [ -f ../bin ]; then echo "exist"; else mkdir ../bin; fi;)
	$(shell if [ -f ../bin/log ]; then echo "exist"; else mkdir ../bin/log; fi;)

clean:
	-rm *.o
	
#-rm XTransServer $(XTRANSSERVER_OBJ)
#objects := $(wildcard *.o) 通配符，objects=所有的*.o
#VPATH = src:../headers 特殊变量 VPATH，指定目录寻找依赖文件，目录冒号分开
#vpath %.cpp dir1:dir2
