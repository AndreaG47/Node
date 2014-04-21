/*
 * fog.cpp
 *
 *  Created on: Apr 14, 2014
 *      Author: Andrea Gil
 */

#include "common.h"
#define G

#ifdef G
#define CLOUDIP 		"192.168.1.2"
#define CLOUDSERVERPORT "5005"
#define NAME "G" //Name of the Fog device
#define DATACONTENT "FOG-G" // Mask showing the data content
#define IP "192.168.1.1"	//IP so other FOGs can connect to it
#define FOGLISTENINGPORT 6000 //FOGSERVERPORT ad FOGLISTENINGPORT have to be the same
#define FOGSERVERPORT "6000"
#define FOGSERVERPORT2 "7000"

#endif
#ifdef N
#define CLOUDIP 		"192.168.2.2"
#define CLOUDSERVERPORT "5005"
#define NAME "N" //Name of the Fog device
#define DATACONTENT "FOG-N" // Mask showing the data content
#define IP "192.168.2.1"	//IP so other FOGs can connect to it
#define FOGLISTENINGPORT 6000 //FOGSERVERPORT ad FOGLISTENINGPORT have to be the same
#define FOGSERVERPORT "6000"
#define FOGSERVERPORT2 "7000"
#endif

#ifdef M
#define CLOUDIP 		"192.168.3.2"
#define CLOUDSERVERPORT "5005"
#define NAME "M" //Name of the Fog device
#define DATACONTENT "FOG-M" // Mask showing the data content
#define IP "192.168.3.1"	//IP so other FOGs can connect to it
#define FOGLISTENINGPORT 6000 //FOGSERVERPORT ad FOGLISTENINGPORT have to be the same
#define FOGSERVERPORT "6000"
#define FOGSERVERPORT2 "7000"
#endif

#define FOGRATE		1 //minutes
#define CLOUDRATE	2 //minutes
#define WAITTIME  1
#define TEST

using namespace std;

struct FogDatabase{
	char ip[INET6_ADDRSTRLEN];
	int s;
	char datacontent[8];
	char name[1];
	int listeningport;
} example;

vector<struct FogDatabase> Database;

int RequestInformationfromotherFogs(int* Cloudfd){
	DEBUG_PRINTF("RequestInformationfromotherFogs: Start\n");

	char header[]="DA";
	send(*Cloudfd,header,2,0);
	DEBUG_PRINTF("RequestInformationfromotherFogs: End\n");
	return 0;
}

int RetrieveFogInfo(char buf[]){
	int i;
	struct FogDatabase temp;
	memset(&temp,0,sizeof(temp));

	Database.clear(); //TODO: Careful with this clear!!!

	for(i=2;i<sizeof(buf);)
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


		Database.push_back(temp);
	}


	return 0;

}

int SelectFogs(int* socketfd, char buf[]){

	DEBUG_PRINTF("SelectFogs: Start\n");

	char header[]="DB";
	int i=0;
	int numbytes=0;
	char temp[MAXDATASIZE];
	memset(temp,0,MAXDATASIZE);

	RetrieveFogInfo(buf);

	memcpy(temp+i,header,2);
	i+=2;
	for(int a=0;a<Database.size();a++){
		memcpy(temp+i,Database[a].name,1);
		i++;
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

	char *ip;
	ip=buf+sizeof(uint32_t)+2;
	DEBUG_PRINTF2("CreateNewFogConnection: IP: %s\n",ip);


	int fd;
	ClientConnectionInit(&fd,ip,port);
	FogFds->push_back(fd);

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
	DEBUG_PRINTF("Main:Start\n");

	int Cloudfd, sockfd, fogfd;
	char port[]=CLOUDSERVERPORT;
	char port2[]=FOGSERVERPORT;
	char port3[]=FOGSERVERPORT2;
	char CloudIP[]=CLOUDIP;


	//Start communication with Cloud
	DEBUG_PRINTF("Main: Start communication with Cloud\n");
	ClientConnectionInit(&Cloudfd,CloudIP,port);
	SendNewFogConnectionInfo(&Cloudfd);


	//Start server for nodes to connect to it
	DEBUG_PRINTF("Main: Start server for nodes\n");
	ServerConnectionInit(&sockfd,port2);

	//Start server for other fogs to connect to it
	DEBUG_PRINTF("Main: Start server for other fogs\n");
	ServerConnectionInit(&fogfd, port3);

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

	//TODO:Remove this after testing
	DEBUG_PRINTF2("Main: fd_list.back()= %d\n",fd_list.back());
	max_fd=*max_element(fd_list.begin(),fd_list.end());
	max_fdfog=*max_element(fdfog_list.begin(),fdfog_list.end());

	DEBUG_PRINTF2("Main: Max_fd= %d\n",max_fd);
	DEBUG_PRINTF2("Main: Max_fdfog= %d\n",max_fdfog);
	DEBUG_PRINTF("Main: Enter to main while loop\n");
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
#ifdef test
		if (FD_ISSET(sockfd,&fds)){  //Node accept new connections

			IncomingNewConnection(&sockfd,&fds_back,&fd_list);
		}

		for (i=1; i<fd_list.size();i++) { //Node existing connections
			if (FD_ISSET(fd_list[i],&fds)){

				memset(buf,0,sizeof(buf));
				numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0);

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
						CaseA(&fd_list[i],buf,&numbytes);
						break;
					case 'B':
						CaseB(&fd_list[i],buf,&numbytes);
						break;
					case 'C':
						CaseC(&fd_list[i],buf,&numbytes);
						break;
					default:
						DEBUG_PRINTF("Packet doesn't match with the standard cases\n");
					}
				}
			}
		}
