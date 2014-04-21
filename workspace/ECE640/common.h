/*
 * common.h
 *
 *  Created on: Apr 7, 2014
 *      Author: andrea
 */

#ifndef COMMON_H_
#define COMMON_H_


#endif /* COMMON_H_ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <vector>
#include <algorithm>
#include <sys/time.h>


#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXDATASIZE 1000

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(x)			printf(x)
#define DEBUG_PRINTF2(x,y)		printf(x,y)
#define DEBUG_PRINTF3(x,y,z)	printf(x,y,z)
#define DEBUG_PRINTF4(w,x,y,z)	printf(w,x,y,z)
#endif
#ifndef DEBUG
#define DEBUG_PRINTF(x)
#define DEBUG_PRINTF2(x,y)
#define DEBUG_PRINTF3(x,y,z)
#define DEBUG_PRINTF4(w,x,y,z)
#endif

 void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

 void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

 int CaseA(int* socket, char buf[], int *numbytes){
//TODO:HEre.
	 DEBUG_PRINTF4("Case A: Socket %d received %d bytes: '%s'\n",*socket,*numbytes,buf);

	 if (send(*socket,buf,*numbytes,0)==-1){
	 		 perror("SubscriberInformationRequestAction: send");
	 	 }
	 DEBUG_PRINTF4("Case A: Socket %d SEND %d bytes: '%s'\n",*socket,*numbytes,buf);

	return 0;
}

 int CaseB(int* socket, char buf[], int *numbytes)
{
	 DEBUG_PRINTF4("Case A: Socket %d received %d bytes: '%s'\n",*socket,*numbytes,buf);

	return 0;

}

 int CaseC(int* socket, char buf[], int *numbytes)
{
	 DEBUG_PRINTF4("Case A: Socket %d received %d bytes: '%s'\n",*socket,*numbytes,buf);

	 return 0;

}

 //TODO: Poner el port and ip address as inputs
 int ServerConnectionInit (int* sockfd, char port[]){

 	DEBUG_PRINTF("ServerConnectionInit: Start\n");
 	struct addrinfo hints, *servinfo, *p;
 	struct sigaction sa;
 	int yes=1;
 	int rv;
 	memset(&hints, 0, sizeof hints);
 	hints.ai_family = AF_UNSPEC;
 	hints.ai_socktype = SOCK_STREAM;
 	hints.ai_flags = AI_PASSIVE; // use my IP

 	if ((rv = getaddrinfo(NULL, port,  &hints, &servinfo)) != 0) {
 		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
 		return 1;
 	}

 	// loop through all the results and bind to the first we can
 	for(p = servinfo; p != NULL; p = p->ai_next) {
 		if ((*sockfd = socket(p->ai_family, p->ai_socktype,
 				p->ai_protocol)) == -1) {
 			perror("ServerConnectionInit: socket");
 			continue;
 		}

 		if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
 				sizeof(int)) == -1) {
 			perror("ServerConnectionInit: setsockopt");
 			exit(1);
 		}

 		if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
 			close(*sockfd);
 			perror("ServerConnectionInit: bind");
 			continue;
 		}

 		break;
 	}

 	if (p == NULL)  {
 		fprintf(stderr, "server: failed to bind\n");
 		return 2;
 	}

 	freeaddrinfo(servinfo); // all done with this structure

 	if (listen(*sockfd, BACKLOG) == -1) {
 		perror("listen");
 		exit(1);
 	}

 	sa.sa_handler = sigchld_handler; // reap all dead processes
 	sigemptyset(&sa.sa_mask);
 	sa.sa_flags = SA_RESTART;
 	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
 		perror("sigaction");
 		exit(1);
 	}

 	printf("ServerConnectionInit: Waiting for connections.\n");
 	DEBUG_PRINTF("ServerConnectionInit: End\n");
 	return 0;
 }

 int ClientConnectionInit(int* sockfd, char ip[], char port[]){
 	DEBUG_PRINTF("ClientConnectionInit: Start\n");

 	struct addrinfo hints, *servinfo, *p;
 	int rv;
 	char s[INET6_ADDRSTRLEN];

 	memset(&hints, 0, sizeof hints);
 	hints.ai_family = AF_UNSPEC;
 	hints.ai_socktype = SOCK_STREAM;

 	if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
 		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
 		return 1;
 	}

 	// loop through all the results and connect to the first we can
 	DEBUG_PRINTF("ClientConnectionInit: Socket creation and connect function\n");
 	for(p = servinfo; p != NULL; p = p->ai_next) {
 		if ((*sockfd = socket(p->ai_family, p->ai_socktype,
 				p->ai_protocol)) == -1) {
 			perror("client: socket");
 			continue;
 		}

 		if (connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
 			close(*sockfd);
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
 	printf("Cloud Connection: connecting to Cloud %s\n", s);

 	freeaddrinfo(servinfo); // all done with this structure
 	DEBUG_PRINTF("ClientConnectionInit: End\n");


 	return 0;
 }

 uint64_t TimeElapsed(struct timeval* current, struct timeval* tv){
 	uint64_t a;
 	a = (current->tv_sec-tv->tv_sec)*1000000+(current->tv_usec-tv->tv_usec);
 	return a;
 }

