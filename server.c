//***********************************************************************
// server.c
// CSCE 3530 Fall 2017 UNT
// C program: Server allows connection from client & receives url input
// 	      from client. Server gets ip address of URL & connects
//            to webserver. Server then sends get request & receives
//	      webpage & stores it in buffer before sending it to the 
// 	      client.  
// by Miguel Melendez 10/04/17
//**********************************************************************



#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <stdbool.h> // use bool
#include <time.h> // get current time

#define h_addr h_addr_list[0]

char cached_time[15];
char* http_response = "200 OK";

bool url_in_file(char* input_url)
{
	char url[20];
	//char cached_time[15];
	char* cached_timeptr;

	FILE* fileptr;
//	printf("opening in url_in_file\n");
	fileptr = fopen("list.txt", "a+");
	if(fileptr == NULL)
	{
		printf("ERROR opeing file\n");
		
	} 
	
	while(!feof(fileptr))
	{
		fscanf(fileptr, "%s %s", url, cached_time);
		int n = strcmp(input_url, url);
		if( n == 0)
		{
			return true;
		}
	}
	fclose(fileptr);
	
	return false;	
}


bool blacklisted(char* input_url)
{
	long long int start_time = 0;
	long long int end_time = 0;
	char url[20];
	char* urlptr = url;
	char* current_timeString;
	time_t current_time;
	double seconds;
	struct tm *info;
	char buff[80];
	long long int now_time;

	FILE* fileptr;
	

//	printf("opening in blacklisted\n");
	fileptr = fopen("blacklist.txt", "r");
	if(fileptr == NULL)
	{
		printf("Error opening file\n");
	}
	
	while(!feof(fileptr))
	{
		
		fscanf(fileptr, "%s %lld %lld", url, &start_time, &end_time);

		int n = strcmp(input_url, urlptr);
		if(n == 0)
		{
			// get current time
			time( &current_time );
			info = localtime( &current_time );	
			strftime(buff, 80, "%Y%m%d%H%M%S", info);
			now_time = atoll(buff);
			if(now_time > start_time)
			{
				if(now_time < end_time)
				{
					//printf("return true\n");
					return true;
				}
			}
			//printf("return false\n");
			return false;
		}
	}
	fclose(fileptr);
	

}


