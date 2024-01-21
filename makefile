LDLIBS += -lpcap

all: beacon-flood

beacon-flood: main.c beacon-flood.c

clean:
	rm -f beacon-flood *.o
