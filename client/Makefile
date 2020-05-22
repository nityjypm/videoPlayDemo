CC = gcc
CFLAGS = -Wall -fexceptions
APP := cvSocketServerTest

SRCS := $(wildcard ./*.c)

OBJS := $(SRCS:.c=.o)

CFLAGS += \
	-I"./" -O2

	
LDFLAGS +=
LIBS +=

all: $(APP)
#	cp $(APP) ../bin/$(APP)
	rm -rf $(OBJS)
%.o: %.CFLAGS
#	@echo "Compiling: $<"
#	protoc --cpp_out=. $(CONFIG_DIR)/config.proto
	$(CC) $(CFLAGS) -c $<

$(APP): $(OBJS)
#	@echo "Linking: $@"
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS) $(LIBS)	

clean:
#	cp $(APP) ../bin/$(APP)
	rm -rf $(APP) $(OBJS)
