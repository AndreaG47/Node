/*
 * fog.cpp
 *
 *  Created on: Apr 14, 2014
 *      Author: Andrea Gil
 */
#define DEBUG

#include "common.h"
#define _STDC_FORMAT_MACROS
#include <inttypes.h>

#define G  //Name of the local fog, options: M,N,G. t is for testing, don't use it on GENI

#define CLOUD
#define CLOUDT
#define FOG

#define FORWARD_NODE
#define FORWARD_FOG


#ifdef t
#define CLOUDIP 		"localhost"
#define CLOUDSERVERPORT "5005"
#define NAME "N" //Name of the Fog device
#define DATACONTENT "FOG-T" // Mask showing the data content
#define IP "localhost"	//My IP, so other FOGs can connect to me
#define FOGLISTENINGPORT 7000 //FOGSERVERPORT ad FOGLISTENINGPORT2 have to be the same
#define FOGSERVERPORT "6000" //For node incoming connections
#define FOGSERVERPORT2 "7000" //For fog incoming connections
#define IP_FOG_G "localhost" //
#define IP_FOG_N "192.168.x.x"
#define IP_FOG_M "192.168.x.x"
#endif

//


#ifdef G
#define CLOUDIP 		"192.168.2.2"
#define CLOUDSERVERPORT "5005"
#define NAME "G" //Name of the Fog device
#define DATACONTENT "FOG-G" // Mask showing the data content
#define IP "192.168.2.1"	//IP so other FOGs can connect to it
#define FOGLISTENINGPORT 7000 //FOGSERVERPORT ad FOGLISTENINGPORT2 have to be the same
#define FOGSERVERPORT "6000" //For node connections
#define FOGSERVERPORT2 "7000" //For fog connections
#define IP_FOG_G "localhost"
#define IP_FOG_N "192.168.13.1"
#define IP_FOG_M "192.168.14.2"


#endif

#ifdef N
#define CLOUDIP 		"192.168.1.2"
#define CLOUDSERVERPORT "5005"
#define NAME "N" //Name of the Fog device
#define DATACONTENT "FOG-N" // Mask showing the data content
#define IP "192.168.1.1"	//IP so other FOGs can connect to it
#define FOGLISTENINGPORT 7000 //FOGSERVERPORT ad FOGLISTENINGPORT2 have to be the same
#define FOGSERVERPORT "6000"
#define FOGSERVERPORT2 "7000"
#define IP_FOG_G "192.168.13.2"
#define IP_FOG_N "localhost"
#define IP_FOG_M "192.168.10.2"
#endif

#ifdef M
#define CLOUDIP 		"192.168.3.1"
#define CLOUDSERVERPORT "5005"
#define NAME "M" //Name of the Fog device
#define DATACONTENT "FOG-M" // Mask showing the data content
#define IP "192.168.3.2"	//IP so other FOGs can connect to it
#define FOGLISTENINGPORT 7000 //FOGSERVERPORT ad FOGLISTENINGPORT2 have to be the same
#define FOGSERVERPORT "6000"
#define FOGSERVERPORT2 "7000"
#define IP_FOG_G "192.168.14.1"
#define IP_FOG_N "192.168.10.1"
#define IP_FOG_M "localhost"
#endif

//-----Modify Packet parameters----

#define PACKETPERIOD_A 4000000
#define PACKETPERIOD_B 4000000
#define PACKETPERIOD_C 4000000

#define PACKETsize_A 50
#define PACKETsize_B 50
#define PACKETsize_C 50

//------

using namespace std;

struct FogDatabase{
	char ip[INET6_ADDRSTRLEN];
	int s;
	char datacontent[8];
	char name[1];
	int listeningport;
} example;

vector<struct FogDatabase> Database;

