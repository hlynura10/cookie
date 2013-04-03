/*
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Bjarki Dagsson, bjarkid10@ru.is 
 *     Bjorn Heidar Runarsson, bjornr11@ru.is
 *     Hlynur Arnason, hlynura10@ru.is 
 *
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */ 

#include "csapp.h"

/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);

/* 
 * main - Main routine for the proxy program 
 */
void echo(int connfd);
void getRequestLine(int connfd);
void getRequiredHTTPheader();
void read_requesthdrs(rio_t *rp);
void clienterror(int connfd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void serve_static(int connfd, char *filename, int filesize);
void serve_dynamic(int connfd, char *filename, char *cgiargs);
void get_filetype(char *filename, char *filetype);

int main(int argc, char **argv)
{

	int listenfd, connfd, port;
	socklen_t clientlen;
	struct sockaddr_in clientaddr;
	struct hostent *hp;
	char *haddrp;
	if (argc != 2) {
	    fprintf(stderr, "usage: %s <port>\n", argv[0]);
	    exit(0);
	} 
	port = atoi(argv[1]);

	listenfd = open_listenfd(port);
	while(1) {
	    clientlen = sizeof(clientaddr);
	    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

	    /* Determine the domain name and IP address of the client */
	    hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
		               sizeof(clientaddr.sin_addr.s_addr), AF_INET);
	    haddrp = inet_ntoa(clientaddr.sin_addr);
	    printf("server connected to %s (%s)\n", hp->h_name, haddrp);

		//TODO
		getRequestLine(connfd);
/*		char *uri = "http://www.ru.is/";
		char hostname[50];
		char pathname[50];
		int port[16];
		printf("before parse %s\n", uri);
		parse_uri(uri, hostname, pathname, port);
		printf("after parse\n");
*/		//getRequestLine(connfd);
		//getRequiredHTTPheaderi();
		

	    //echo(connfd); //remove this
	    Close(connfd);
	}
	exit(0);

    /* Check arguments 
    if (argc != 2) {
	fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	exit(0);
    }*/
}
void get_filetype(char *filename, char *filetype)
{
	if(strstr(filename, ".html"))
	{
		strcpy(filetype, "text/html");
	}
	else if(strstr(filename, ".gif"))
	{
		strcpy(filetype, "image/gif");
	}
	else if(strstr(filename, ".jpg"))
	{
		strcpy(filetype, "image/jpeg");
	}
	else
	{
		strcpy(filetype, "text/plain");
	}
}
void serve_dynamic(int connfd, char *filename, char *cgiargs)
{
	char buf[MAXLINE], *emptylist[] = {NULL};

	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	Rio_writen(connfd, buf, strlen(buf));
	sprintf(buf, "Server: Tiny Web Server\r\n");
	Rio_writen(connfd, buf, strlen(buf));

	if(Fork() == 0)
	{
		setenv("QUERY_STRING", cgiargs, 1);
		Dup2(connfd, STDOUT_FILENO);
		Execve(filename, emptylist, environ);
	}
	Wait(NULL);
}

void serve_static(int connfd, char *filename, int filesize)
{
	int srcfd;
	char *srcp, filetype[MAXLINE], buf[MAXBUF];

	get_filetype(filename, filetype);
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
	sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
	sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
	Rio_writen(connfd, buf, strlen(buf));

	srcfd = Open(filename, O_RDONLY, 0);
	srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	Close(srcfd);
	Rio_writen(connfd, srcp, filesize);
	Munmap(srcp, filesize);
}
void clienterror(int connfd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
	char buf[MAXLINE], body[MAXBUF];
	sprintf(body, "<html><title>Tiny Error</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Tiny ..<\em>\r\n", body);
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	Rio_writen(connfd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	Rio_writen(connfd, buf, strlen(buf));
	sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
	Rio_writen(connfd, buf, strlen(buf));
	Rio_writen(connfd, body, strlen(body));

}
void read_requesthdrs(rio_t *rp)
{
	char buf[MAXLINE];
	Rio_readlineb(rp, buf, MAXLINE);
	while(strcmp(buf, "\r\n"))
	{
		Rio_readlineb(rp, buf, MAXLINE);
		printf("%s", buf);
	}
}

void getRequestLine(int connfd)
{
	int is_static;
	struct stat sbuf;
	rio_t rio;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];
	int port[1];

	Rio_readinitb(&rio, connfd);
-	Rio_readlineb(&rio, buf, MAXLINE);
	sscanf(buf, "%s %s %s", method, uri, version);
	printf("buf = %s, method = %s, uri = %s, version = %s", buf, method, uri, version);
	if(strcasecmp(method, "GET"))
	{
		clienterror(connfd, method, "501", "Not Implemented", "TINY");
		return;
	}
	read_requesthdrs(&rio);

	is_static = parse_uri(uri, filename, cgiargs, port);
	if(stat(filename, &sbuf) < 0)
	{
		clienterror(connfd, filename, "404", "Not found", "Tiny");
		return;
	}
	if(is_static)
	{
		if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
		{
			clienterror(connfd, filename, "403", "Forbidden", "Tiny 4");
			return;
		}
		serve_static(connfd, filename, sbuf.st_size);
	}
	else
	{
		if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
		{
			clienterror(connfd, filename, "403", "Forbidden", "Tiny 4");
			return;
		}
		serve_dynamic(connfd, filename, cgiargs);
	}
}

void getRequiredHTTPheader()
{

}

void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
	printf("server received %d bytes\n", n);
	Rio_writen(connfd, buf, n);
    }
}
/*
 * parse_uri - URI parser
 * 
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port)
{
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

	printf("1 ");
    if (strncasecmp(uri, "http://", 7) != 0) {
	hostname[0] = '\0';
	return -1;
    }
       
	printf("2 ");
    // Extract the host name
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';
    
	printf("3 ");
    // Extract the port number
    *port = 80; // default
    if (*hostend == ':')   
	*port = atoi(hostend + 1);
    
	printf("4 ");
    // Extract the path
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
	pathname[0] = '\0';
    }
    else {
	pathbegin++;	
	strcpy(pathname, pathbegin);
    }

	printf("5 ");
    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
		      char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /* 
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;


    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s", time_str, a, b, c, d, uri);
}


