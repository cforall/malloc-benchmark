CC  := $(CC)
CXX := $(CXX)

CFLAGS  := -g3 -O3 -Wall -Wextra #-DMALLOC_PRELOAD_DEBUG
LIBS:= dataStructures.c utilities.c -lm

#####

TARGET = mallocWrappers.so
all : $(TARGET)
	@echo "DONE: malloc wrappers"

#####

mallocWrappers.so: mallocWrappers.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $^ $(LIBS) -lpthread -ldl

#####

clean:
	rm -f $(TARGET)
