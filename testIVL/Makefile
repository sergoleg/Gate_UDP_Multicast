
UNAME := WIN
#UNAME := UNIX

ifeq ($(UNAME), WIN)
# start for win32
#DEFINES           = -DWIN32 -D_WIN32 -DNDEBUG
DEFINES           = -DWIN32 -D_WIN32           -D_SKIP_ERROR
LDOPTIONS         = -lws2_32 -lm -lstdc++
INCLUDE_LIBCONFIG = -Ilibconfig_win
LIB_LIBCONFIG     = libconfig_win/libconfig++.dll.a
# stop for win32
else
# start for Linux, Unix
#DEFINES           = -D_LINUX -DNDEBUG -D_SKIP_ERROR
DEFINES           = -D_LINUX -D_SKIP_ERROR
LDOPTIONS         = -lm -lstdc++
INCLUDE_LIBCONFIG = -Ilibconfig_x32
LIB_LIBCONFIG     = libconfig_x32/libconfig++.a
# stop for Linux, Unix
endif



CC = gcc -m32
#CC = g++

CFLAGS = -g -O2 -Wall -W -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-deprecated-declarations -Wformat=0 $(DEFINES)

AR        = ar
AROPTIONS = -cru
RANLIB    = ranlib

ifeq ($(UNAME), WIN)
# start for win32
#mods = libLoggerCpp.a libLinkXika.a LoggerCpp_test Wconf_test Wshm_test Wmcast_test_send Wmcast_test_recv Wxika_test GateIvs
mods =  libLoggerCpp.a libLinkXika.a Wmcast_test_send Wmcast_test_recv
# stop for win32
else
# start for Linux, Unix
#mods = libLoggerCpp.a libLinkXika.a LoggerCpp_test Wconf_test Wshm_test Wmcast_test_send Wmcast_test_recv Wxika_test GateIvs
mods =  libLoggerCpp.a libLinkXika.a Wmcast_test_send Wmcast_test_recv
# stop for Linux, Unix
endif



mod : $(mods)

all : mod



INCLUDE_LOGGERCPP = -ILoggerCpp/Includes -ILoggerCpp/Sources
INCLUDE_LINKXIKA  = -ILinkXika/Includes -ILinkXika/Sources -ILinkXika/Includes/test -ILinkXika/Sources/test



#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
TRG			= libLoggerCpp.a
TRG_PATH	= LoggerCpp
$(TRG) :	$(TRG_PATH)/Objects/Config.o \
			$(TRG_PATH)/Objects/DateTime.o \
			$(TRG_PATH)/Objects/Log.o \
			$(TRG_PATH)/Objects/Logger.o \
			$(TRG_PATH)/Objects/Manager.o \
			$(TRG_PATH)/Objects/OutputConsole.o \
			$(TRG_PATH)/Objects/OutputDebug.o \
			$(TRG_PATH)/Objects/OutputFile.o \
			$(TRG_PATH)/Objects/OutputSyslog.o
			rm -f $@
			$(AR) $(AROPTIONS) $@ $^
			$(RANLIB) $@
$(TRG_PATH)/Objects/%.o : $(TRG_PATH)/Sources/%.cpp
	$(CC) -c -o $@ $(INCLUDE_LOGGERCPP) $(CFLAGS) $<

#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
TRG			= libLinkXika.a
TRG_PATH	= LinkXika
$(TRG) :	$(TRG_PATH)/Objects/Wmcast.o \
			$(TRG_PATH)/Objects/Wxika.o
			rm -f $@
			$(AR) $(AROPTIONS) $@ $^
			$(RANLIB) $@
$(TRG_PATH)/Objects/%.o : $(TRG_PATH)/Sources/%.cpp
	$(CC) -c -o $@ $(INCLUDE_LIBCONFIG) $(INCLUDE_LOGGERCPP) $(INCLUDE_LINKXIKA) $(CFLAGS) $<

#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
TRG			= Wmcast_test_send
TRG_PATH	= LinkXika
$(TRG) :	$(TRG_PATH)/Objects/test/Wmcast_test_send.o
			$(CC) -o $@ $^ libLinkXika.a libLoggerCpp.a $(LIB_LIBCONFIG) $(LDOPTIONS)
$(TRG_PATH)/Objects/test/%.o : $(TRG_PATH)/Sources/test/%.cpp
	$(CC) -c -o $@ $(INCLUDE_LOGGERCPP) $(INCLUDE_LIBCONFIG) $(INCLUDE_LINKXIKA) $(CFLAGS) $<

#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
TRG			= Wmcast_test_recv
TRG_PATH	= LinkXika
$(TRG) :	$(TRG_PATH)/Objects/test/Wmcast_test_recv.o
			$(CC) -o $@ $^ libLinkXika.a libLoggerCpp.a $(LIB_LIBCONFIG) $(LDOPTIONS)
$(TRG_PATH)/Objects/test/%.o : $(TRG_PATH)/Sources/test/%.cpp
	$(CC) -c -o $@ $(INCLUDE_LOGGERCPP) $(INCLUDE_LIBCONFIG) $(INCLUDE_LINKXIKA) $(CFLAGS) $<

#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
.PHONY: objdirs
TargetDirs = $(shell ls -d *)
objects:	$(addsuffix /Objects, $(TargetDirs))
%/Objects:	%
		mkdir $@

.PHONY: clean
clean:
	rm -f libLoggerCpp.a
	rm -f libLinkXika.a
	rm -f Wmcast_test_send*
	rm -f Wmcast_test_recv*

	find . -name '*.o' | xargs rm -f 
