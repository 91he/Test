# Parameters
SRC_CLNT = rdict.c rdict_cif.c 
SRC_SVC  = rdict_srp.c rdict_sif.c

STUB_SPEC = rdict.x
STUB_SVC  = rdict_svc.c
STUB_CLNT = rdict_clnt.c
STUB = $(STUB_CLNT) $(STUB_SVC) rdict.h

TARGET_CLNT = rdict
TARGET_SVC  = rdictd

# Compiler and its flags
CC = gcc
RM = rm -f

CFLAGS += -g -o
RPC_COMPILER = rpcgen

# Targets
all:$(TARGET_CLNT) $(TARGET_SVC)

$(TARGET_CLNT):$(SRC_CLNT) $(STUB_CLNT)
	$(CC) $(CFLAGS) $(TARGET_CLNT) $(SRC_CLNT) $(STUB_CLNT)

$(TARGET_SVC):$(SRC_SVC) $(STUB_SVC)
	$(CC) $(CFLAGS) $(TARGET_SVC) $(STUB_SVC) $(SRC_SVC) 

$(STUB):$(STUB_SPEC)
	$(RPC_COMPILER) $(STUB_SPEC)

clean:
	$(RM) $(TARGET_CLNT)
	$(RM) $(TARGET_SVC)
	$(RM) $(STUB)