int PacketGeneration(int *sockfd,struct timeval *TimeA, struct timeval* TimeB, struct timeval*TimeC)
{
	struct timeval CurrentTime;
	char PacketBuffer[MAXPACKETSIZE];
	memset(PacketBuffer,5,MAXPACKETSIZE); //fill buffer with any data.

	gettimeofday(&CurrentTime,NULL);
	if((TimeElapsed(&CurrentTime, TimeA))>PACKETPERIOD_A){
		memcpy(PacketBuffer,"A",1);
		memcpy(PacketBuffer+1,&CurrentTime,sizeof(CurrentTime));

		if ((send(*sockfd,PacketBuffer, PACKETsize_A, 0)) == -1)
			perror("send");
		DEBUG_PRINTF2("PacketGeneration:Packet A sent to socket %d\n",*sockfd);
		*TimeA=CurrentTime;
	}
//	if((TimeElapsed(&CurrentTime, TimeB))>PACKETPERIOD_B){
//		memcpy(PacketBuffer,"B",1);
//		memcpy(PacketBuffer+1,&CurrentTime,sizeof(CurrentTime));
//		if (send(*sockfd,PacketBuffer, PACKETsize_B, 0) == -1)
//			perror("send");
//		DEBUG_PRINTF("MAIN: Packet B sent\n");
//		*TimeB=CurrentTime;
//	}
//	if((TimeElapsed(&CurrentTime, TimeC))>PACKETPERIOD_C){
//		memcpy(PacketBuffer,"C",1);
//		memcpy(PacketBuffer+1,&CurrentTime,sizeof(CurrentTime));
//		if (send(*sockfd,PacketBuffer, PACKETsize_C, 0) == -1)
//			perror("send");
//		DEBUG_PRINTF("MAIN: Packet C sent\n");
//		*TimeC=CurrentTime;
//	}

	return 0;
}

int ForwardPacket(char buf[], int *fd, int *size)
{
	DEBUG_PRINTF("ForwardPacket: Start\n");
	timeval now;
	memcpy(&now,buf+1,sizeof(now));
	DEBUG_PRINTF3("time = %u.%06u\n",now.tv_sec, now.tv_usec);
	DEBUG_PRINTF2("ForwardPacket: Size to send= %d\n",*size);
	int numbytes;
	if((numbytes=send(*fd,buf,*size,0))==-1)
		DEBUG_PRINTF("ForwardPacket: Send error\n");
	DEBUG_PRINTF2("ForwardPacket: Size sent= %d\n",numbytes);

	DEBUG_PRINTF("ForwardPacket: End\n");
	return 0;
}

int RequestInformationfromotherFogs(int* Cloudfd){
	DEBUG_PRINTF("RequestInformationfromotherFogs: Start\n");

	char header[]="DA";
	send(*Cloudfd,header,2,0);
	DEBUG_PRINTF("RequestInformationfromotherFogs: End\n");
	return 0;
}

int RetrieveFogInfo(char buf[],int *n){
	DEBUG_PRINTF("RetrieveFogInfo: Start\n");
	int i;
	struct FogDatabase temp;
	memset(&temp,0,sizeof(temp));

	Database.clear(); //TODO: Careful with this clear!!!

	for(i=2;i<*n;)
	{
		memcpy(temp.ip,buf+i,sizeof(temp.ip));
		i+=INET6_ADDRSTRLEN;
		DEBUG_PRINTF2("RetrieveFogInfo: ip: %s\n",temp.ip);

		memcpy(temp.datacontent,buf+i,sizeof(temp.datacontent));
		i+=sizeof(temp.datacontent);
		DEBUG_PRINTF2("RetrieveFogInfo: datacontent: %s\n",temp.datacontent);

		memcpy(temp.name,buf+i,sizeof(temp.name));
		i+=sizeof(temp.name);
		DEBUG_PRINTF2("RetrieveFogInfo: name: %s\n",temp.name);

		DEBUG_PRINTF2("RetrieveFogInfo: byte number %d\n",i);


		Database.push_back(temp);
	}

	DEBUG_PRINTF("RetrieveFogInfo: End\n");
	return 0;

}

int SelectFogs(int* socketfd, char buf[],int* n){

	DEBUG_PRINTF("SelectFogs: Start\n");

	char header[]="DB";
	int i=0;
	int numbytes=0;
	char temp[MAXDATASIZE];
	memset(temp,0,MAXDATASIZE);

	char name[1];
	memset(name,0,1);
	memcpy(name,NAME,1);


	RetrieveFogInfo(buf,n);

	memcpy(temp+i,header,2);
	i+=2;
	memcpy(temp+i,name,1);
	i+=1;

	for(int a=0;a<Database.size();a++){
		if ((Database[a].name)[0]!=name[0]){
			DEBUG_PRINTF2("SelectFogs: Add %s to list of selected fogs\n",Database[a].name);
			memcpy(temp+i,Database[a].name,1);
			i++;
		}

	}
	numbytes=i;
	DEBUG_PRINTF2("SelectFogs: Packet to send: %s\n",temp);

	if (send(*socketfd,temp,numbytes,0)==-1)
		perror("SelectFogs: Send error\n");

	DEBUG_PRINTF("SelectFogs: End\n");

	return 0;
}

