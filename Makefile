# @author Matthew Strait

CXX=g++

CPPFLAGS=-Wall -Wextra -O3 -ffast-math -fPIC -fno-threadsafe-statics
LINKFLAGS=$(CPPFLAGS)

ROOTINC = `root-config --cflags` -I${DOGS_PATH}/DCDisplay/ZOE

LIB += -lm `root-config --libs`

all: idivc

idivc_obj = idivc_main.o idivc_root.o

other_obj = ${DOGS_PATH}/DCDisplay/ZOE/z{geo,cont}.o

idivc: $(idivc_obj) 
	@echo Linking idivc
	@$(CXX) $(LINKFLAGS) $(LIB) -o idivc $(idivc_obj) $(other_obj)

idivc_root.o: idivc_root.cpp idivc_cont.h
	@echo Compiling $<
	@$(COMPILE.cc) $(ROOTINC) $(OUTPUT_OPTION) $<

idivc_main.o: idivc_main.cpp idivc_cont.h idivc_root.h idivc_progress.cpp
	@echo Compiling $<
	@$(COMPILE.cc) $(ROOTINC) $(OUTPUT_OPTION) $<

clean: 
	@rm -f idivc *.o *_dict.* G__* AutoDict_* *_dict_cxx.d
