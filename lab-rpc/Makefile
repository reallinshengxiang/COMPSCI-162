BIN_DIR = bin

CLIENT = $(BIN_DIR)/client
SERVER = $(BIN_DIR)/server

SOURCES_CLNT.c = client.c kv_store_client.c
SOURCES_CLNT.h = kv_store_client.h
SOURCES_SVC.c = server.c
SOURCES_SVC.h =
SOURCES.x = kv_store.x

TARGETS_SVC.c = kv_store_svc.c kv_store_xdr.c 
TARGETS_CLNT.c = kv_store_clnt.c kv_store_xdr.c 
TARGETS = kv_store.h kv_store_xdr.c kv_store_clnt.c kv_store_svc.c

OBJECTS_CLNT = $(SOURCES_CLNT.c:%.c=%.o) $(TARGETS_CLNT.c:%.c=%.o)
OBJECTS_SVC = $(SOURCES_SVC.c:%.c=%.o) $(TARGETS_SVC.c:%.c=%.o)
# Compiler flags 

UNAME := $(shell uname -m)
CFLAGS += -g -I/usr/include/tirpc -I/usr/include/glib-2.0
ifeq "$(UNAME)" "x86_64"
	CFLAGS +=  -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
endif
ifeq "$(UNAME)" "aarch64"
	CFLAGS += -I/usr/lib/aarch64-linux-gnu/glib-2.0/include
endif
ifeq "$(UNAME)" "arm64"
	CFLAGS += -I/usr/lib/arm64-linux-gnu/glib-2.0/include
endif
LDLIBS += -lnsl -lglib-2.0 -ltirpc
RPCGENFLAGS =

# Targets 

all : $(CLIENT) $(SERVER)

$(BIN_DIR) :
	mkdir -p $(BIN_DIR)

$(TARGETS) : $(SOURCES.x) 
	rpcgen $(RPCGENFLAGS) $(SOURCES.x)
	rm kv_store_svc.c
	rpcgen -m -o kv_store_svc.c $(RPCGENFLAGS) $(SOURCES.x)

$(OBJECTS_CLNT) : $(SOURCES_CLNT.c) $(SOURCES_CLNT.h) $(TARGETS_CLNT.c) 

$(OBJECTS_SVC) : $(SOURCES_SVC.c) $(SOURCES_SVC.h) $(TARGETS_SVC.c) 

$(CLIENT) : $(OBJECTS_CLNT) $(BIN_DIR)
	$(LINK.c) -o $(CLIENT) $(OBJECTS_CLNT) $(LDLIBS) 

$(SERVER) : $(OBJECTS_SVC) $(BIN_DIR)
	$(LINK.c) -o $(SERVER) $(OBJECTS_SVC) $(LDLIBS)

 clean:
	 $(RM) core $(TARGETS) $(OBJECTS_CLNT) $(OBJECTS_SVC) $(CLIENT) $(SERVER)