int main(int argc, char *argv[])
{
	char recvline[200];
    	int sockfd, newsockfd, port_num, cli_size;
   	struct sockaddr_in serv_addr, cli_addr;
	int n;
	char* recvlineptr = recvline;
	char* url_in_file_return;

	char current_time_char[80];
	time_t current_time;
	struct tm *info;

	// websever declarations
	char* web_ip;
	int websocket, m;

	struct sockaddr_in web_servaddr;
	char buffer[4000];
	
	struct hostent *he;
	char request[] = "GET /index.html\r\n\r\n";

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
	
	port_num = atoi(argv[1]);
   	bzero(&serv_addr, sizeof(serv_addr)); 
   	serv_addr.sin_family = AF_INET;
    	serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
    	serv_addr.sin_port = htons(port_num);
 
    	/* Binds the above details to the socket */
	if(bind(sockfd,  (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR binding");
		exit(1);
	}

	/* Start listening to incoming connections */
	listen(sockfd, 10);
	cli_size = sizeof(cli_addr);

    	while(1)
    	{
      		/* Accepts an incoming connection */
	  	newsockfd = accept(sockfd,  (struct sockaddr*)&cli_addr, &cli_size);
		if(newsockfd < 0)
		{
			perror("ERROR on accepting");
			exit(1);
		}
		
		/* Reads incoming message from client */
		bzero(recvline, sizeof(recvline));	
		n = read(newsockfd, recvline, sizeof(recvline));
		if(n < 0)
		{
			perror("ERROR reading");
			exit(1);	
		}
		
		// checks if url is in black list		
		if(!blacklisted(recvlineptr))
		{
		//*****URL is not in black list*****************
		
			// check if webpage has been cached
			if(!url_in_file(recvlineptr))
			{
	
			
				/* Gets host IP Address */
				he = gethostbyname(recvline);
				if(he == NULL)
				{
					perror("ERROR gethostbyname");
				exit(1);
				}

				web_ip = (inet_ntoa(*(struct in_addr*)(he->h_addr_list[0])));	// get ip address 
				printf("we_ip: %s\n", web_ip);
				//printf("ip: %s\n", inet_ntoa(*(struct in_addr*)(he->h_addr_list[0])));	
			
				//		printf("Name: %s\n", he->h_name);
	
		
				/************ create Web socket *************************/
				websocket = socket(AF_INET, SOCK_STREAM, 0);
				if(websocket < 0)
				{
					perror("ERROR opening websocket");
					exit(1);
				}
			
				bzero(&web_servaddr, sizeof(web_servaddr));
				web_servaddr.sin_family=AF_INET;
				web_servaddr.sin_port=htons(80);

				// covert IPv4 and IPv6 addresses from text to binary form  
				inet_pton(AF_INET, web_ip, &(web_servaddr.sin_addr));
		
				// connect to webserver
				if(connect(websocket, (struct web_sockaddr *)&web_servaddr, sizeof(web_servaddr)) < 0)
				{
					perror("ERROR connecting");
					exit(1);
				}	
				else
				{
					printf("Connected to Webserver..........\n");
				}	
	
				sleep(3);
	
				// send request to webserver
				m = write(websocket, request, strlen(request));
				if(m < 0)
				{
					perror("ERROR writing to sever");
					exit(1);
				}				
			
	
				bzero(buffer, 4000); // clear buffer
				FILE* list_fileptr;
				FILE* web_page_fileptr;
				
				read(websocket, buffer, 20);
				char* temp = strstr(buffer, http_response);
				if(temp == NULL)
				{
					write(newsockfd, buffer, 13);
					write(newsockfd, "\n", 1);
				}
				else
				{
					time( &current_time );
					info = localtime( &current_time );
					strftime(current_time_char, 80, "%Y%m%d%H%M%S", info);
					list_fileptr = fopen("list.txt", "a+");
					if(list_fileptr == NULL)
					{
						printf("ERROR opening: list.txt\n");
					}					
					fputs(recvline, list_fileptr);
					fputs(" ", list_fileptr);
					fputs(current_time_char, list_fileptr);
					fclose(list_fileptr);
					
					strcat(current_time_char, ".txt");
					web_page_fileptr = fopen(current_time_char, "a+");
					
					fwrite(buffer, sizeof(char), sizeof(buffer), web_page_fileptr);
					write(newsockfd, buffer, 20);
					bzero(buffer, 4000);
					
					// reading from webserver
					while(read(websocket, buffer, 3999) != 0)
					{		
						// send buffer to client
						//m = write(newsockfd, buffer, strlen(buffer));
						//if(m < 0)
						//{
						//	perror("ERROR writing to client");
						//	exit(1);
						//}
	
						fwrite(buffer, sizeof(char), sizeof(buffer), web_page_fileptr);	
						bzero(buffer, 40000); // clear buffer
					}
					fclose(web_page_fileptr);
					close(websocket);
					printf("Disconnected from webserver.........\n");
				}
					/************* end WEB socket *********************/
			}
			else
			{
				//****** send catched web page to client**********
				FILE* fileptr;
				char buff[200];

				fileptr = fopen(cached_time, "r+");
				if(fileptr == NULL)
				{
					printf("ERROR opening: %s\n", url_in_file_return);
				}
				while(!feof(fileptr))
				{
					fgets(buff, 200, fileptr);
					write(newsockfd, buff, strlen(buff));
					bzero(buff, 200); 

				}							

			}
		}
		else
		{
			//*******URL is in black list*********
			
			// send "blocked" to client
			m = write(newsockfd, "BLOCKED\n", 8);

		}	

      			close (newsockfd); //close the connection
   	 }
}
