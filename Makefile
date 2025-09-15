CC = gcc

ifeq ($(OS),Windows_NT)
	TARGET = main.exe
	CFLAGS = -g -Wall -Iinclude -MMD -MP
	LDLIBS = -lm
else
	TARGET = main
	CFLAGS = -g -Wall -fPIC -Iinclude -MMD -MP
	LDLIBS = -lm -ldl
endif


SRC = lib/swe/sweph.c lib/swe/swephlib.c lib/swe/swejpl.c lib/swe/swedate.c lib/swe/swemmoon.c lib/swe/swemplan.c lib/swe/swehouse.c lib/swe/swecl.c lib/swe/swehel.c
OBJECTS = $(SRC:.c=.o)

.PHONY: clean

$(TARGET): main.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $^ $(LDLIBS)
	rm -f main.o $(OBJECTS) $(OBJECTS:.o=.d) main.d

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) main.o $(OBJECTS) $(OBJECTS:.o=.d) main.d

