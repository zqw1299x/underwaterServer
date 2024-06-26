
CROSS_PREFIX = arm-linux-
CC = $(CROSS_PREFIX)gcc

STRIP = $(CROSS_PREFIX)strip


CFLAGS = -D_GNU_SOURCE -std=c99 -g   -DUSE_NOR_FLASH  -DRUN_IN_ARM

DIST_DIR = ../dist
BIN_DIR = ../bin
LIB_DIR = ../lib

CFLAGS += -lpthread -lrt -lm -ldl -L$(LIB_DIR)

TARGET = $(BIN_DIR)/UWaterServer
#TARGET = $(BIN_DIR)/monitor.bin
WATCHDOG_EXEC = $(BIN_DIR)/monitor.bin
DEBUG_EXEC = $(BIN_DIR)/debug

APPOBJS = main.c
#APPOBJS = monitor/monitor_main.c
#APPOBJS = test.c 
GLOBAOBJS = system.c
LOGOBJS = log/writelog.c
UARTOBJS = uart/uart.c

MONITOROBJS = monitor/monitor_main.c

DEBUGOBJS = monitor/debug.c

EXTERNLIB = -lipcmsg -lthread -lencrypt -luart

$(TARGET):$(APPOBJS) $(SO_LIBS) $(GLOBAOBJS) $(UARTOBJS)
	$(CC) $(APPOBJS) $(GLOBAOBJS) $(UARTOBJS) $(LOGOBJS) $(CFLAGS) $(EXTERNLIB) -o $@

$(WATCHDOG_EXEC):$(MONITOROBJS) $(SO_LIBS) $(GLOBAOBJS) $(UARTOBJS)
	$(CC) $(CFLAGS) $(MONITOROBJS) $(GLOBAOBJS) $(UARTOBJS) $(EXTERNLIB) -o $@

$(DEBUG_EXEC):$(DEBUGOBJS) $(SO_LIBS) $(GLOBAOBJS) $(UARTOBJS)
	$(CC) $(CFLAGS) $(DEBUGOBJS) $(GLOBAOBJS) $(UARTOBJS) $(EXTERNLIB) -o $@

$(LIB_DIR):	
	-if [ -e "$(LIB_DIR)" ]; then echo "$(LIB_DIR) exists already"; else mkdir $(LIB_DIR); fi

$(BIN_DIR):	
	-if [ -e "$(BIN_DIR)" ]; then echo "$(BIN_DIR) exists already"; else mkdir $(BIN_DIR); fi

$(DIST_DIR):	
	-if [ -e "$(DIST_DIR)" ]; then echo "$(DIST_DIR) exists already"; else mkdir $(DIST_DIR); fi


SO_LIBS =
include utils/thread/lib_thread.mk
include ipc/ipcmsg.mk
include encrypt/encrypt.mk
include uart/lib_uart.mk

lib: $(LIB_DIR) $(SO_LIBS)

monitor:$(WATCHDOG_EXEC)

debug:$(DEBUG_EXEC)

all:$(LIB_DIR) $(SO_LIBS) $(TARGET) $(WATCHDOG_EXEC) $(DEBUG_EXEC)

strip:
	$(STRIP) $(TARGET)

dist:$(DIST_DIR) 
	-cp $(BIN_DIR)/* $(DIST_DIR)
	-cp $(LIB_DIR)/* $(DIST_DIR)
	$(STRIP) $(DIST_DIR)/*
	
dirclean:
	rm -rf $(BIN_DIR)

clean:
	rm -rf $(TARGET)

allclean:
	rm -rf $(TARGET) $(SO_LIBS) $(WATCHDOG_EXEC) $(DEBUG_EXEC)