#endif
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
				DEBUG_PRINTF3("Main: Cloud connections: Received %d bytes: '%s'\n",numbytes,buf);

				switch(buf[0])
				{
				case 'A':
					CaseA(&Cloudfd,buf,&numbytes);
					break;
				case 'B':
					CaseB(&Cloudfd,buf,&numbytes);
					break;
				case 'C':
					CaseC(&Cloudfd,buf,&numbytes);
					break;
				case 'D':
					switch (buf[1]) {
					case 'A': //Receive from cloud list of available fogs to retrieve info
						SelectFogs(&Cloudfd,buf);
						break;
					}
					break;
				case 'E':
					switch (buf[1]){
					case 'A': //Receive from cloud command to connect to other fog
						DEBUG_PRINTF("Main: Case EA\n");
						CreateNewFogConnection(buf,&fdfog_list);
						break;
					}
					break;
				default:
					DEBUG_PRINTF("Packet doesn't match with the standard cases\n");
				}
			}
		}

#ifdef TEST
		//Fogs

		if (FD_ISSET(fogfd,&fdsfog)){  //Node accept new connections

					IncomingNewConnection(&fogfd,&fdsfog_back,&fdfog_list);
				}

		for (i=0; i<fdfog_list.size();i++) { //Fog existing connections
			if (FD_ISSET(fdfog_list[i],&fdsfog)){

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
						//CaseA(&fd_list[i],buf,&numbytes);
						break;
					case 'B':
						//CaseB(&fd_list[i],buf,&numbytes);
						break;
					case 'C':
						//CaseC(&fd_list[i],buf,&numbytes);
						break;
					default:
						DEBUG_PRINTF("Packet doesn't match with the standard cases\n");
					}
				}
			}
		}
#endif

		//Send request to Cloud to get info about other fogs
		gettimeofday(&CurrentTime,NULL);

		if (ConnectionRequest==false){
			if((TimeElapsed(&CurrentTime, &StartTime))>10000000){
				DEBUG_PRINTF("Main: TimeElapsed\n");
				RequestInformationfromotherFogs(&Cloudfd);
				ConnectionRequest = true;
			}
		}



//		//Send data to Cloud
//		gettimeofday(&CurrentTime,NULL);
//		if((TimeElapsed(&CurrentTime, &CloudTime))>CLOUDRATE*1000000*60){
//			SendDatatoCloud(&Cloudfd);
//			gettimeofday(&CloudTime,NULL);
//		}
//
//		//Send data to Cloud
//		gettimeofday(&CurrentTime,NULL);
//		if((TimeElapsed(&CurrentTime, &FogTime))>FOGRATE*1000000*60){
//			SendDatatoFog(&fdfog_list);
//			gettimeofday(&CloudTime,NULL);
//		}

	}

	return 0;
}



