#include <stdio.h>

void usage();


int main(int argc, char* argv[]) {

    if(argc != 3) {
        usage();
        return -1;
    }

    int res;
    const char* interface = argv[1];
    const char* filename = argv[2];
    // res = beacon_flood(interface, filename);

    return 0;

}


void usage() {
    puts("syntax: beacon-flood <interface> <ssid_list_file>");
    puts("sample: beacon-flood wlan0 ssid_list.txt");
}
