/*
 * fog.cpp
 *
 *  Created on: Apr 14, 2014
 *      Author: Andrea Gil
 */


#include "/home/andrea/workspace/ECE640/common.h"

#define CLOUDSERVERPORT "5005"

#define FOGNAMELENGTH 8
#define SIZEDATACONTENT 8

using namespace std;

vector<struct FogDatabase> Database;

struct FogDatabase{
	char ip[INET6_ADDRSTRLEN];
	int socket;
	char datacontent[8];
	char name[1];
	int listeningport;
};

/*
 * Name: CreateFogDatabasePacket
 */
int CreateFogDatabasePacket(char *buf, int* numbytes){
	DEBUG_PRINTF("CreateFogDatabasePacket: Start\n");
	unsigned int i;
	int a=0;

	memset(buf,0,sizeof(buf));
	char header[]="DA";

	memcpy(buf+a,header,2);
	a+=2;

	for(i=0;i<Database.size();i++)
	{
		memcpy(buf+a,Database[i].ip,sizeof(Database[i].ip));
		a+=sizeof(Database[i].ip);

		memcpy(buf+a,Database[i].datacontent,sizeof(Database[i].datacontent));
		a+=sizeof(Database[i].datacontent);

		memcpy(buf+a,Database[i].name,sizeof(Database[i].name));
		a+=sizeof(Database[i].name);

	}

	*numbytes=a;
	DEBUG_PRINTF("CreateFogDatabasePacket: End\n");

	return 0;
}

/*
 * Name: AddHeader
 * Description:Make sure that buf is at least sizepacket +sizeof(header)
 */
int AddHeader(char buf[], int* sizepacket, char header)
{
	DEBUG_PRINTF("AddHeader: Start\n");
	char temp[MAXDATASIZE];
	temp[0]=header;
	memcpy(temp+1,buf,*sizepacket);
	DEBUG_PRINTF("AddHeader: End\n");
	return 0;
}

int SubscriberInformationRequestAction(int* socketfd, char *temp){
	DEBUG_PRINTF("SubscriberInformationRequestAction: Start\n");

	memset(temp,0,MAXDATASIZE);
	int a;
	CreateFogDatabasePacket(temp,&a);

	 if (send(*socketfd,temp,a,0)==-1){
		 perror("SubscriberInformationRequestAction: send");
	 }

	DEBUG_PRINTF2("SubscriberInformationRequestAction: temp:%s\n",temp);
	 DEBUG_PRINTF("SubscriberInformationRequestAction: End\n");
	 return 0;
}

