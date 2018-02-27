//**************************************************************************
// Client.c
// CSCE 3530 Fall 2017 UNT
// C program: Client connects to Server using TCP connection. User inputs
//            url, which is sent to server. Server sends back request webpage
// by Miguel Melendez 10/04/17
//***************************************************************************

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
int main(int argc,char *argv[])
{
	int sockfd, n;
    	int len = sizeof(struct sockaddr);
    	char recvline[40960];
    	struct sockaddr_in serv_addr;
	char input[200];
	int port_num;

	if(argc < 2)
	{
		printf("ERROR: Usage format: ./a.out <port_number>\n");
		return 0;
	}
   	 /* AF_INET - IPv4 IP , Type of socket, protocol*/
    	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("ERROR opening socket");
		exit(1);
	}
    	bzero(&serv_addr,sizeof(serv_addr));
 	port_num = atoi(argv[1]);
	serv_addr.sin_family=AF_INET;
    	serv_addr.sin_port=htons(port_num); // Server port number
 
   	 /* Convert IPv4 and IPv6 addresses from text to binary form */
	inet_pton(AF_INET,"129.120.151.94",&(serv_addr.sin_addr));
 
   	 /* Connect to the server */
    	if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
	{
		perror("ERROR connecting");
		exit(1);
	}
 
	// user input
	printf("url: ");
	scanf("%s", input);

	// send input url to server
	n = write(sockfd, input, strlen(input));
	if(n < 0)
	{
		perror("ERROR writing to server");
		exit(1);
	}

	// receives data from server & prints to screen
    	while(n = read(sockfd, recvline, sizeof(recvline)) > 0)
    	{
        	printf("%s",recvline); // print the received text from server
   	 }
 
}
