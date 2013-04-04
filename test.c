#include "csapp.h"

int main(int argc, char **argv)
{
	int port = atoi(argv[1]);
	int new_fd;
	struct sockaddr_in clientaddr;
	socklen_t clientlen;
	int result;
	char *node = "www.msn.com";
	char *serv = "http";
	struct addrinfo hints;
	struct addrinfo *servinfo;

	//prepare hints
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	//wait for connection
	int listenfd = open_listenfd(port);
	//accept incoming connection
	new_fd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
	
	//TODO CLIENT
	//does all kinds of good stuff for us
	result = getaddrinfo(node, serv, &hints, &servinfo);
	//
	if(result != 0)
	{
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(result));
		exit(1);
	}
	printf("result = %d\n", result);
	int sockfd;
	struct addrinfo *p;
	for(p = servinfo; p!= NULL; p = p->ai_next)
	{
		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(sockfd == -1)
		{
			perror("socket");
			continue;
		}
		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("connect");
			continue;
		}
		break;
	}
	//we are not connected
	if(p == NULL)
	{
		fprintf(stderr, "failed to connect\n");
		exit(2);
	}
	//we are connected and we want to read from the website
	//send request to server
	char *message = "GET /?st=1 HTTP/1.1\r\nHost: www.msn.com\r\n\r\n";
	int sendResult = send(sockfd, message, strlen(message), 0);
	if(sendResult < 0)
	{
		printf("SEND FAAAAAILED\n");
		return 1;
	}
	printf("Data Send\n");
	
	//receive data from server
	char data[108222];
	int recvResult = recv(sockfd, data, 108222, 0);
	if(recvResult < 0)
	{
		printf("recv FAAAILED!!shift+1\n");
	}
	printf("Data Received\n%s", data);

	//free the servinfo
	freeaddrinfo(servinfo);
	//TODO END CLIENT
	
	//TODO SEND to new_fd
	//TODO END SEND to new_fd
	
	//close connection
	close(new_fd);

	return 0;
}

int recv_timeout(int s , int timeout) {     int size_recv , total_size= 0;     struct timeval begin , now;     char chunk[CHUNK_SIZE];     double timediff;           //make socket non blocking     fcntl(s, F_SETFL, O_NONBLOCK);           //beginning time     gettimeofday(&begin , NULL);           while(1)     {         gettimeofday(&now , NULL);                   //time elapsed in seconds         timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);                   //if you got some data, then break after timeout         if( total_size > 0 && timediff > timeout )         {             break;         }                   //if you got no data at all, wait a little longer, twice the timeout         else if( timediff > timeout*2)         {             break;         }                   memset(chunk ,0 , CHUNK_SIZE);  //clear the variable         if((size_recv =  recv(s , chunk , CHUNK_SIZE , 0) ) < 0)         {             //if nothing was received then we want to wait a little before trying again, 0.1 seconds             usleep(100000);         }         else        {             total_size += size_recv;             printf("%s" , chunk);             //reset beginning time             gettimeofday(&begin , NULL);         }     }           return total_size; } 

