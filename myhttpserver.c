
#define SUCCESS "HTTP/1.0 200 OK\n"
#define ERROR "HTTP/1.0 404 Not Found\n"
#define BAD "TTTP/1.0 400 Bad Request\n"
#define MESS_404 "<html><body><h1>FILE NOT FOUND</h1></body></html>"
#define DIRRES "NO index.html Found. Here is the files in this directory:\n"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <netdb.h>
#include <dirent.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include "thread_pool.h"
#include "type.h"


int waitingtime=60, islog=0, iswait, isdebug=0;
int thread_num=4;
char *log_file=NULL;
thread_pool_t pool;
char response[]="HTTP/1.1 200 OK\n"
"Content-Type:text/html;charset=UTF-8\n\n"
"this is a web server test";
//"<html><head><title>ENZE SErver</title></head>\n"
//"<body><center><h1>HELLO WORLD</h1></center></body>\n"
//"</html>\n";


void usage(){
 fprintf(stderr, "usage:  [-p port] [-s sched] [-r directory] [-t sleep time]\n");
 exit(1);
}

void* request_process(struct request *req){
	
	time_t curtime=time(NULL);
	struct tm tm=*gmtime(&curtime);
	char serve_time[128];
	strcpy(serve_time,asctime(&tm));
	//strftime(serve_time, sizeof(serve_time), "[%d/%b/%Y : %H:%M:%S %z]", &tm);
	char *status=NULL;
	char *fname = NULL;
	char *method = NULL;
	char fsize[128];
	char output[8192];
	struct stat fileStat;
	char hostname[128];
	char buf[1024];
	char dir_name[128];
	size_t nread;

	char bytes[4];
	unsigned int remote_ip=req->ip;
	bytes[0] = remote_ip & 0xFF;
	bytes[1] = (remote_ip >> 8) & 0xFF;
	bytes[2] = (remote_ip >> 16) & 0xFF;
	bytes[3] = (remote_ip >> 24) & 0xFF;
	fname=req->file_name;

	if(stat(fname, &fileStat)< 0) 
		status=ERROR;
	else
		status=SUCCESS;
	
	if(islog==1){
		if(isdebug==0){
			FILE * file_log=fopen(log_file,"a");
			fprintf(file_log,"%d.%d.%d.%d  -  ", bytes[0], bytes[1], bytes[2], bytes[3]);
			fprintf(file_log,"%s\b%s\b%s\b%s\b%d\n",req->time_arrival,serve_time,req->buffer,status, req->file_size);
			fclose(file_log);
		}
		else{
			printf("%d.%d.%d.%d  -  ", bytes[0], bytes[1], bytes[2], bytes[3]);
			printf("%s\b%s\b%s\b%s\b%d\n",req->time_arrival,serve_time,req->buffer, status, req->file_size);
		}
	}

	gethostname(hostname,sizeof(hostname));   	
	memset(output, 0, sizeof(output));;
	memset(buf,0, sizeof buf);

	method=strtok(req->buffer," ");

    //If the req->est is for a folder!
	if(strstr(fname,".")==NULL){
		if(stat(fname, &fileStat)< 0){
			status=ERROR;
			strcat(output,ERROR);
			perror("Directory NOT Found!");
			send(req->socketfd, output, sizeof(output), 0);
			close(req->socketfd);
		}
		else{
			strcpy(dir_name,fname);
			strcat(fname,"/index.html");
			FILE *file=fopen(fname,"r"); 
			if(!file){
				DIR *dr=opendir(dir_name);
				struct dirent *dir;
				if(dr){
					send(req->socketfd,DIRRES, sizeof(DIRRES),0);
					while((dir=readdir(dr))!=NULL){
						char res[128];
						snprintf(res,128,"%s\n",dir->d_name);
						if(res[0]=='.') continue;
						printf("%s",res);         	        
						send(req->socketfd, res, 128, 0);
					} 
					closedir(dr);
					close(req->socketfd);
				}
			}
		    else
		      fclose(file);
		}
	}

	if(stat(fname, &fileStat)< 0){             //the file do not exist 
		status=ERROR;
		strcat(output,ERROR);
		perror("No file!");
		send(req->socketfd, output, sizeof(output), 0);
	}
	else{                                 //get the file 
		//const struct timespec lmodified=fileStat.st_mtimespec;
		//send(req->socketfd, response, sizeof(response), 0);
		strcat(output,SUCCESS);
		strcat(output,"Date:");
		strcat(output,serve_time);
		strcat(output,"Server:");
		strcat(output,hostname);
		strcat(output,"\n");
		strcat(output,"Last-Modified:");
		strcat(output,asctime(gmtime(&fileStat.st_mtime)));
		strcat(output,"Content-Length:");
		snprintf(fsize,128,"%d",fileStat.st_size);
		strcat(output,fsize);
		strcat(output,"\n");

		if(strstr(fname, ".jpg") != NULL || strstr(fname, ".gif") != NULL){
			strcat(output,"Content-Type:image/gif;charset=UTF-8\n\n");
			send(req->socketfd,output,sizeof(output),0);
			if(strcmp(method,"HEAD")== 0){
				close(req->socketfd);
			}
			FILE *fp =fopen(fname,"rb");
			FILE *newfp= fopen("zztest.jpg","ab");
			while((nread=fread(buf, 1, sizeof(buf), fp)) > 0){
			    	// //strcat(output,buf);
				  //fwrite(newfp, buf, sizeof(buf));
		         //fwrite(buf, 1, nread, newfp);
			    send(req->socketfd,buf,sizeof buf,0);
			}
			printf("File sent!\n");
			fclose(fp);
		}
		else if( strstr(fname,".html") != NULL || strstr(fname,".txt")!= NULL){
			strcat(output,"Content-Type:text/html;charset=UTF-8\n\n");
			send(req->socketfd,output,sizeof(output),0);
			if(strcmp(method,"HEAD")== 0){
				close(req->socketfd);
			}
			//send(req->socketfd,output,sizeof(output),0);
			FILE *fp =fopen(fname, "rb");
			memset(buf,0, sizeof buf);
			while(fread(buf, 1, sizeof(buf), fp) > 0){
			        //strcat(output,buf);
				send(req->socketfd,buf,sizeof(buf),0);
			}
			printf("File sent!\n");
			fclose(fp);
		}
	}
	printf("closing...\n\n");
	close(req->socketfd);
	return NULL;
}


