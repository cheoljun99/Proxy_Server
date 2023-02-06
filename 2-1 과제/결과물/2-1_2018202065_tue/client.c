#include <stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>


#define BUFFSIZE 1024
#define PORTNO 40000

int main()
{
	/////////////////////////////////////////////////////////////////////
	// main 함수												         
	// ==================================================================
	// 																	 
	// 목적: main 함수로 리눅스에서 실행 되며 위에 정의한 함수들을       
	// 이용하여 본 과제의 목적인 socket programing을 구현한다.
	// proxy 서버통신에 있어 클라이언트 역할을 한다.       
	// 																	 
	// 																	 
	///////////////////////////////////////////////////////////////////////

	int socket_fd, len;
	struct sockaddr_in server_addr;
	char haddr[] = "127.0.0.1";
	char buf[BUFFSIZE];

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("can't create socket.\n");
		return -1;

	}
	bzero(buf, sizeof(buf));
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(haddr);
	server_addr.sin_port = htons(PORTNO);
	if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)//클라이언트를 서버에 컨넥트 시킴
	{
		printf("can't connect.\n");
		return -1;

	}

	write(STDOUT_FILENO, "input url > ", sizeof("input url > "));
	while ((len = read(STDIN_FILENO, buf, sizeof(buf))) > 0)//반복해서 리퀘스트를 입력받음
	{
		if (strcmp(buf, "bye\n") == 0)// bye종료문을 받으면 소켓통신을 종료함
		{
			write(socket_fd, buf, strlen(buf));// bye종료문을 받으면 소켓통신을 종료함
			break;

		}

		if (write(socket_fd, buf, strlen(buf)) > 0)//서버에게 리퀘스트를 전달.
		{
			if ((len = read(socket_fd, buf, sizeof(buf))) > 0)//서버에게 리스폰스를 받음
			{
				write(STDOUT_FILENO, buf, strlen(buf));//받은 리스폰스를 출력
				bzero(buf, sizeof(buf));//버퍼 초기화

			}
		}

		write(STDOUT_FILENO, "input url > ", sizeof("input url > "));
	}
	close(socket_fd);
	return 0;
}
