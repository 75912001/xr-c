#XR_PATH=编译完成后导出的路径(include&&libxr.so)
EXE=libxr.so
CC=g++ -std=c++11

FLAG=-Wall -Wconversion -Wno-sign-conversion -fPIC -shared -O2 -g -o
LOCAL_INCLUDE=../include

INCLUDES=-I$(LOCAL_INCLUDE) -I/usr/include/libxml2

LINK=-lpthread -lxml2

LOCAL_SRCS=$(wildcard ./*.cpp)

$(EXE):
	$(CC) $(LOCAL_SRCS) $(INCLUDES) $(LINK) $(FLAG) $@
	rm $(XR_PATH) -rf 
	mkdir -p $(XR_PATH)
	cp -f $@ $(XR_PATH)
	cp -rf $(LOCAL_INCLUDE) $(XR_PATH)
	rm $@
clean:
	rm $(EXE) *.o -f

