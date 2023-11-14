#include <stdio.h>
#include <sys/types.h>   
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <locale.h>
#include <sys/wait.h>	
#include <signal.h>	
#include <errno.h>
#include <fcntl.h>

#define PORT 1111


const int BUFLEN = 1024;
static char* HEADER_200_OK ="HTTP/1.1 200 OK\r\nContent-type: %s\r\n\r\n";
static char* HEADER_400_BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\nContent-type: %s\r\n\r\n";
static char* HEADER_400_NOT_FOUND = "HTTP/1.1 404 Not Found\r\nContent-type: %s\r\n\r\n";
static char* HEADER_501_METHOD_NOT_IMPLEMENTED = "HTTP/1.1 501 Method Not Implemented\r\nContent-type: %s\r\n\r\n";



char* get_file_extension(char* file_type){

	char * content_type;
	if (strcmp (file_type, "txt") == 0 || strcmp(file_type,"TXT") ==0)
		content_type = "text/plain";
	else if (strcmp (file_type, "html") == 0 || strcmp (file_type, "HTML") == 0)
		content_type = "text/html";
	else if (strcmp (file_type, "jpeg") == 0 || strcmp(file_type,"JPEG") ==0)
		content_type = "image/jpeg";
	else if (strcmp (file_type, "jpg") == 0 || strcmp (file_type, "JPG") == 0)
		content_type = "image/jpeg";
	else if (strcmp (file_type, "ico") == 0 || strcmp (file_type, "ICO") == 0)
		content_type = "image/x-ico";
	else if (strcmp (file_type, "pdf") == 0 || strcmp (file_type, "PDF") == 0)
		content_type = "application/pdf";

	else
		content_type = "text/plain";
		
		return content_type;
}

void send_file(int destination, char *content, int file_fd, char *content_type, char* template){
	int i;
	char c;
	//pentru a retine in response HEADER content type ul
	snprintf(content, BUFLEN-1, template, content_type);
	write(destination, content, strlen (content));
    
	while ( (i = read(file_fd, &c, 1)) ) {
		if ( write(destination, &c, 1) < 0 )
		  {
      			perror ("[server] Eroare la write()\n");
      			exit(1);
    	          }
	}
}


static int get_file (int requestfd, char* path) {
	char *file_type, *content_type;
	int file_fd;
	char response[BUFLEN];
	memset(response,0,BUFLEN);

	if (strcmp(path,"/") == 0)
	{

		file_fd = open("homepage.html",O_RDONLY);
		content_type = "text/html";
		send_file(requestfd,response, file_fd, content_type, HEADER_200_OK);
		close(file_fd);

		return 0;
				
	}

	file_type = strchr(path, '.');
	if (!file_type){
	
		char new_path[BUFLEN], path1[BUFLEN];
		strcpy(new_path,path);
		content_type = "text/plain";
		strcat(new_path, ".txt");
		strcpy(path1, new_path);
		path1[strlen(path1)] = '\0';
		path = (char*)path1;
		
		file_type = strchr (path, '.');
		file_type++;
		}
	else
		file_type++;
	file_fd = open(++path, O_RDONLY);
	if (file_fd < 0) {

		file_fd = open("404.html",O_RDONLY);
		content_type = "text/html";
		send_file(requestfd,response,file_fd, content_type,HEADER_400_NOT_FOUND);
		close(file_fd);

		return 0;
	}

	content_type = get_file_extension(file_type);

	send_file(requestfd,response,file_fd, content_type, HEADER_200_OK);
	close(file_fd);

	return 0;
}

int client_request(char* buffer, int sockfd)
{
	
	char * method1;
	char * uri1;
	char * http_version1;
	char * content_type;
        char method[BUFLEN];
        char uri[BUFLEN];
        char http_version[BUFLEN];
        int file_fd;
        char response[BUFLEN]; 
	memset(response, 0, BUFLEN);

	method1 = strtok(buffer, " ");
	uri1 = strtok(NULL, " ");
	http_version1 = strtok (NULL, "\n");
 
     strcpy(method, method1);
     strcpy(uri, uri1);
     strcpy(http_version, http_version1);
     
     http_version[strlen(http_version)-1] = '\0';

    
	if (strlen(method) == 0 || strlen(uri) == 0 || strlen(http_version) == 0){

		file_fd = open("400.html",O_RDONLY);
		content_type = "text/html";
		send_file(sockfd,response, file_fd, content_type, HEADER_400_BAD_REQUEST);
		close(file_fd);
		
		return -1;
	}

	if (strcmp(method, "GET")){
		file_fd = open("501.html",O_RDONLY);
		content_type = "text/html";
		send_file(sockfd,response, file_fd, content_type, HEADER_501_METHOD_NOT_IMPLEMENTED);
		close(file_fd);
		
		return -1;
	}


	else if  (strcmp(http_version, "HTTP/1.0") && strcmp(http_version, "HTTP/1.1")) {

		file_fd = open("400.html",O_RDONLY);
		content_type = "text/html";
		send_file(sockfd,response, file_fd, content_type, HEADER_400_BAD_REQUEST);
		close(file_fd);
		
		return -1;
	} 
	else
		return get_file(sockfd, uri);
}

int main()
{

     int sockfd, newsockfd, pid;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
            {
      		perror ("[server] Eroare la socket().\n");
      		exit(1);
    	    }
     bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	    {
      		perror ("[server] Eroare la bind().\n");
      		exit(1);
    	    }
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     
     
     
     while (1) {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0) 
            {
      		perror ("[server] Eroare la accept().\n");
      		exit(1);
    	    }
         pid = fork(); //create a new process
         if (pid < 0)
            {
      		perror ("[server] Eroare la fork().\n");
      		exit(1);
    	    }
         if (pid == 0)  { //child process
             
             close(sockfd);
		int n;
	        char buffer[BUFLEN];
		memset(buffer, 0, BUFLEN);	
		n = recv(newsockfd,buffer,BUFLEN-1,0);
		if (n < 0)
	          {
      			perror ("[server] Eroare la recv().\n");
      			exit(1);
    	          }
		printf("Requestul primit de la client: \n%s\n",buffer);
		buffer[n] = '\0'; 
		n = client_request(buffer, newsockfd);
		if (n == -1) 
			printf("Request was not successful.");

		printf("\n--urmatorul request--\n");
             exit(0);
         }
         else 
             close(newsockfd); 
     } 
     return 0; 
}