int CreateNewFogConnection(char buf[], vector<int> *FogFds ){

	DEBUG_PRINTF("CreateNewFogConnection: Start\n");
	uint32_t a;
	memcpy(&a,buf+2,sizeof(uint32_t));
	int portint=ntohl(a);
	char port[20];
	memset(port,0,sizeof(port));
	sprintf(port,"%d",portint);

	DEBUG_PRINTF3("CreateNewFogConnection: Port int: %d,  port char %s:\n",portint,port);

//	char *ip;
//	ip=buf+sizeof(uint32_t)+2;
//	DEBUG_PRINTF2("CreateNewFogConnection: IP: %s\n",ip);


	char name[2];
	memset(name,0,2);
	name[1]='\0';
	int fd;
	char ip[INET6_ADDRSTRLEN];

	DEBUG_PRINTF2("CreateNewFogConnection: Sizeof name %d\n",sizeof(name));
	DEBUG_PRINTF2("CreateNewFogConnection: Sizeof NAME %d\n",sizeof(NAME));

	memcpy(name,buf+sizeof(uint32_t)+2,1);

	switch (name[0]){
	case 'G':
		memcpy(ip,IP_FOG_G,INET6_ADDRSTRLEN);
		break;
	case 'M':
		memcpy(ip,IP_FOG_M,INET6_ADDRSTRLEN);
		break;
	case 'N':
		memcpy(ip,IP_FOG_N,INET6_ADDRSTRLEN);
		break;
	}
	DEBUG_PRINTF4("CreateNewFogConnection: Connect to %s : ip: %s, port %s\n",name,ip,port);
	ClientConnectionInit(&fd,ip,port);
	FogFds->push_back(fd);
	DEBUG_PRINTF2("CreateNewFogConnection: socket %d added",fd);
	DEBUG_PRINTF("CreateNewFogConnection: End\n");
	return 0;
}

