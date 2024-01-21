LDLIBS += -lpcap

all: beacon-flood

airodump: *.c

clean:
	rm -f beacon-flood *.o