int GetIPaddress(int *fd, char ipstr2[])
{
	DEBUG_PRINTF("GetIPaddress: Start\n");

	char ipstr[INET6_ADDRSTRLEN];
	socklen_t len;
	struct sockaddr_storage addr;
	int port;
	len = sizeof addr;
	getpeername(*fd, (struct sockaddr*)&addr, &len);


	// deal with both IPv4 and IPv6:
	if (addr.ss_family == AF_INET) {
		DEBUG_PRINTF("GetIPaddress: IPV4\n");
		struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		port = ntohs(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	} else { // AF_INET6
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		port = ntohs(s->sin6_port);
		inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
	}

	memcpy(ipstr2,ipstr,sizeof(ipstr));
	DEBUG_PRINTF2("GetIPaddress: Peer IP address: %s\n", ipstr2);
	DEBUG_PRINTF2("GetIPaddress: Peer port      : %d\n", port);
	DEBUG_PRINTF("GetIPaddress: End\n");

	return 0;
}

int CreateNewConnectionInformationPacket(unsigned int* i, char buf[], char ip[], int* counter){

	DEBUG_PRINTF("CreateNewConnectionInformationPacket: Start\n");
	char header[]="EA";

	memcpy(buf,header,2);
	*counter+=2;
	uint32_t a=htonl(Database[(*i)].listeningport);
	memcpy(buf+*counter,&a,sizeof(uint32_t));
	*counter+=sizeof(uint32_t);
	DEBUG_PRINTF2("CreateNewConnectionInformationPacket: sizeof(uint32_t): %d\n",sizeof(uint32_t));

	memcpy(buf+*counter,ip,INET6_ADDRSTRLEN);
	*counter+=INET6_ADDRSTRLEN;

	DEBUG_PRINTF("CreateNewConnectionInformationPacket: End\n");
	return 0;
}

int SuscriberInformationChoicesAction(int* socketfd, char* pbuf,int* numbytes){
	//TODO: NOte that the struct must include socket, IP, Area

	DEBUG_PRINTF("SuscriberInformationChoicesAction: Start\n");

	char ipstr[INET6_ADDRSTRLEN], *pipstr;
	char temp[MAXDATASIZE], *ptemp;
	pipstr=ipstr;
	ptemp=temp;
	memset(ipstr,0,INET6_ADDRSTRLEN);
	memset(temp,0,MAXDATASIZE);

	unsigned int i;
	int m=0;
	int p=2; //Two is because these are the two bytes that it uses for header.


	for (;p<*numbytes;p++){

		for (i=0;i<Database.size();i++)
		{
			if((Database[i].name[0]==pbuf[p])){
				DEBUG_PRINTF("SuscriberInformationChoicesAction: Comparison\n");
				GetIPaddress(socketfd,ipstr);
				DEBUG_PRINTF2("SuscriberInformationChoicesAction:IP address: %s\n",ipstr);
				DEBUG_PRINTF2("SuscriberInformationChoicesAction:size of IP address: %d\n",sizeof(ipstr));

				CreateNewConnectionInformationPacket(&i,temp,ipstr,&m);
				if (send(*socketfd,temp,m,0)==-1){ //TODO:IMPORTANT: Change *sockfd back to send(Database[i].socket
						 perror("SuscriberInformationChoicesAction: send");
					 }
			}
		}
	}

	DEBUG_PRINTF("SuscriberInformationChoicesAction: End\n");
	return 0;
}

int ForwardIncomingTraffic(int* sockfd, char buf[]){

	send(*sockfd,buf,sizeof(buf),0);
	return 0;
}

int ReceivedNewConnectionInfo(char buf[],vector<struct FogDatabase>* list){

	DEBUG_PRINTF("ReceivedNewConnectionInfo: Start\n");

	int a=1;
	uint32_t port;
	memset(&port,0,sizeof(uint32_t));
	struct FogDatabase info;
	memset(&info,0,sizeof(info));
	char temp[10];
	memset(temp,0,10);

	memcpy(info.name,buf+a,sizeof(info.name));
	DEBUG_PRINTF2("ReceivedNewConnectionInfo: name: %s\n",info.name);
	a+=sizeof(info.name);

	memcpy(info.datacontent,buf+a,sizeof(info.datacontent));
	DEBUG_PRINTF2("ReceivedNewConnectionInfo: content: %s\n",info.datacontent);
	a+=sizeof(info.datacontent);

	memcpy(info.ip,buf+a,sizeof(info.ip));
	DEBUG_PRINTF2("ReceivedNewConnectionInfo: IP: %s\n",info.ip);
	a+=sizeof(info.ip);

	memcpy(&port,buf+a,sizeof(uint32_t));
	info.listeningport=ntohl(port);
	DEBUG_PRINTF2("ReceivedNewConnectionInfo: port: %d\n",info.listeningport);
	a+=sizeof(uint32_t);

	list->push_back(info);
	DEBUG_PRINTF("ReceivedNewConnectionInfo: Info added to database\n");
	DEBUG_PRINTF("ReceivedNewConnectionInfo: End\n");
	return 0;
}


int main(void)
{

	//
//	struct FogDatabase temp2;
//	memset(&temp2,0, sizeof(temp2));
//	char name[]="Z";
//	memcpy(temp2.name,name,sizeof(name));
//	char ip[]="192.168.2.23";
//	memcpy(temp2.ip,ip,12);
//	temp2.listeningport=;
//	char content[]="HOLA";
//	memcpy(temp2.datacontent,content,4);
//	Database.push_back(temp2);
	//

	int sockfd, new_fd;
	char port[]=CLOUDSERVERPORT;

	ServerConnectionInit(&sockfd,port);

	vector<int> fd_list;
	fd_list.clear();

	struct timeval tv;
	fd_set fds, fds_back;
	tv.tv_sec=tv.tv_usec=0;

	FD_ZERO(&fds);
	FD_ZERO(&fds_back);
	FD_SET(sockfd,&fds_back);
	fds=fds_back;
	fd_list.push_back(sockfd);

	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	sin_size = sizeof their_addr;
	char s[INET6_ADDRSTRLEN];

	int numbytes;
	unsigned int i;
	char buf[MAXDATASIZE],*pbuf;
	pbuf=buf;
	char temp[MAXDATASIZE], *ptemp;
	ptemp=temp;

	int max_fd;
	max_fd=*max_element(fd_list.begin(),fd_list.end());

	DEBUG_PRINTF2("fd_list.back()= %d\n",fd_list.back());
	DEBUG_PRINTF2("Max_fd= %d\n",max_fd);

	while(1) {  // main accept() loop

		FD_ZERO(&fds);
		fds=fds_back;

		max_fd=*max_element(fd_list.begin(),fd_list.end());

		select(max_fd+1,&fds,NULL,NULL,&tv);

		if (FD_ISSET(sockfd,&fds)){

			DEBUG_PRINTF("Inside if\n");

			new_fd = (accept(sockfd, (struct sockaddr *)&their_addr, &sin_size));

			if (new_fd == -1) {
				perror("accept");
				//TODO: Check which are the consequences of having this continue here
				continue;
			}

			FD_SET(new_fd,&fds_back);
			fd_list.push_back(new_fd);

			inet_ntop(their_addr.ss_family,	get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
			printf("server: got connection from %s\n", s);

			for (i=0;i<fd_list.size();i++){
				DEBUG_PRINTF3("Socket %d: %d\n",i,fd_list[i]);
			}
		}

		for (i=1; i<fd_list.size();i++)
		{
			if (FD_ISSET(fd_list[i],&fds)){

				if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
					    perror("recv");
					    //exit(1);
					    //TODO: Remove this exits because it's causing that the server disconnects
					}
				if (numbytes==0){
					FD_CLR(fd_list[i],&fds_back);
					close(fd_list[i]);
					fd_list.erase(fd_list.begin()+i);
				}
				else {
					buf[numbytes] = '\0';
					DEBUG_PRINTF3("Cloud: received %d bytes, type: '%s'\n",numbytes,buf);

					switch(pbuf[0])
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
					case 'D': //Receive information request from FOG
						switch(pbuf[1])
						{
						case 'A': //Subscriber request Fog Database packet |D|A| (2 bytes)
							SubscriberInformationRequestAction(&fd_list[i],ptemp);
							break;
						case 'B': //Cloud receive choices from subscriber |D|B|List of choices:1byte each|
							SuscriberInformationChoicesAction(&fd_list[i],pbuf,&numbytes);
							break;
						}

						break;
					case 'E':
						ReceivedNewConnectionInfo(buf,&Database);


						break;
					default:
						DEBUG_PRINTF("Packet doesn't match with the standard cases\n");
					}
				}

			}
		}

	}
	return 0;
}

