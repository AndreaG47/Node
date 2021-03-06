/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <arpa/inet.h>

#include <inttypes.h>

#define PORT "6006" // the port client will be connecting to

#define TEST

#define MAXDATASIZE 100 // max number of bytes we can get at once 
#define MAXPACKETSIZE 1500
#define PACKETPERIOD_A 2000000
#define PACKETPERIOD_B 1000000
#define PACKETPERIOD_C 5000000
#define MAXPUBLISHERPACKETSIZE 100

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINTF(x)			printf(x)
#define DEBUG_PRINTF2(x,y)		printf(x,y)
#define DEBUG_PRINTF3(x,y,z)	printf(x,y,z)
#endif

#ifndef DEBUG
#define DEBUG_PRINTF(x)
#define DEBUG_PRINTF(x,y)
#define DEBUG_PRINTF(x,y,z)
#endif

#define DEBUG_L2
#ifdef DEBUG_L2
#define L2DEBUG_PRINTF(x)			printf(x)
#define L2DEBUG_PRINTF2(x,y)		printf(x,y)
#define L2DEBUG_PRINTF3(x,y,z)	printf(x,y,z)
#endif

#ifndef DEBUG_L2
#define L2DEBUG_PRINTF(x)
#define L2DEBUG_PRINTF(x,y)
#define L2DEBUG_PRINTF(x,y,z)
#endif

int CreatePublisherPacket(char buf[])
{
	memset(buf,0,sizeof(buf));
	//memcpy(temp);
	return 0;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

uint64_t TimeElapsed(struct timeval* current, struct timeval* tv){
	uint64_t a;
	a = (current->tv_sec-tv->tv_sec)*1000000+(current->tv_usec-tv->tv_usec);
	return a;
}

struct PacketInfo {
	float rate; 			//packets per second
	int length; 		//packet length
	struct timeval tv; 	//structure to keep track of time to send
};

int main(int argc, char *argv[])
{
	DEBUG_PRINTF("MainSTART\n");
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	char PacketBuffer[MAXPACKETSIZE];

	struct timeval tv, CurrentTime;
	fd_set fds;
	tv.tv_sec=0;
	tv.tv_usec=0;
	CurrentTime.tv_sec=CurrentTime.tv_usec=0;
	struct PacketInfo A, B, C;

	memset(&A,0,sizeof(A));
	memset(&B,0,sizeof(B));
	memset(&C,0,sizeof(C));
	memset(PacketBuffer,5,MAXPACKETSIZE); //fill buffer with any data.

	//---Temporarily hardcoded, but it can be change to the input file or parameters

	A.length=99;
	A.rate= 2;

	B.length=25;
	B.rate= 1;

	C.length=75;
	C.rate= 5;

	//------------------

	DEBUG_PRINTF("MAIN: Parse parameters\n");
	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	DEBUG_PRINTF("MAIN: getaddrinfo\n");
	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	DEBUG_PRINTF("MAIN:Socket and connect\n");
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure


	//
	char PublisherPacket[MAXPUBLISHERPACKETSIZE];
	CreatePublisherPacket(PublisherPacket);


	if (send(sockfd,PublisherPacket,sizeof(PublisherPacket), 0) == -1)
		perror("send");

	//

	while(1)
	{
		FD_ZERO(&fds);
		FD_SET(sockfd,&fds);
		select(sockfd+1,&fds,NULL,NULL,&tv);

		if (FD_ISSET(sockfd,&fds)){

			if((numbytes = recv(sockfd,buf, MAXDATASIZE-1,0))==-1){
				perror("recv");
				exit(1);
				//TODO: exit again!! Where to put it
			}
			else if (numbytes==0){
				break;
			}
			else {
				buf[numbytes] = '\0';
				L2DEBUG_PRINTF3("MAIN: Received %d bytes: '%s'\n",numbytes,buf);
			}

		}

		//TODO: Do the multiple packet send with the header.

#ifndef TEST
		gettimeofday(&CurrentTime,NULL);
		if((TimeElapsed(&CurrentTime, &(A.tv)))>PACKETPERIOD_A){
			memcpy(PacketBuffer,"A",1);
			if (send(sockfd,PacketBuffer, A.length, 0) == -1)
				perror("send");
			DEBUG_PRINTF("MAIN: Packet A sent\n");
			A.tv=CurrentTime;
		}
		if((TimeElapsed(&CurrentTime, &(B.tv)))>PACKETPERIOD_B){
			memcpy(PacketBuffer,"B",1);
			if (send(sockfd,PacketBuffer, B.length, 0) == -1)
				perror("send");
			DEBUG_PRINTF("MAIN: Packet B sent\n");
			B.tv=CurrentTime;
		}
		if((TimeElapsed(&CurrentTime, &(C.tv)))>PACKETPERIOD_C){
			memcpy(PacketBuffer,"C",1);
			if (send(sockfd,PacketBuffer, C.length, 0) == -1)
				perror("send");
			DEBUG_PRINTF("MAIN: Packet C sent\n");
			C.tv=CurrentTime;
		}
#endif

	}

//	if (send(sockfd, "Hello,  sending world!", 13, 0) == -1)
//					perror("send");
//
//	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
//	    perror("recv");
//	    exit(1);
//	}
//
//	buf[numbytes] = '\0';
//
//	printf("client: received '%s'\n",buf);

	close(sockfd);

	return 0;
}

