#ifndef CHARLIE_INTERFACE_H
#define CHARLIE_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif
void processing_threadV(int isInverted);

int sound_read(void *buf, int size, int count);


typedef unsigned char  ui8_t;
typedef unsigned short ui16_t;
typedef unsigned int   ui32_t;
typedef char  i8_t;
typedef short i16_t;
typedef int   i32_t;


#define BITFRAME_LEN    ((51*16)/2)  // ofs=8: 52..53: AA AA (1..5) or 00 00 (6)
#define RAWBITFRAME_LEN (BITFRAME_LEN*2)
#define FRAMESTART      (HEADOFS+HEADLEN)

#define FRAME_LEN       (BITFRAME_LEN/8)


typedef struct {
    i8_t vbs;  // verbose output
    i8_t raw;  // raw frames
    i8_t crc;
    i8_t ecc;  // Hamming ECC
    i8_t sat;  // GPS sat data
    i8_t ptu;  // PTU: temperature humidity (pressure)
    i8_t inv;
    i8_t aut;
    i8_t col;  // colors
    i8_t jsn;  // JSON output (auto_rx)
    i8_t slt;  // silent
    i8_t dbg;
    i8_t unq;
} option_t;

typedef struct {
    ui8_t subcnt1;
    ui8_t subcnt2;
    //int frnr;
    int yr; int mth; int day;
    int hrs; int min; int sec;
    double lat; double lon; double alt;
    double vH; double vD; double vV;
    ui8_t numSats;
    float calA; // A(ntc)
    float calB; // B(ntc)
    float calC; // C(ntc)
    float A_adcT; float B_adcT; float C_adcT;
    float A_adcH; float B_adcH; float C_adcH;
    ui8_t frame[FRAME_LEN+16];
    char frame_bits[BITFRAME_LEN+16];
    ui32_t cfg[16];
    ui32_t snC;
    ui32_t snD;
    float T; float RH;
    ui8_t cfg_ntc; ui8_t cfg_T; ui8_t cfg_H;
    ui8_t crcOK;
    //
    int sec_day;
    int sec_day_prev;
    int gps_cnt;
    int gps_cnt_prev;
    int week;
    int jsn_freq;   // freq/kHz (SDR)
    option_t option;
} gpx_t;


void display_packet(gpx_t *gpx, int crcOK);

#ifdef __cplusplus
}
#endif

#endif // CHARLIE_INTERFACE_H