void* Listen_Thread(void * sockfd){

	unsigned int sock_fd = (unsigned int*)sockfd;
	char temp_buf[128];
	struct sockaddr_in cli_addr;
	socklen_t cli_len = sizeof(cli_addr);
	struct request *ac_request;
	

	unsigned int remote_ip=cli_addr.sin_addr.s_addr;
	char *file_name = NULL;
	char *method =NULL;
	struct stat fStat;
	listen(sock_fd,1);

	while(1){

		ac_request=(struct request*)malloc(sizeof(struct request));

		fprintf(stderr, "Waiting for connection...\n");
		int new_socket= accept(sock_fd, (struct sockaddr *) &cli_addr, &cli_len);

		if(new_socket<0)
			perror("Can not accept!");

		memset(temp_buf, 0, sizeof(temp_buf));

		if(recv(new_socket,temp_buf,sizeof(temp_buf), 0)<0){
			perror("can not recv!");
		}
		else{
			temp_buf[strlen(temp_buf)-1] = 0;
			printf("Request recieved: %s \n", temp_buf);
			strcpy(ac_request->buffer,temp_buf);
			method = strtok(temp_buf, " \t\n");
			file_name = strtok(NULL, " \t");		
		}
		if(strcmp(file_name,"/")==0)
			file_name="index.html";
		else 
		    file_name++; //remove the first char "/" from the file name to get the relative path
    
		if(stat(file_name, &fStat)<0 || strcmp(method,"HEAD")==0)
			ac_request->file_size = 0;
		else
			ac_request->file_size = fStat.st_size;
		//get current time in unix
		time_t curtime=time(NULL);
		struct tm tm=*gmtime(&curtime);
		char atime[128];
		strcpy(atime,asctime(&tm));

		ac_request->file_size = fStat.st_size;
		ac_request->socketfd= new_socket;
		ac_request->file_name= file_name;
		strcpy(ac_request->time_arrival,atime);
		ac_request->ip = remote_ip;

		//request_process(&ac_request->;
		//close(new_socket);
		thread_pool_add_task(pool, request_process,  (void*)ac_request);
		
		//free(ac_request);
	}
	//  //this is a test for calling the request_process!
	return NULL;
}



