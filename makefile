LDLIBS += -lpcap

all: beacon-flood

beacon-flood: *.c

clean:
	rm -f beacon-flood *.o
