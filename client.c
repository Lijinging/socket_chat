//客户端

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#define MAXLINE 4096

pthread_mutex_t mutex;



void *pforsend(void *_sock){
    char sendbuf[MAXLINE];
    long sockfd = (long)_sock;
    char sexit[10] = {'e','x','i','t','\n',0};
    while(1){

        fgets(sendbuf, 4096, stdin);
        if(strcmp(sendbuf,sexit)==0){
        	exit(0);
            break;
        }
        pthread_mutex_lock(&mutex);
        if( send(sockfd, sendbuf, strlen(sendbuf), 0) < 0)
        {
            printf("\033[32msend msg error: %s(errno: %d)\n\033[0m", strerror(errno), errno);
            exit(0);
        }
        printf("\033[32m");
        printf("I>:%s",sendbuf);
        printf("\033[0m");
        pthread_mutex_unlock(&mutex);
        usleep(100000);
    }
    return;
}

int myprint(char *s){
    while((*s!=0)&&(*s!='\n')){
    	printf("\033[33m");
        putchar(*s);
        printf("\033[0m");
        s++;
    }
    return 0;
}


int main(int argc, char** argv)
{
    int    sockfd, n;
    char    recvline[4096], sendline[4096];
    struct sockaddr_in    servaddr;
    pthread_t pp;

    
    if( argc != 2){
        printf("usage: ./client <ipaddress>\n");
        exit(0);
    }
    pthread_mutex_init(&mutex,NULL);

    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    if( inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0){
        printf("inet_pton error for %s\n",argv[1]);
        exit(0);
    }

    if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        printf("\033[31mconnect error: %s(errno: %d)\n\033[0m",strerror(errno),errno);
        exit(0);
    }

    //printf("\\033[32msend msg to server: \n \\033[0m");
    fcntl(sockfd,F_SETFL,O_NONBLOCK);
    pthread_create(&pp,NULL,pforsend,(void*)((long)sockfd));
    while(1){
        pthread_mutex_lock(&mutex);
 //       printf("1\n");
        if(recv(sockfd,recvline,MAXLINE,0)>0){
           // printf("%s\n",recvline);
            myprint(recvline);
            printf("\n");
        }
    //    printf("12\n");
        pthread_mutex_unlock(&mutex);
   		usleep(100000);
    }

    close(sockfd);
    exit(0);
}