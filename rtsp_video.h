#ifndef RTSP_VIDEO_H
#define RTSP_VIDEO_H
#include "rtsp.h"


/* Enumeration of H.264 NAL unit types */
enum {
	NAL_TYPE_UNDEFINED = 0,
	NAL_TYPE_SINGLE_NAL_MIN = 1,
	NAL_TYPE_SINGLE_NAL_MAX = 23,
	NAL_TYPE_STAP_A = 24,
	NAL_TYPE_FU_A = 28,
};



int sdp_parse_rtpmap(RTSPState *s, const char *p);


void get_word_sep(char *buf, int buf_size, const char *sep, const char **pp);


typedef struct SPROS_DescribeBase64_tag
{
	unsigned char SPROS_DescribeResultsBase64Data[100];
	int  SPROS_DescribeResultsBase64Size;
} SPROS_DescribeBase64;


int rtsp_response_status(char *response, char **error);
int rtsp_cmd_describe(int sock, char *buf, int bufsize, int *numSPropRecords);
int rtsp_cmd_option(int sock, char *buf, int bufsize);
int rtsp_cmd_setup(int sock, char *buf, int bufsize, char Session[50]);
int rtsp_cmd_play(int sock, char *buf, int bufsize);
int streamer_write_nal();
int streamer_write_h264_header();
int  streamer_write(const void *buf, size_t count);
void RTSPVideo_Close();


#endif
