#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h> //  our new library
#include "aes.h"
#include "brouter.h"
#include "rtsp_fifo.h"
#include "fifo.h"
#include "rtsp_video.h"
#include "base64.h"

static int init_socket_for_rtsp();
static int IhSocket = -1;
static struct sockaddr_in ISockAddr;
static FILE *handle = NULL;
static uint8_t m_deliverBuffer[10000];
static int m_dwr = 0;
static char SPROS_DescribeResults[3][100];
static SPROS_DescribeBase64  spros_DescribeBase64[3];
int m_running = 1;

static int verbose = 1;
static int dumpfile = 0;

int InitiateRTSPVideo()
{

    int port, i, ret, err;
	char host[50], path[1024], tcpname[1024], cmd[2048];

	RTSP_CreateFifo(1024 * 1024 * 2);

	if (init_socket_for_rtsp() == -1)
	{
	   printf("failed to init socket for rtsp\n");
		return -1;
	}


	int IConnect;
	/* extract hostname and port */

	err = 0;
	i = 0;
	//strcpy(host, "10.0.0.10");
	//strcpy(path, "mystream");
	//strcpy(host, "224.1.1.24");
	strcpy(host, "192.168.10.100");
	//port = 8600;	
	port = 554;

	char test_url[100];
	//sprintf(test_url, "rtsp://%s:%d/mystream", host, port);
	//sprintf(test_url, "rtsp://%s:%d", host, port);	
	sprintf(test_url, "%s", "rtsp://admin:12345@192.168.10.100:554/video1");
	

	IhSocket = socket(AF_INET, SOCK_STREAM, 0);

	ISockAddr.sin_family = AF_INET;
	ISockAddr.sin_port = htons(port);
	ISockAddr.sin_addr.s_addr = inet_addr(host);

	IConnect = connect(IhSocket, (struct sockaddr *)&ISockAddr, sizeof(ISockAddr));
	
	printf("111\n");

	if (IConnect != 0)
	{
	   
		return 0;
	}
	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec =0;
	setsockopt(IhSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

	if (dumpfile == 1)
	{
		handle = fopen("video.ts", "w+b");
		if (handle == NULL)
			return 0;
	}


	char optioncmd[400];
	sprintf(optioncmd, "OPTIONS %s RTSP/1.0\r\nCSeq: 2\r\nUser-Agent: OpenRTSP.exe (LIVE555 Streaming Media v2016.10.11)\r\n\r\n", test_url);

	if (rtsp_cmd_option(IhSocket, optioncmd, strlen(optioncmd)) == 0)
	{
		printf("Failed to do option\n");
		return 0;
	}
	printf("option ok:\n");


	char describecmd[400];
	sprintf(describecmd, "DESCRIBE %s RTSP/1.0\r\nCSeq: 3\r\nUser-Agent: D:\\Temp\\RTSP Server test\\OpenRTSP\\live555-master\\x64\\Debug\\openRTSP.exe (LIVE555 Streaming Media v2016.10.11)\r\nAccept:application/sdp\r\n\r\n", test_url);


	int numSPropRecords = 0;
	if (rtsp_cmd_describe(IhSocket, describecmd, strlen(describecmd), &numSPropRecords) == 0)
	{
		printf("Failed to do describe\n");
		return 0;
	}
	printf("numSPropRecords = %d\n", numSPropRecords);


	char setupcmd[400];
	//sprintf(setupcmd, "SETUP %s/trackID=1 RTSP/1.0\r\nCSeq: 4\r\nUser-Agent:OpenRTSP\r\nTransport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n", test_url);
	sprintf(setupcmd, "SETUP %s/track1 RTSP/1.0\r\nCSeq: 4\r\nUser-Agent:OpenRTSP\r\nTransport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n", "rtsp://192.168.10.100/video1");	

	char Session[50];
	if (rtsp_cmd_setup(IhSocket, setupcmd, strlen(setupcmd), Session) == 0)
	{
		printf("Failed to do setup\n");
		return 0;
	}
	if (verbose)
	{
		printf("cmd setup ok\n");
	}

	char playcmd[400];
	sprintf(playcmd, "PLAY %s RTSP/1.0\r\nCSeq: 5\r\nUser-Agent:OpenRTSP\r\nSession: %s\r\nRange :npt=0.000-\r\n\r\n", test_url, Session);

	if (rtsp_cmd_play(IhSocket, playcmd, strlen(playcmd)) == 0)
	{
		printf("Failed to do setup\n");
		return 0;
	}

	if (verbose)
		printf("cmd play ok\n");

	int totalBytes = 0;
	/* write H264 header */
	totalBytes+= streamer_write_h264_header();
	if (RTSP_FifoPushSocketFirst(IhSocket, 1999) == 0)
	{
		printf("failed to start Push First data\n");
		return 0;
	}
	if (CreateRTSPClientThread(IhSocket) == 0)
	{
		printf("failed to start RTSP Client thread\n");
		return 0;
	}

	uint8_t data[5000];

	printf("starting main rtsp video loop\n");

	while (m_running)
	{
		int r;
		rtp_header header;
		rtp_header *ph;
		int sizeofRtpHeader = sizeof(rtp_header);
		//int r = recv(IhSocket, (char *)data, 4, 0);

		RTSP_FifoPull(data, 4);
		ph = (rtp_header *)data;
		if (data[0] == 0x24)
		{
			int channel = data[1];
			int rtp_length = (data[2] << 8 | data[3]);
			if (channel == 1)
			{
				RTSP_FifoPull((uint8_t *)data, rtp_length);
				continue;
			}

			//printf("   Channel       : %i\n", channel);
			//printf("   Payload Length: %i\n", rtp_length);

			//r = recv(IhSocket, (char *)&header, sizeofRtpHeader, 0);
			RTSP_FifoPull((uint8_t *)&header, sizeofRtpHeader);
			if (header.byte_0.u.version != 2)
			{
				printf("version not 2");
			}
			if (header.byte_0.u.extension == 1)
			{
				printf("extenstion exists");
			}
			else
			{
				/*printf("   >> RTP\n");
				printf("      Version     : %i\n", header.byte_0.u.version);
				printf("      Padding     : %i\n", header.byte_0.u.padding);
				printf("      Extension   : %i\n", header.byte_0.u.extension);
				printf("      CSRC Count  : %i\n", header.byte_0.u.cc);
				printf("      Marker      : %i\n", header.byte_1.u.marker);
				printf("      Payload Type: %i\n", header.byte_1.u.pt);
				printf("      Sequence    : %i\n", header.seq);
				printf("      Timestamp   : %u\n", header.ts);
				printf("      Sync Source : %u\n", header.ssrc);*/

				int paysize = rtp_length - sizeofRtpHeader;
				RTSP_FifoPull((uint8_t *)data, paysize);
				//r = recv(IhSocket, (char *)data, paysize, 0);
				uint8_t *payload = data;
				/*
				* NAL, first byte header
				*
				*   +---------------+
				*   |0|1|2|3|4|5|6|7|
				*   +-+-+-+-+-+-+-+-+
				*   |F|NRI|  Type   |
				*   +---------------+
	*/
				int nal_forbidden_zero = CHECK_BIT(payload[0], 7);
				int nal_nri = (payload[0] & 0x60) >> 5;
				int nal_type = (payload[0] & 0x1F);

				/*printf("      >> NAL\n");
				printf("         Forbidden zero: %i\n", nal_forbidden_zero);
				printf("         NRI           : %i\n", nal_nri);
				printf("         Type          : %i\n", nal_type);*/

				/* Single NAL unit packet */
				if (nal_type >= NAL_TYPE_SINGLE_NAL_MIN &&
					nal_type <= NAL_TYPE_SINGLE_NAL_MAX) {

					/* Write NAL header */
					totalBytes += streamer_write_nal();

					/* Write NAL unit */
					totalBytes += streamer_write(payload, sizeof(paysize));
				}

				else if (nal_type == NAL_TYPE_STAP_A)
				{
					uint8_t *q;
					uint16_t nalu_size;

					q = payload + 1;
					int nidx = 0;

					nidx = 0;
					while (nidx < paysize - 1)
					{
						if (m_running == 0)
							break;
						/* write NAL header */
						totalBytes += streamer_write_nal();

						/* get NALU size */
						nalu_size = (q[nidx] << 8) | (q[nidx + 1]);
						nidx += 2;

						/* write NALU size */
						totalBytes += streamer_write(&nalu_size, 1);

						if (nalu_size == 0) {
							nidx++;
							continue;
						}

						/* write NALU data */
						totalBytes += streamer_write(q + nidx, nalu_size);
						nidx += nalu_size;
					}
				}
				else if (nal_type == NAL_TYPE_FU_A)
				{
					/*printf("         >> Fragmentation Unit\n");*/

					uint8_t *q;
					q = payload;

					uint8_t h264_start_bit = q[1] & 0x80;
					uint8_t h264_end_bit = q[1] & 0x40;
					uint8_t h264_type = q[1] & 0x1F;
					uint8_t h264_nri = (q[0] & 0x60) >> 5;
					uint8_t h264_key = (h264_nri << 5) | h264_type;

					if (h264_start_bit)
					{
						/* write NAL header */
						totalBytes += streamer_write_nal();

						/* write NAL unit code */
						totalBytes += streamer_write(&h264_key, sizeof(h264_key));
					}
					totalBytes += streamer_write(q + 2, paysize - 2);

					if (h264_end_bit) {
						/* nothing to do... */
					}
				}
				else if (nal_type == NAL_TYPE_UNDEFINED)
				{
					printf("NAL_TYPE_UNDEFINED: %i\n", nal_type);
				}
				else
				{
					printf("OTHER NAL!: %i\n", nal_type);
				}
			}
			printf("total bytes = %f\n", (double)totalBytes/ (1024.0 * 1024.0));
			if (m_dwr >= 1500)
			{
				FifoPush(m_deliverBuffer, m_dwr);
				m_dwr = 0;

				// push here to output queue.
			}
			/*if (totalBytes > 1024 * 1024 * 4)
				break;*/
		}
		else
		{
			printf("error here!!");
		}
	}

	if (handle != NULL)
		fclose(handle);
	return 1;

}


int init_socket_for_rtsp()
{

	IhSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (IhSocket == -1) {
		printf("Invalid Socket\n");
		return -1;
	}
	memset(&ISockAddr, 0, sizeof(ISockAddr));

	return IhSocket;

}


int rtsp_cmd_option(int sock, char *buf, int bufsize)
{
	char *err;
	int n = send(sock, buf, bufsize, 0);
	char recvbuf[1000];
	memset(recvbuf, '\0', sizeof(recvbuf));
	n = recv(sock, recvbuf, 1000, 0);
	if (n <= 0) {
		printf("Error: Server did not respond properly, closing...");
		close(sock);
		exit(EXIT_FAILURE);
	}

	if (verbose)
	{
		printf("cmd option: %s\n", recvbuf);
	}
	int status = rtsp_response_status(recvbuf, &err);
	if (status != 200) {

		return 0;
	}
	return 1;

}
int rtsp_cmd_describe(int sock, char *buf, int bufsize, int *numSPropRecords)
{
	*numSPropRecords = 0;
	char *err;
	int n = send(sock, buf, bufsize, 0);
	char recvbuf[1000];
	memset(recvbuf, '\0', sizeof(recvbuf));
	n = recv(sock, recvbuf, 1000, 0);
	if (n <= 0) {
		printf("Error: Server did not respond properly, closing...");
		close(sock);
		exit(EXIT_FAILURE);
	}

	int status = rtsp_response_status(recvbuf, &err);
	if (status != 200) {
		return 0;
	}

	if (verbose)
	{
		printf("cmd describe: %s\n", recvbuf);
	}

	char sPropParameterSetsStr[1000];
	char *p = strstr(recvbuf, "sprop-parameter-sets=");
	if (p == NULL)
		return 0;
	p += strlen("sprop-parameter-sets=");
	strcpy(sPropParameterSetsStr, p);
	p = sPropParameterSetsStr;
	if (strchr(p, ';') != NULL)
		*strchr(p, ';') = 0;

	char *token;

	/* get the first token */
	token = strtok(sPropParameterSetsStr, ",");
	if (token != NULL)
	{
		strcpy(SPROS_DescribeResults[*numSPropRecords], token);
		(*numSPropRecords)++;
	}
	else
		return 1;



	/* walk through other tokens */
	token = strtok(NULL, ",");
	while (token != NULL)
	{
		if (m_running == 0)
			return 0;

		printf(" %s\n", token);
		strcpy(SPROS_DescribeResults[*numSPropRecords], token);
		(*numSPropRecords)++;
		token = strtok(NULL, ",");
	}

	for (unsigned i = 0; i < *numSPropRecords; ++i)
	{
		int outLen = 0;
		unsigned char *p = base64_decode((const unsigned char *)SPROS_DescribeResults[i], strlen(SPROS_DescribeResults[i]), &outLen);
		memcpy(spros_DescribeBase64[i].SPROS_DescribeResultsBase64Data , p, outLen);
		spros_DescribeBase64[i].SPROS_DescribeResultsBase64Size = outLen;
	}


	return 1;
}

/*
 * Returns the RTSP status code from the response, if an error occurred it
 * allocates a memory buffer and store the error message on 'error' variable
 */
int rtsp_response_status(char *response, char **error)
{
	int size = 256;
	int err_size;
	int offset = sizeof(RTSP_RESPONSE) - 1;
	char buf[8];
	char *sep;
	char *eol;
	*error = NULL;

	if (strncmp(response, RTSP_RESPONSE, offset) != 0) {
		*error = (char *)malloc(size);
		snprintf(*error, size, "Invalid RTSP response format");
		return -1;
	}

	sep = strchr(response + offset, ' ');
	if (!sep) {
		*error = (char *)malloc(size);
		snprintf(*error, size, "Invalid RTSP response format");
		return -1;
	}

	memset(buf, '\0', sizeof(buf));
	strncpy(buf, response + offset, sep - response - offset);

	eol = strchr(response, '\r');
	err_size = (eol - response) - offset - 1 - strlen(buf);
	*error = (char *)malloc(err_size + 1);
	strncpy(*error, response + offset + 1 + strlen(buf), err_size);

	return atoi(buf);
}

int rtsp_cmd_setup(int sock, char *buf, int bufsize, char Session[50])
{

	char *err;
	int n = send(sock, buf, bufsize, 0);
	char recvbuf[2000];
	memset(recvbuf, '\0', sizeof(recvbuf));
	n = recv(sock, recvbuf, 1000, 0);
	if (n <= 0) {
		printf("Error: Server did not respond properly, closing...");
		close(sock);
		exit(EXIT_FAILURE);
	}
	
	if (verbose)
	{
		printf("cmd setup: %s\n", recvbuf);
	}

	int status = rtsp_response_status(recvbuf, &err);
	if (status != 200) {

		return 0;
	}



	// search for session:
	char *p = strstr(recvbuf, "Session:");
	if (p == NULL)
		return 0;
	p += strlen("session:");
	while (*p == ' ')
		p++;

	strcpy(Session, p);
	p = strchr(Session, ';');
	if (p == NULL)
		return 0;
	*strchr(Session, ';') = 0;


	return 1;
}

int rtsp_cmd_play(int sock, char *buf, int bufsize)
{
	char *err;
	int n = send(sock, buf, bufsize, 0);
	char recvbuf[2000];
	memset(recvbuf, '\0', sizeof(recvbuf));
	n = recv(sock, recvbuf, strlen("RTSP/1.0 200 OK"), 0);
	if (n <= 0) {
		printf("Error: Server did not respond properly, closing...");
		close(sock);
		exit(EXIT_FAILURE);
	}
	if (strcmp(recvbuf, "RTSP/1.0 200 OK") != 0)
		return 0;


	uint8_t c;
	while (m_running)
	{
		n = recv(sock, (char *)&c, 1, 0);
		if (c == 0x24)
			return 100;
	}
	return 1;
}

int streamer_write_h264_header()
{
	uint8_t nal_header[4] = { 0x00, 0x00, 0x00, 0x01 };

	int count = 8;
	/* [00 00 00 01] [SPS] */
	streamer_write_nal();
	if (handle != NULL)
		fwrite(spros_DescribeBase64[0].SPROS_DescribeResultsBase64Data, 1, spros_DescribeBase64[0].SPROS_DescribeResultsBase64Size, handle);
	count += spros_DescribeBase64[0].SPROS_DescribeResultsBase64Size;
	memcpy(m_deliverBuffer + m_dwr, spros_DescribeBase64[0].SPROS_DescribeResultsBase64Data, spros_DescribeBase64[0].SPROS_DescribeResultsBase64Size);
	m_dwr += spros_DescribeBase64[0].SPROS_DescribeResultsBase64Size;

	streamer_write_nal();
	if (handle != NULL)
		fwrite(spros_DescribeBase64[1].SPROS_DescribeResultsBase64Data, 1, spros_DescribeBase64[1].SPROS_DescribeResultsBase64Size, handle);
	count += spros_DescribeBase64[1].SPROS_DescribeResultsBase64Size;
	memcpy(m_deliverBuffer + m_dwr, spros_DescribeBase64[1].SPROS_DescribeResultsBase64Data, spros_DescribeBase64[1].SPROS_DescribeResultsBase64Size);
	m_dwr += spros_DescribeBase64[1].SPROS_DescribeResultsBase64Size;

	return count;
}

int streamer_write_nal()
{
	uint8_t nal_header[4] = { 0x00, 0x00, 0x00, 0x01 };

	/* write header to file system debug file */
	if (handle != NULL)
		fwrite(&nal_header, 1, sizeof(nal_header), handle);
	memcpy(m_deliverBuffer + m_dwr, &nal_header, sizeof(nal_header));
	m_dwr += 4;
	return 4;

}
int streamer_write(const void *buf, size_t count)
{
	/* write to file system debug file */
	if (handle != NULL)
		fwrite(buf, 1, count, handle);
	memcpy(m_deliverBuffer + m_dwr, buf, count);
	m_dwr += count;
	return count;
}

void RTSPVideo_Close()
{
   m_running = 0;
}

