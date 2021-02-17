#ifndef RTSP_H 
#define RTSP_H 
#include <stdint.h>


#define VERSION           "0.1"
#define PROTOCOL_PREFIX   "rtsp://"
#define RTSP_PORT         554
#define RTSP_CLIENT_PORT  9500
#define RTSP_RESPONSE     "RTSP/1.0 "
#define CMD_OPTIONS       "OPTIONS rtsp://%s:%lu RTSP/1.0\r\nCSeq: %i\r\n\r\n"
#define CMD_DESCRIBE      "DESCRIBE %s RTSP/1.0\r\nCSeq: %i\r\nAccept: application/sdp\r\n\r\n"
#define RTP_FREQ    90000
#define RTP_SPROP   "sprop-parameter-sets="
#define CMD_SETUP         "SETUP %s RTSP/1.0\r\nCSeq: %i\r\nTransport: RTP/AVP/TCP;interleaved=0-1;\r\n\r\n"
#define CMD_PLAY          "PLAY %s RTSP/1.0\r\nCSeq: %i\r\nSession: %lu\r\nRange: npt=0.00-\r\n\r\n"

/* Transport header constants */
#define SETUP_SESSION      "Session: "
#define SETUP_TRNS_CLIENT  "client_port="
#define SETUP_TRNS_SERVER  "server_port="

 /* Check if a bit is 1 or 0 */
#define CHECK_BIT(var, pos) !!((var) & (1 << (pos)))


#pragma pack(push, 1)
typedef union
{
	uint8_t Value;

	struct
	{
		unsigned char cc : 4;          /* CSRC count */
		unsigned char padding : 1;     /* padding flag */
		unsigned char extension : 1;   /* header extension flag */
		unsigned char version : 2;     /* protocol version */
	}u;

} RTP_Byte_0;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union
{
	uint8_t Value;

	struct
	{		
		unsigned char pt : 7;          /* payload type */
		unsigned char marker : 1;      /* marker bit */
	}u;

} RTP_Byte_1;
#pragma pack(pop)


#pragma pack(push, 1)
typedef union
{
	uint32_t Value;

	struct
	{
		unsigned int S : 1;    
		unsigned int L : 1;    
		unsigned int R : 1;    
		unsigned int D : 1;    
		unsigned int I : 1;
		unsigned int Res : 3;
		unsigned int length : 24;
	}u;

} RTP_BytePF;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct rtp_header_tag 
{
	RTP_Byte_0  byte_0;
	RTP_Byte_1  byte_1;
	
	uint16_t seq;            /* sequence number */
	uint32_t ts;                /* timestamp */
	uint32_t ssrc;              /* synchronization source */
} rtp_header;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct rtp_payload_format
{
	RTP_Byte_0  byte_0;
	RTP_Byte_1  byte_1;

	uint16_t seq;            /* sequence number */
	uint32_t ts;                /* timestamp */

} rtp_payload_format;

#pragma pack(pop)




typedef struct rtsp_session_
{
	int socket;
	char *stream;

	/* RTSP protocol stuff */
	unsigned int packetization; /* Packetization mode from SDP data */
	unsigned int cport_from;    /* client_port from */
	unsigned int cport_to;      /* client port to   */
	unsigned int sport_from;    /* server port from */
	unsigned int sport_to;      /* server port to   */
	unsigned long session;      /* session ID       */
} rtsp_session;


enum RTSPProtocol {							 
	RTSP_PROTOCOL_RTP_UDP = 0,
	RTSP_PROTOCOL_RTP_TCP = 1,
	RTSP_PROTOCOL_RTP_UDP_MULTICAST = 2,
};

#define RTSP_DEFAULT_PORT   554				 
#define RTSP_MAX_TRANSPORTS 8				 
#define RTSP_TCP_MAX_PACKET_SIZE 1472		 
											 
typedef struct RTSPTransportField {			 
	int interleaved_min, interleaved_max;	 
	int port_min, port_max; 				 
	int client_port_min, client_port_max;	 
	int server_port_min, server_port_max;	 
	int ttl; 								 
	unsigned int destination; 				 
	enum RTSPProtocol protocol;				 
} RTSPTransportField;

typedef struct RTSPHeader
{
	int content_length;						 
	int status_code; 		 
	int nb_transports;						 
	RTSPTransportField transports[RTSP_MAX_TRANSPORTS];
	int seq; 								 
	char session_id[512];					 
} RTSPHeader;

// the callback can be used to extend the connection setup/teardown step  
enum RTSPCallbackAction {
	RTSP_ACTION_SERVER_SETUP,					 
	RTSP_ACTION_SERVER_TEARDOWN,				 
	RTSP_ACTION_CLIENT_SETUP,					 
	RTSP_ACTION_CLIENT_TEARDOWN,				 
};												 


typedef struct RTSPActionServerSetup {
	unsigned int  ipaddr;				 			 
	char transport_option[512];			 
} RTSPActionServerSetup;				 

typedef struct URLContext_tag {
	//struct URLProtocol *prot; 
	int flags;
	int is_streamed;		// true if streamed (no seek possible), default = false  
	int max_packet_size;	// if non zero, the stream is packetized with this max packet size  
	void *priv_data;
	char filename[1];		// specified filename  
} URLContext;

typedef struct RTSPState {

	URLContext *rtsp_hd; 	// RTSP TCP connexion handle  

	int fd;					// socket file descriptor  
	// Server  
	int state;
	uint8_t buffer_ptr[30];
	uint8_t *buffer_ptr1, *buffer_end;
	uint8_t *buffer;
	char protocols[16];
	char method[16];
	char url[128];
	int buffer_size;
	// RTSP state specific  
	uint8_t *pb_buffer; // XXX: use that in all the code  

	// SDP  
	int nb_streams;
	char title[128];
	char comment[128];
	int sdp_port; 			// port (from SDP content - not used in RTSP)  
	struct in_addr sdp_ip; 	// IP address  (from SDP content - not used in RTSP)  
	int sdp_ttl;  			// IP TTL (from SDP content - not used in RTSP)  
	int sdp_payload_type; 	// payload type - only used in SDP  
	char control_url[1024]; // url for this stream (from SDP)  

	int codec_id;
	int codec_type;

	// XXX: currently we use unbuffered input  
	// ByteIOContext rtsp_gb; 
	int seq;        			// RTSP command sequence number  
	char session_id[512];		// session id  
	enum RTSPProtocol protocol;	// 
	char last_reply[2048]; 		// XXX: allocate ?  

} RTSPState;

  
typedef int FFRTSPCallback(enum RTSPCallbackAction action, 	// event type 
	const char *session_id,			// session id 
	char *buf, int buf_size,			// buffer pointer & buffer size 
	void *arg);						// argument(rtsp.c or ffserver.c ) 


															 
extern int rtsp_default_protocols;							 
extern int rtsp_rtp_port_min;								 
extern int rtsp_rtp_port_max;								 
extern FFRTSPCallback *ff_rtsp_callback;					 
//extern AVInputFormat rtsp_demux;							 
															 

typedef struct SDPParseState {
	/* SDP only */
	struct in_addr default_ip;
	int default_ttl;
} SDPParseState;



#endif /* RTSP_H */ 