//服务器端

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

const int MAXLINE = 4096;
const int MaxThread = 64;
int nowthread ;

struct _message{
	char mess[100][4096];
	long mefrom[100];
	int nowlocate;	
};
struct _message maillist; 

pthread_rwlock_t list_mutex;



void *client(void* _sock){
	char buf[MAXLINE];
	char bufsend[MAXLINE];
	long cilentSocket=(long)_sock;
	int numbytes;
	int nowcount = 0;
	int first  = 1;
	char name[100];

	fcntl(cilentSocket,F_SETFL,O_NONBLOCK);

	while(1)
	{
		/*
			begin to send message
		*/
		pthread_rwlock_rdlock(&list_mutex);

		while(maillist.nowlocate>nowcount){
			if(maillist.mefrom[nowcount%100]!=cilentSocket){
				strcpy(buf,maillist.mess[nowcount%100]);
				//printf("k = %d\n",k);
				//buf[k] = 0;
				send(cilentSocket,buf,strlen(buf),0);
			}
			nowcount++;
		}

		pthread_rwlock_unlock(&list_mutex);

		/*
			begin to recieve message.
		*/
		if ((numbytes=recv(cilentSocket , buf, MAXLINE, 0)) == -1) {
		    //perror("error message");
	//	    sleep(1);
		    continue;
		}   
		if(numbytes==0)
		{
			printf("out of client\n");
			break;
		}
		buf[numbytes] = '\0';
		if(first){
			strcpy(name,buf);
			name[numbytes-1] = 0;
			first = 0;
		}
		//printf("%s>: %s\n",name,buf);
		int k = sprintf(bufsend,"%s>: %s",name,buf);
		bufsend[k] = 0;
		pthread_rwlock_wrlock(&list_mutex);
		strcpy(maillist.mess[maillist.nowlocate%100],bufsend);
		maillist.mess[maillist.nowlocate%100][k] = 0;
		maillist.mefrom[maillist.nowlocate%100] = cilentSocket;
		maillist.nowlocate++;
		pthread_rwlock_unlock(&list_mutex);
		usleep(50000);
	}
//	close(cilentSocket);
	printf("socket close!\n");
	pthread_rwlock_wrlock(&list_mutex);
	--nowthread;
	pthread_rwlock_unlock(&list_mutex);
	pthread_rwlock_rdlock(&list_mutex);
	printf("Number of clients:%d\n",nowthread);
	pthread_rwlock_unlock(&list_mutex);
	close(cilentSocket);
}




int main(int argc, char** argv)
{
	int    listenfd, connfd;
	struct sockaddr_in     servaddr,useraddr;
	char    buff[4096];
	int     n;
	pthread_t threads[10];

	pthread_rwlock_init(&list_mutex,NULL);
	
	maillist.nowlocate = 0;
	if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(6666);

	if( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
		printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
	}

	if( listen(listenfd, 10) == -1){
		printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
	}

	nowthread = 0;
	printf("======waiting for client's request======\n");
	while(1){
		pthread_rwlock_rdlock(&list_mutex);
		if (nowthread>MaxThread)
		{
			break;
		}
		pthread_rwlock_unlock(&list_mutex);
		printf("1\n");
		if( (connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1){
			printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
			continue;
		}
		printf("12\n");
		// printf("get connect from %s ！\n", inet_ntoa(useraddr.sin_addr));

		if (send(connfd, "Please input your name:\n", MAXLINE, 0) == -1){
		    perror("fail to connect\n");
		    continue;
		}


		/*n = recv(connfd, buff, MAXLINE, 0);
		buff[n] = '\0';
		printf("recv msg from client: %s\n", buff);
		close(connfd);*/
		pthread_rwlock_wrlock(&list_mutex);
		if(pthread_create(&threads[nowthread],NULL,client,(void*)((long)connfd))!=0){
			printf("error create the %d\n pthread\n",nowthread);
		}
		nowthread++;
		pthread_rwlock_unlock(&list_mutex);
		pthread_rwlock_rdlock(&list_mutex);
		printf("Number of clients:%d\n",nowthread);
		pthread_rwlock_unlock(&list_mutex);
	}
	pthread_rwlock_destroy(&list_mutex);
	close(listenfd);
}