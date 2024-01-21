#include <stdio.h>
#include <pcap.h> // for pcap
#include <string.h> // for memcpy
#include <stdlib.h> // for calloc
#include <time.h>

#include "radiotap.h"
#include "BeaconFrame.h"

typedef struct ieee80211_FixMacHeader MacHdr;
typedef struct ieee80211_FixFrameBody FrameBody;

typedef struct {
    uint8_t header[8];
} RadiotapHdr;

typedef struct {
    RadiotapHdr r_hdr;
    MacHdr m_hdr;
    FrameBody body;
} __attribute__((__packed__)) FakeBeaconFrame;

int beacon_flood(const char* interface, const char* list_file) {

    char errbuf[PCAP_ERRBUF_SIZE];
    const char* ifname = interface;

    // open ssid list file
    FILE* ssid_list_file = fopen(list_file, "rt");
    if(ssid_list_file == NULL) {
        fprintf(stderr, "failed to open file - %s\n", list_file);
        return -1;
    }

    char ssid[1024];
    while (fgets(ssid, sizeof(ssid), ssid_list_file) != NULL) {
        printf("ssid - %s\n", ssid); // 읽은 줄 출력
    }

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

    RadiotapHdr radio_hdr = {{0,}};
    printf("radio header1 - %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu\n",
    radio_hdr.header[0],radio_hdr.header[1],radio_hdr.header[2],radio_hdr.header[3]
    ,radio_hdr.header[4],radio_hdr.header[5],radio_hdr.header[6],radio_hdr.header[7]);

    printf("mac_hdr\n");
    MacHdr mac_hdr;
    printf("mac_hdr size: %d\n", sizeof(mac_hdr));
    // frame control field: (type(4bit) + subtype(2bit) + protocol ver(2bit)) (0b 1000 0000 -> 0x80) + flags (0b 0000 0000 -> 0x00)
    printf("frame_control: %02x\n", mac_hdr.frame_control);
    mac_hdr.frame_control = (uint16_t)0x8000 + (uint16_t)0x0000;

    mac_hdr.duration = (uint16_t)0x0000;

    printf("memcpy on add1, 2, 3");
    // beacon frame의 addr1은 DA
    uint8_t broadcast[6] = {0xff,};
    memcpy(mac_hdr.addr1, broadcast, sizeof(uint8_t) * 6);

    // add2는 SA, addr3는 BSSID
    uint8_t fake_sa[6] = {0xaa,};
    memcpy(mac_hdr.addr2, fake_sa, sizeof(uint8_t) * 6);
    memcpy(mac_hdr.addr3, fake_sa, sizeof(uint8_t) * 6);

    printf("memcpy on seq ctl");
    uint8_t fake_seq_ctl[2] = {0xaa,};
    memcpy(&mac_hdr.seq_control, fake_seq_ctl, sizeof(uint8_t) * 2);


    FrameBody frame_body;
    srand(time(NULL));
    frame_body.timestamp = ((uint64_t)rand() << 32) | rand();
    frame_body.beacon_interval = 0x6400;
    frame_body.cap_info = 0x0c11; // automatic power save delivery, short slot time, ESS capabilities bits 1

    // set ssid tag
    frame_body.ssid_tag.tag_num = 0x00;
    const char* fake_ssid = "hdxianTest";
    frame_body.ssid_tag.tag_len = strlen(fake_ssid);
    frame_body.ssid_tag.data = calloc(strlen(fake_ssid), 1);
    strncpy(frame_body.ssid_tag.data, fake_ssid, strlen(fake_ssid));

    // set supported rate tag
    frame_body.supp_rate_tag.tag_num = 0x01;
    frame_body.supp_rate_tag.tag_len = 0x08;
    frame_body.supp_rate_tag.data = calloc(0x08, 1);
    // 82 84 8b 96 0c 12 18 24
    snprintf(frame_body.supp_rate_tag.data, 8, "%hhu %hhu %hhu %hhu %hhu %hhu %hhu", 0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24);

    // set channel tag
    frame_body.channel_tag.tag_num = 0x03;
    frame_body.channel_tag.tag_len = 0x01;
    frame_body.supp_rate_tag.data = calloc(0x01, 1);
    *(frame_body.supp_rate_tag.data) = 0x01;

    FakeBeaconFrame frame;
    frame.r_hdr = radio_hdr;
    frame.m_hdr = mac_hdr;
    frame.body = frame_body;

    // while(1) {
    // }
    for(int i=0; i<10; i++) {
        unsigned char* packet = (unsigned char*)&frame;
        pcap_sendpacket(handle, packet, sizeof(frame));
    }
    
    free(frame_body.ssid_tag.data);
    free(frame_body.supp_rate_tag.data);
    free(frame_body.channel_tag.data);

    return 0;

}
