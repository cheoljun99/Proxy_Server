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
	// main �Լ�												         
	// ==================================================================
	// 																	 
	// ����: main �Լ��� ���������� ���� �Ǹ� ���� ������ �Լ�����       
	// �̿��Ͽ� �� ������ ������ socket programing�� �����Ѵ�.
	// proxy ������ſ� �־� Ŭ���̾�Ʈ ������ �Ѵ�.       
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
	if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)//Ŭ���̾�Ʈ�� ������ ����Ʈ ��Ŵ
	{
		printf("can't connect.\n");
		return -1;

	}

	write(STDOUT_FILENO, "input url > ", sizeof("input url > "));
	while ((len = read(STDIN_FILENO, buf, sizeof(buf))) > 0)//�ݺ��ؼ� ������Ʈ�� �Է¹���
	{
		if (strcmp(buf, "bye\n") == 0)// bye���Ṯ�� ������ ��������� ������
		{
			write(socket_fd, buf, strlen(buf));// bye���Ṯ�� ������ ��������� ������
			break;

		}

		if (write(socket_fd, buf, strlen(buf)) > 0)//�������� ������Ʈ�� ����.
		{
			if ((len = read(socket_fd, buf, sizeof(buf))) > 0)//�������� ���������� ����
			{
				write(STDOUT_FILENO, buf, strlen(buf));//���� ���������� ���
				bzero(buf, sizeof(buf));//���� �ʱ�ȭ

			}
		}

		write(STDOUT_FILENO, "input url > ", sizeof("input url > "));
	}
	close(socket_fd);
	return 0;
}