int main(int argc, char *argv[]){

	int socket_fd, ch;
	socklen_t len;
	int sch_pcy = 0;
	char *port = "8080";
	char *sched = "FCFS";
	char *cdir =NULL;

	pthread_t thread_listener, thread_scheduler;

	struct sockaddr_in severv_addr,client_addr;
	struct servent *se;
	

	int isdir=0;

	int thread_num = 4;
	//port="8080";
	//char *port_num = 8080;
	extern char *optarg;
	extern int optind;

	while ((ch = getopt(argc, argv, "hdi:t:p:n:r:s:")) != -1)
		switch(ch) {
			case 'h':
				usage();		/* print address in output */
				break;
			case 'd':
				isdebug = 1;
				break;
			case 'i':
			    islog = 1;
			    log_file = optarg;
			case 't':
			    iswait = 1;
			    waitingtime = atoi(optarg);
			    break;
			case 'p':
				port = optarg;
				break;
			case 'n':
			    thread_num = atoi(optarg);
                break;
            case 'r':
                isdir=1;
                cdir=optarg;
                break;
            case 's':
			    sched=optarg;
				break;
			case '?':
			default:
				usage();
	}
	argc -= optind;
	if (argc != 0)
		usage();
	if (isdir==1){
		if(chdir(cdir)<0)
			perror("Can not find the dirctory!");
		else{
			char cpath[128];
			getcwd(cpath,128);
			printf("Current directory:%s\n",cpath);
		    //change the dir
		}
	}
    
    if(strcmp(sched,"FCFS")==0){
    	sch_pcy=0;
    	printf("Using FCFS Scheuling!\n");
    }
    else if(strcmp(sched,"SJF")==0){
    	sch_pcy=1;
    	printf("Using SJF Scheuling!\n");
    }
    else
    	printf("Invalid Scheuling! FCFS will be used by default!\n");

	if(isdebug){ printf("Debugging model:\n");
	 	pool = thread_pool_create(1, sch_pcy, waitingtime); 		//create just one thread!
	}
	else
		pool = thread_pool_create(thread_num, sch_pcy, waitingtime);

	socket_fd = socket(AF_INET, SOCK_STREAM,0);    //create the socket
	if(socket_fd<0){
		perror("wrong socket!");
		exit(1);
	}
   
    len=sizeof(client_addr);

	memset((void *)&severv_addr, 0, sizeof(severv_addr));

	severv_addr.sin_family = AF_INET;
	//port = atoi(argv[1]);	
	//severv_addr.sin_port = htons(port);
	if (port == NULL)
		severv_addr.sin_port = htons(0);
	else if (isdigit(*port))
		severv_addr.sin_port = htons(atoi(port));
	else {
		if ((se = getservbyname(port, (char *)NULL)) < (struct servent *) 0) {
			perror(port);
			exit(1);
		}
		severv_addr.sin_port = se->s_port;
	}

	if(bind(socket_fd, (struct sockaddr *) &severv_addr, sizeof(severv_addr)) < 0){
		perror("bind error!");
		exit(1);
	}

	if (getsockname(socket_fd, (struct sockaddr *) &client_addr, &len) < 0) {
		perror("getsockname");
		exit(1);
	}

	fprintf(stderr, "Port number is %d\n", ntohs(client_addr.sin_port));

	pthread_create(&thread_listener,NULL, (void* (*)(void*))Listen_Thread, (void *)socket_fd); // server listener thread!
	

	// for(int i=0;o<thread_num;i++){
	// 	cout<<"thread created!"<<endl;
	// 	pthread_create(&threads[i],NULL,request_process, NULL);
	// }
	//new_socketfd=socket_fd;
	pthread_join(thread_listener, NULL);

	return 0;

}

// void
// usage()
// {
// 	fprintf(stderr, "usage: %s -h host -p port\n", progname);
// 	fprintf(stderr, "usage: %s -s [-p port]\n", progname);
// 	exit(1);
// }
