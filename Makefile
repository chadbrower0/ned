# variables ##################################################

MODE = ${suffix ${basename $(MAKECMDGOALS)}}

# check target
ifeq ($(MAKECMDGOALS),)
else ifeq ($(MAKECMDGOALS),all)
else ifeq ($(MAKECMDGOALS),help)
else ifeq ($(MAKECMDGOALS),clean)
else ifeq ($(MODE),) 
  $(error "Need to specify a compile mode." )
endif

# set compile options for compile mode
CXXFLAGS_DEBUG = -g -D DEBUG=DEBUG -w
CXXFLAGS_FAST = -g -D PSEUDOSTACK=TRUE -O3 -w
ifeq ($(MODE),.debug)
  CXXFLAGS =  $(CXXFLAGS_DEBUG)
else ifeq ($(MODE),.fast)
  CXXFLAGS =  $(CXXFLAGS_FAST)
endif

# set compile options
INCLUDE_PATH =  -I .   -I /usr/include/ncurses
LIBRARY_PATH =  -lncurses -lpanel


# targets ##################################################

all:  main.cpp  *.h  Shared/*.h
	g++ $(CXXFLAGS_DEBUG)  main.cpp  -o main.debug.exe  $(INCLUDE_PATH) $(LIBRARY_PATH)
	g++ $(CXXFLAGS_FAST)  main.cpp  -o main.fast.exe  $(INCLUDE_PATH) $(LIBRARY_PATH)
	rm -f ned
	cp main.debug.exe ned
	@echo 'Add to path:  PATH=$$PATH:'`pwd`

help:  
	@echo 
	@echo "Make targets:  all  main.debug.exe  clean " 
	@echo 

main$(MODE).exe:  main.cpp  *.h  Shared/*.h
	g++ $(CXXFLAGS)  main.cpp  -o main$(MODE).exe  $(INCLUDE_PATH) $(LIBRARY_PATH)

clean:
	rm -f  *.exe  *.stackdump  *.core  *.log  ned 


