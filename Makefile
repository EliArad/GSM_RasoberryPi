CC      = gcc
CFLAGS  = -g
RM      = rm -f


default: all

all: brouter

brouter: brouter.c  aes.c  test.c  brouter.h  tcpserver.c  fifo.c  fifo.h tcp_client.c \
	  config.c  rtsp_fifo.c rtsp_input.c  os.c rtsp_video.c base64.c base64.h  cutils.c tcp_sender.c  udp_unicast_server.c
	$(CC) $(CFLAGS) -o brouter brouter.c config.c  fifo.c   aes.c  test.c  rtsp_fifo.c tcp_client.c   udp_unicast_server.c  \
	 tcp_sender.c  tcpserver.c  rtsp_input.c  os.c   rtsp_video.c   base64.c  cutils.c  -lpthread

clean veryclean:
	$(RM) brouter