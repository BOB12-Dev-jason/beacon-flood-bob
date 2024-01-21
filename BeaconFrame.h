#ifndef BOB12_BEACONFRAME_H_HDXIAN
#define BOB12_BEACONFRAME_H_HDXIAN

#include <stdint.h>

struct ieee80211_radiotapMoreHeader {
    uint8_t flags;
    uint8_t rate;
    uint16_t frequency;
    uint16_t channel_flags;
    uint8_t antenna_signal;
};


// Beacon Frame is a type of Management Frame
// fixed part of Management Frame MAC Header
struct ieee80211_FixMacHeader {
    uint16_t frame_control;
    uint16_t duration;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint16_t seq_control;
    // addr4, qos, ... 는 optional
};


typedef struct ieee80211_TaggedParameter {
    uint8_t tag_num;
    uint8_t tag_len;
    unsigned char* data; // maximum tag length is 256.
} TaggedParameter;


struct ieee80211_FixFrameBody {
    uint64_t timestamp;         // timestamp. 8 bytes
    uint16_t beacon_interval;   // beacon interval. 2 byte
    uint16_t cap_info;          // capability information. 2 byte
    
    // composed with essential tags only for beacon flooding. real tag parameters are very various and variable.
    TaggedParameter* ssid_tag;
    TaggedParameter* supp_rate_tag;
    TaggedParameter* channel_tag;
};


struct BeaconFrame {
    uint8_t radiotap[8];
    // Header
    uint16_t frame_control;
    uint16_t duration;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint16_t seq_control;

    // body
    uint64_t timestamp;         // timestamp. 8 bytes
    uint16_t beacon_interval;   // beacon interval. 2 byte
    uint16_t cap_info;          // capability information. 2 byte
    uint8_t ssid_tag_num;
    uint8_t ssid_tag_len;
    // unsigned char* ssid;
}__attribute__((__packed__));


#endif