int IncomingNewConnection(int *sockfd, fd_set *fds_back, vector<int> *fd_list){

	DEBUG_PRINTF("IncomingNewConnection: Start\n");
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	sin_size = sizeof their_addr;
	char s[INET6_ADDRSTRLEN];
	int new_fd;
	unsigned int i;

	new_fd = (accept(*sockfd, (struct sockaddr *)&their_addr, &sin_size));

	if(new_fd!=-1){

		//Add new connection socket to file descriptor set and fd list.
		FD_SET(new_fd,fds_back);
		fd_list->push_back(new_fd);

		//TODO: Here you should store the ip address information. Should I store their_addr?
		inet_ntop(their_addr.ss_family,	get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
		DEBUG_PRINTF2("IncomingNewConnection: got connection from %s\n", s);

		//TODO: Remove this after testing
		for (i=0;i<fd_list->size();i++){
			DEBUG_PRINTF3("IncomingNewConnection: Socket %d: %d\n",i,fd_list->at(i));
		}
	}
	else if (new_fd == -1) {
		perror("IncomingNewConnection: accept");
	}

	DEBUG_PRINTF2("IncomingNewConnection: Socket %d\n",new_fd);
	DEBUG_PRINTF("IncomingNewConnection: End\n");
	return 0;
}

int SendDatatoCloud(int* fd){
	return 0;
}

int SendDatatoFog(vector<int> *list){
	unsigned int i;
	char temp[MAXDATASIZE];

	for(i=0; i<list->size();i++){
		send(list->at(i),temp,10,0);

	}
	return 0;
}

int SendNewFogConnectionInfo(int* fd){

	DEBUG_PRINTF("SenNewFogConnectionInfo: Start\n");

	char info[MAXDATASIZE];
	memset(info,0, MAXDATASIZE);
	int a=0;
	uint32_t port = htonl(FOGLISTENINGPORT);


	memcpy(info+a,"E",1);
	DEBUG_PRINTF2("SendNewFogConnectionInfo: header: %s\n",info+a);
	a+=1;

	memcpy(info+a,NAME,sizeof(example.name));
	DEBUG_PRINTF2("SendNewFogConnectionInfo: name: %s\n",info+a);
	a+=sizeof(example.name);

	memcpy(info+a,DATACONTENT,sizeof(example.datacontent));
	DEBUG_PRINTF2("SendNewFogConnectionInfo: datacontent: %s\n",info+a);
	a+=sizeof(example.datacontent);

	memcpy(info+a,IP,sizeof(example.ip));
	DEBUG_PRINTF2("SendNewFogConnectionInfo: ip: %s\n",info+a);
	a+=sizeof(example.ip);

	memcpy(info+a,&port,sizeof(uint32_t)); //TODO: This 10 is because I don't have other way to define the length of port.
	DEBUG_PRINTF2("SendNewFogConnectionInfo: port: %d\n",ntohl(port));
	a+=sizeof(uint32_t);

	DEBUG_PRINTF2("SendNewFogConnectionInfo: numbytes: %d\n",a);

	send(*fd,info,a,0);
	DEBUG_PRINTF("SendNewFogConnectionInfo: Sent packet");

	DEBUG_PRINTF("SenNewFogConnectionInfo: End\n");

	return 0;
}

int main(void)
{
	//freopen("Output_Fog.txt","w",stdout);
	//freopen("Error_Fog.txt","w",stderr);

	DEBUG_PRINTF("Main:Start\n");

	int Cloudfd, sockfd, fogfd, new_fd;
	char port[]=CLOUDSERVERPORT;
	char port2[]=FOGSERVERPORT;
	char port3[]=FOGSERVERPORT2;
	char CloudIP[]=CLOUDIP;


	//Start communication with Cloud
#ifdef CLOUD
	DEBUG_PRINTF("Main: Start communication with Cloud\n");
	ClientConnectionInit(&Cloudfd,CloudIP,port);
	SendNewFogConnectionInfo(&Cloudfd);
#endif
#ifndef CLOUD
	Cloudfd=0;
#endif

	//Start server for nodes to connect to it
#ifdef NODE
	DEBUG_PRINTF("Main: Start server for nodes\n");
	ServerConnectionInit(&sockfd,port2);
#endif
#ifndef NODE
	sockfd=0;
#endif

#ifdef FOG
	//Start server for other fogs to connect to it
	DEBUG_PRINTF("Main: Start server for other fogs\n");
	ServerConnectionInit(&fogfd, port3);
#endif
#ifndef FOG
	fogfd=0;
#endif

	bool ConnectionRequest = false;
	struct timeval tv;
	tv.tv_sec=tv.tv_usec=0;

	vector<int> fd_list, fdfog_list;
	fd_list.clear();
	fd_set fds, fds_back, fdscloud, fdscloud_back, fdsfog, fdsfog_back;


	//Add sockets to file descriptor set and to vector of fd.
	FD_ZERO(&fds_back);
	FD_ZERO(&fdscloud_back);
	FD_ZERO(&fdsfog_back);

	FD_SET(sockfd,&fds_back);
	FD_SET(Cloudfd,&fdscloud_back);
	FD_SET(fogfd,&fdsfog_back);

	fd_list.push_back(sockfd);
	fdfog_list.push_back(fogfd);

	char buf[MAXDATASIZE];
	int numbytes, max_fd, max_fdfog, i;

	struct timeval CurrentTime, StartTime, CloudTime, FogTime;
	gettimeofday(&CurrentTime,NULL);
	gettimeofday(&StartTime,NULL);
	CloudTime.tv_sec=CloudTime.tv_usec=0;
	FogTime.tv_sec=FogTime.tv_usec=0;

	struct timeval TimeA, TimeB, TimeC, RecvTime;
	TimeA=TimeB=TimeC=RecvTime=CloudTime;
	uint64_t elapsed;

	//TODO:Remove this after testing
	DEBUG_PRINTF2("Main: fd_list.back()= %d\n",fd_list.back());
	max_fd=*max_element(fd_list.begin(),fd_list.end());
	max_fdfog=*max_element(fdfog_list.begin(),fdfog_list.end());

	DEBUG_PRINTF2("Main: Max_fd= %d\n",max_fd);
	DEBUG_PRINTF2("Main: Max_fdfog= %d\n",max_fdfog);
	DEBUG_PRINTF("Main: Enter to main while loop\n");

	bool sendtofog=false;


	//
	while(1) {  // main accept() loop


		FD_ZERO(&fds);
		FD_ZERO(&fdsfog);
		FD_ZERO(&fdscloud);

		fds=fds_back;
		fdsfog=fdsfog_back;
		fdscloud=fdscloud_back;

		max_fd=*max_element(fd_list.begin(),fd_list.end());
		max_fdfog=*max_element(fdfog_list.begin(),fdfog_list.end());

		select(max_fd+1,&fds,NULL,NULL,&tv);
		select(max_fdfog+1,&fdsfog,NULL,NULL,&tv);
		select(Cloudfd+1,&fdscloud,NULL,NULL,&tv);

		//Nodes
#ifdef NODE
		if (FD_ISSET(sockfd,&fds)){  //Node accept new connections

			DEBUG_PRINTF("Main: New Node incoming connection\n");
			IncomingNewConnection(&sockfd,&fds_back,&fd_list);
		}

		for (i=1; i<fd_list.size();i++) { //Node existing connections
			if (FD_ISSET(fd_list[i],&fds)){

				DEBUG_PRINTF2("Main: Node existing connection: fd_list[i]=%d\n",fd_list[i]);
				memset(buf,0,sizeof(buf));
				numbytes = recv(fd_list[i], buf, MAXDATASIZE-1, 0);

				if (numbytes == -1) {
					    perror("Main: Node existing connections: recv");
					    //exit(1); //TODO: Remove this exits because it's causing that the server disconnects
					}
				if (numbytes==0){
					//Close socket and remove from set and list
					FD_CLR(fd_list[i],&fds_back);
					close(fd_list[i]);
					fd_list.erase(fd_list.begin()+i);
				}
				else {
					//Actions for packet received from node existing connections

					//TODO: Remove this after testing
					buf[numbytes] = '\0';
					DEBUG_PRINTF3("Main: Node existing connection: Received %d bytes: '%s'\n",numbytes,buf);
					//
					switch(buf[0])
					{
					case 'A':
#ifdef FORWARD_NODE
						ForwardPacket(buf,&fd_list[i],&numbytes);
#endif
						break;
					case 'B':
	//					ForwardPacket(buf,&fd_list[i],&numbytes);
						//CaseB(&fd_list[i],buf,&numbytes);
						break;
					case 'C':
		//				ForwardPacket(buf,&fd_list[i],&numbytes);
						//CaseC(&fd_list[i],buf,&numbytes);
						break;
					default:
						DEBUG_PRINTF("Packet from Node doesn't match with the standard cases\n");
					}
				}
			}
		}
#endif

#ifdef CLOUD
		//Cloud
		if (FD_ISSET(Cloudfd,&fdscloud)){ //Cloud connection

			memset(buf,0,sizeof(buf));
			numbytes = recv(Cloudfd, buf, MAXDATASIZE-1, 0);

			if (numbytes == -1) {
				    perror("Main:Cloud connection: recv");
				    //exit(1); //TODO: Remove this exits because it's causing that the server disconnects
				}
			if (numbytes==0){
				close(Cloudfd);
			}
			else {
				buf[numbytes] = '\0';
				DEBUG_PRINTF3("Main: Cloud incoming connections: Received %d bytes: '%s'\n",numbytes,buf);

				switch(buf[0])
				{
				case 'A':
					DEBUG_PRINTF3("MAIN: Received %d bytes: '%s'\n",numbytes,buf);
					memcpy(&RecvTime,buf+1,sizeof(RecvTime));
					DEBUG_PRINTF3("Main: RecvTime = %u.%06u\n",RecvTime.tv_sec, RecvTime.tv_usec);
					gettimeofday(&CurrentTime,NULL);
					DEBUG_PRINTF3("Main: CurrentTime = %u.%06u\n",CurrentTime.tv_sec, CurrentTime.tv_usec);
					elapsed=TimeElapsed(&CurrentTime, &RecvTime);
					printf("Main: Elapsed : %u us\n", elapsed);

					break;
//				case 'B':
//					CaseB(&Cloudfd,buf,&numbytes);
//					break;
//				case 'C':
//					CaseC(&Cloudfd,buf,&numbytes);
//					break;
				case 'D':
					switch (buf[1]) {
					case 'A': //Receive from cloud list of available fogs to retrieve info
						SelectFogs(&Cloudfd,buf,&numbytes);
						break;
					}
					break;
				case 'E':
					switch (buf[1])
					{
					case 'A': //Receive from cloud command to connect to other fog
						DEBUG_PRINTF("Main: Fog: Case EA\n");
						CreateNewFogConnection(buf,&fdfog_list);
						DEBUG_PRINTF2("Main: Fog: New socket %d\n",(fdfog_list.back()));
						DEBUG_PRINTF("fdfog_list:\n");
						for(i=0;i<fdfog_list.size();i++)
							DEBUG_PRINTF2("\t%d\n",fdfog_list[i]);
						sendtofog=true;
						break;
					}
					break;
				default:
					DEBUG_PRINTF("Main: Cloud fd: Packet doesn't match with the standard cases\n");
				}
			}
		}

#endif

#ifdef FOG
		//Fogs

		if (FD_ISSET(fogfd,&fdsfog)){  //Node accept new connections
					DEBUG_PRINTF("Main: New incoming fog connection");
					IncomingNewConnection(&fogfd,&fdsfog_back,&fdfog_list);
					//sendtofog=true;
					DEBUG_PRINTF("fdfog_list:\n");
					for(i=0;i<fdfog_list.size();i++){
						DEBUG_PRINTF2("\t%d\n",fdfog_list[i]);
					}
					DEBUG_PRINTF("Main: Continue\n");

				}

		if((FD_ISSET(5,&fdsfog))){
			DEBUG_PRINTF("TEST\n");
		}



		for (i=0; i<fdfog_list.size();i++) { //Fog existing connections
			if (FD_ISSET(fdfog_list[i],&fdsfog)){
				DEBUG_PRINTF2("Fog existing connections: socket %d \n",fdfog_list[i]);

				memset(buf,0,sizeof(buf));
				numbytes = recv(fdfog_list[i], buf, MAXDATASIZE-1, 0);

				if (numbytes == -1) {
					    perror("Main: Fog existing connections: recv");
					    //exit(1);
					    //TODO: Remove this exits because it's causing that the server disconnects
					}
				else if (numbytes==0){
					//Close socket and remove from set and list
					FD_CLR(fdfog_list[i],&fdsfog);
					close(fdfog_list[i]);
					fdfog_list.erase(fdfog_list.begin()+i);
				}
				else {
					//Actions for packet received from fog existing connections

					//TODO: Remove this after testing
					buf[numbytes] = '\0';
					DEBUG_PRINTF3("Main: Fog existing connection: Received %d bytes: '%s'\n",numbytes,buf);
					//
					switch(buf[0])
					{
					case 'A':
#ifdef FORWARD_FOG
						ForwardPacket(buf,&(fdfog_list[i]),&numbytes);
#endif
						break;
//					case 'B':
//						//CaseB(&fd_list[i],buf,&numbytes);
//						break;
//					case 'C':
//						//CaseC(&fd_list[i],buf,&numbytes);
//						break;
					default:
						DEBUG_PRINTF("Main: Fog connections: Packet doesn't match with the standard cases\n");
					}
				}
			}
		}
#endif

#ifdef CLOUDT

		//Send request to Cloud to get info about other fogs
		gettimeofday(&CurrentTime,NULL);

		if (ConnectionRequest==false){
			if((TimeElapsed(&CurrentTime, &StartTime))>25000000){
				DEBUG_PRINTF("Main: TimeElapsed\n");
				RequestInformationfromotherFogs(&Cloudfd);
				ConnectionRequest = true;
			}
		}

#endif

#ifdef CLOUD
//			PacketGeneration(&Cloudfd,&TimeA,&TimeB,&TimeC);
#endif


#ifdef FOG
			if (sendtofog && (fdfog_list.size()>1)){
				for (i=1;i<fdfog_list.size();i++){
					PacketGeneration(&(fdfog_list[i]),&TimeA,&TimeB,&TimeC);

				}
			}
#endif

	}

	return 0;
}



