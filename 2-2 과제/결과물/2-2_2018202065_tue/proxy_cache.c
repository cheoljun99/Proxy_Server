//////////////////////////////////////////////////////////////////////
// File Name : proxy_cache.c								        
// Date : 2022/04/27												 
// Os : Ubuntu 16.04 LTS 64bits										 
// Author : Park Cheol Jun											 
// Student ID : 2018202065											 
// ----------------------------------------------------------------- 
// Title : System Programming Assignment #2-2 (proxy server)		 
// Description : Construction Proxy Connection				         		 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/stat.h>
#include<pwd.h>
#include<time.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<errno.h>
#include<fcntl.h> 
#include<dirent.h>
#include<openssl/sha.h>
#include<stdlib.h>
#include <errno.h>

#define BUFFSIZE 1024
#define PORTNO 39999


char* sha1_hash(char* input_url, char* hashed_url) {
	///////////////////////////////////////////////////////////////////////
	// sha1_hash														 //	
	// ==================================================================//
	// ��ǲ: char* -> input url										     //
	//		 char* -> pre hashed url									 //
	// �ƿ�ǲ: char* -> post hashed url									 //
	// 																	 //
	// ����: �ؽ��� ���ڿ��� ��ȯ�ϴ� �뵵�̴�.							 //
	// 																	 //
	// 																	 //
	///////////////////////////////////////////////////////////////////////


	unsigned char hashed_160bits[20];

	char hashed_hex[41];

	int i;

	SHA1(input_url, strlen(input_url), hashed_160bits);



	for (i = 0; i < sizeof(hashed_160bits); i++)

		sprintf(hashed_hex + i * 2, "%02x", hashed_160bits[i]);

	strcpy(hashed_url, hashed_hex);

	return hashed_url;



}

char* getHomeDir(char* home)
{
	////////////////////////////////////////////////////////////////////////
	// getHomeDir														  //
	// ===================================================================//
	// ��ǲ: char* ->pre home										      //
	// �ƿ�ǲ: char* -> post home										  //
	// 																	  //
	// ����: home���丮�� ����ϱ����� �Լ��̴�.						  //
	// 																	  //
	// 																	  //
	////////////////////////////////////////////////////////////////////////

	struct passwd* usr_info = getpwuid(getuid());

	strcpy(home, usr_info->pw_dir);

	return home;

}

int discrimination(char* first_HS_URL, char* second_HS_URL, char* URL)
{
	////////////////////////////////////////////////////////////////////////
	// discrimination													  //
	// ===================================================================//
	// ��ǲ: char* ->first_HS_URL										  //
	// ��ǲ: char* -> second_HS_UR										  //
	// ��ǲ: char* -> URL												  //
	//																	  //
	// �ƿ�ǲ: Hit�ΰ�� 2��ȯ											  //
	// �ƿ�ǲ: Miss�ΰ�� 1��ȯ								 			  //
	// 																	  //
	// ����: Hit, Miss�� Ȯ���ϱ����� �Լ�								  //
	//																	  //
	//																	  //
	//																	  //
	////////////////////////////////////////////////////////////////////////
	char dire2[100];
	getHomeDir(dire2);
	strcat(dire2, "/cache/");
	strcat(dire2, first_HS_URL);

	DIR* dp = NULL;// ���丮�� �������� DIR*�� dp ���� ����
	struct  dirent* entry = NULL; // �ؽ� ���丮�ȿ� �ؽ� ������ �������� dirent* ����ü entry���� ����


	if ((dp = opendir(dire2)) == NULL)// �ؽ� ���丮�� ������
	{
		//���Ŀ� ���� �ؽ�Ʈ ���Ͽ� �ۼ��Ѵ�.
		getHomeDir(dire2);
		strcat(dire2, "/logfile/");
		strcat(dire2, "logfile.txt");
		int fd3;
		fd3 = open(dire2, O_WRONLY | O_APPEND, 0777);
		char st[100];
		memset(st, 0, 100);
		write(fd3, "[Miss] ", strlen("[Miss] "));

		write(fd3, "ServerPID : ", strlen("ServerPID : "));
		sprintf(st, "%ld", ((long)getpid()));
		write(fd3, st, strlen(st));
		write(fd3, " | ", strlen(" | "));


		write(fd3, URL, strlen(URL));
		write(fd3, "-[", strlen("-["));
		time_t now;
		struct tm* ltp;
		time(&now);
		ltp = localtime(&now);
		sprintf(st, "%d", (ltp->tm_year + 1900));
		write(fd3, st, strlen(st));
		write(fd3, "/", strlen("/"));
		sprintf(st, "%d", (ltp->tm_mon));
		write(fd3, st, strlen(st));
		write(fd3, "/", strlen("/"));
		sprintf(st, "%d", (ltp->tm_mday));
		write(fd3, st, strlen(st));
		write(fd3, ", ", strlen(", "));
		sprintf(st, "%d", (ltp->tm_hour));
		write(fd3, st, strlen(st));
		write(fd3, ":", strlen(":"));
		sprintf(st, "%d", (ltp->tm_min));
		write(fd3, st, strlen(st));
		write(fd3, ":", strlen(":"));
		sprintf(st, "%d", (ltp->tm_sec));
		write(fd3, st, strlen(st));
		write(fd3, "]\n", strlen("]\n"));
		close(fd3);
		closedir(dp);
		return 1;//Miss�� ��� 1�� ��ȯ

	}
	while ((entry = readdir(dp)) != NULL)//�ؽ� ���丮�� ������
	{

		if (!strcmp(second_HS_URL, entry->d_name))// �ؽ������� ������ Ȯ��
		{
			//�ؽ������� ������ ���Ŀ� ���� ���
			getHomeDir(dire2);
			strcat(dire2, "/logfile/");
			strcat(dire2, "logfile.txt");
			int fd4;
			fd4 = open(dire2, O_WRONLY | O_APPEND, 0777);

			char st2[100];
			memset(st2, 0, 100);

			write(fd4, "[Hit]", strlen("[Hit]"));

			write(fd4, "ServerPID : ", strlen("ServerPID : "));
			sprintf(st2, "%ld", ((long)getpid()));
			write(fd4, st2, strlen(st2));
			write(fd4, " | ", strlen(" | "));

			write(fd4, first_HS_URL, strlen(first_HS_URL));
			write(fd4, "/", strlen("/"));
			write(fd4, second_HS_URL, strlen(second_HS_URL));
			write(fd4, "-[", strlen("-["));
			time_t now2;
			struct tm* gtp;
			time(&now2);
			gtp = localtime(&now2);
			sprintf(st2, "%d", (gtp->tm_year + 1900));
			write(fd4, st2, strlen(st2));
			write(fd4, "/", strlen("/"));
			sprintf(st2, "%d", (gtp->tm_mon));
			write(fd4, st2, strlen(st2));
			write(fd4, "/", strlen("/"));
			sprintf(st2, "%d", (gtp->tm_mday));
			write(fd4, st2, strlen(st2));
			write(fd4, ", ", strlen(", "));
			sprintf(st2, "%d", (gtp->tm_hour));
			write(fd4, st2, strlen(st2));
			write(fd4, ":", strlen(":"));
			sprintf(st2, "%d", (gtp->tm_min));
			write(fd4, st2, strlen(st2));
			write(fd4, ":", strlen(":"));
			sprintf(st2, "%d", (gtp->tm_sec));
			write(fd4, st2, strlen(st2));
			write(fd4, "]\n", strlen("]\n"));
			write(fd4, "[Hit]", strlen("[Hit]"));
			write(fd4, URL, strlen(URL));
			write(fd4, "\n", strlen("\n"));
			close(fd4);
			closedir(dp);
			return 2;//Hit�� ��� 2�� ��ȯ
		}

	}

	// ���� �̸��� �ؽ� ���丮�� �־����� �ؽ������� ���°�� 
	getHomeDir(dire2);
	strcat(dire2, "/logfile/");
	strcat(dire2, "logfile.txt");
	int fd5;
	fd5 = open(dire2, O_WRONLY | O_APPEND, 0777);

	char st3[100];
	memset(st3, 0, 100);

	write(fd5, "[Miss]", strlen("[Miss]"));

	write(fd5, "ServerPID : ", strlen("ServerPID : "));
	sprintf(st3, "%ld", ((long)getpid()));
	write(fd5, st3, strlen(st3));
	write(fd5, " | ", strlen(" | "));

	write(fd5, URL, strlen(URL));
	write(fd5, "-[", strlen("-["));
	time_t now3;
	struct tm* etp;
	time(&now3);
	etp = localtime(&now3);
	sprintf(st3, "%d", (etp->tm_year + 1900));
	write(fd5, st3, strlen(st3));
	write(fd5, "/", strlen("/"));
	sprintf(st3, "%d", (etp->tm_mon));
	write(fd5, st3, strlen(st3));
	write(fd5, "/", strlen("/"));
	sprintf(st3, "%d", (etp->tm_mday));
	write(fd5, st3, strlen(st3));
	write(fd5, ", ", strlen(", "));
	sprintf(st3, "%d", (etp->tm_hour));
	write(fd5, st3, strlen(st3));
	write(fd5, ":", strlen(":"));
	sprintf(st3, "%d", (etp->tm_min));
	write(fd5, st3, strlen(st3));
	write(fd5, ":", strlen(":"));
	sprintf(st3, "%d", (etp->tm_sec));
	write(fd5, st3, strlen(st3));
	write(fd5, "]\n", strlen("]\n"));
	close(fd5);
	closedir(dp);
	return 1;//Miss�� ��� 1�� ��ȯ


}
void bye(int hitcount, int misscount, int result)
{
	////////////////////////////////////////////////////////////////////////
	// bye													  			  //
	// ===================================================================//
	// ��ǲ: int -> hitcount hit����									  //
	// ��ǲ: int -> misscount miss����									  //  
	// ��ǲ: int -> result ���α׷� ������ �ð�(sec.)					  //						  
	//																	  //
	// �ƿ�ǲ: ����							 			  				  //
	// 																	  //
	// ���Ṯ�� ���Ŀ� �°� �Է��ϴ� �Լ�								  //
	//																	  //
	//																	  //
	//																	  //
	////////////////////////////////////////////////////////////////////////

	char dire3[100];
	getHomeDir(dire3);
	strcat(dire3, "/logfile/");
	strcat(dire3, "logfile.txt");
	int fd5;
	fd5 = open(dire3, O_WRONLY | O_APPEND, 0777);
	// �Ʒ��� ���Ṯ�� ���Ŀ� �°� �Է��ϴ� ����
	char st3[100];
	memset(st3, 0, 100);

	write(fd5, "[Terminated] ", strlen("[Terminated] "));

	write(fd5, "ServerPID : ", strlen("ServerPID : "));
	sprintf(st3, "%ld", ((long)getpid()));
	write(fd5, st3, strlen(st3));
	write(fd5, " | ", strlen(" | "));

	write(fd5, "run time: ", strlen("run time: "));
	sprintf(st3, "%d", result);
	write(fd5, st3, strlen(st3));
	write(fd5, " sec. ", strlen(" sec. "));
	write(fd5, "#request hit : ", strlen("#request hit : "));
	sprintf(st3, "%d", hitcount);
	write(fd5, st3, strlen(st3));
	write(fd5, ", ", strlen(", "));
	write(fd5, "miss : ", strlen("miss : "));
	sprintf(st3, "%d", misscount);
	write(fd5, st3, strlen(st3));
	write(fd5, "\n", strlen("\n"));
	close(fd5);
	return;
}

int sub_server_processing_helper(char* input_url)
{
	///////////////////////////////////////////////////////////////////////
	// sub_server_processing_helper �Լ�											
	// ==================================================================
	// ��ǲ : input_url
	// �ƿ�ǲ : Hit�� ��� 1��ȯ, miss�� ��� 0��ȯ 
	// 																	 
	// ����: Ŭ���̾�Ʈ�� ������ Ȯ�ε� ���� ���ϵ� ���μ��� ��
	// �������μ����� �����ϰ�
	// sub_server_processing�Լ��� ���� cache�� �����ϴ� ���� �������� �Ѵ�.
	// �� �Լ��� ����� 1-2�������� ������ main�Լ��� ����̴�.
	// �������� �ִٸ� �� �Լ��� ����ϴ� ���� �Լ� �� main�Լ��� ���ϵ�
	// ���μ������� Ŭ���̾�Ʈ���� bye��� ���Ṯ�� ������������ ������Ʈ
	// �� �ݺ��ؼ� �ޱ� ������ ������ 1-2�� main�Լ����� �����ѰͰ� �޸� 
	// �� �Լ����� ���� �ݺ�����
	// ������� �ʰ� �� �Լ��� ȣ���ϴ� main�Լ��� ���ϵ����μ�������
	// ������Ʈ�� �ݺ��� ���ؼ� ������ �����ϴ� �������� �ִ�.
	// �� �� �Լ������� cache ���� �Ǵ��� ������ �� �ѹ� ������.                                       		                             
	// 																	 
	// 																	 
	///////////////////////////////////////////////////////////////////////



	char* URL = input_url;//URL�� input_url ���� �Ҵ�


	char HS_URL[100]; // �ؽ�ȭ�� URL�� ������ ���� �˳��ϰ� 100���� �ʱ�ȭ

	char first_HS_URL[4]; // �ؽ�ȭ�� URL���� �������� ���� �տ� 3���ڸ� �����ؼ� ������ ����

	char second_HS_URL[100]; // �ؽ�ȭ�� URL���� ���ϸ��� ���� �տ� 3���� �����ؼ� ������ ����

	int hitcount = 0;
	int misscount = 0;
	time_t start, end;
	time(&start);


	char dire[100]; // ����ϰ��� �ϴ� ���丮 ��ġ�� ������ ����

	getHomeDir(dire);

	strcat(dire, "/cache/");// ���α׷� ������ �� ��Ʈ ���丮�� cache���� �����ϱ����� dire������ ����Ͽ� ���丮 ��ġ ����

	umask(0000);// �ʱ⼳���� umask���� ����
	mkdir(dire, S_IRWXU | S_IRWXG | S_IRWXO);//cache���� ����

	getHomeDir(dire);

	strcat(dire, "/logfile/");// ���α׷� ������ �� ��Ʈ ���丮�� logfile���� �����ϱ����� dire������ ����Ͽ� ���丮 ��ġ ����

	mkdir(dire, S_IRWXU | S_IRWXG | S_IRWXO);//logfile���� ����

	strcat(dire, "logfile.txt");// ���α׷� ������ �� logfile.txt �����ϱ����� dire������ ����Ͽ� ���丮 ��ġ ����

	int fd1;
	fd1 = open(dire, O_WRONLY | O_CREAT | O_EXCL, 0777);//logfile.txt ����
	close(fd1);



	sha1_hash(URL, HS_URL);//SHA1�� �ϱ����� �Է¹��� ���ڿ��� �ƿ�ǲ�� ������ ������ ���ڷ��Ͽ� �Լ� ����.
	int i;
	for (i = 0; HS_URL[i] != '\0'; i++)
	{
		if (i < 3)
		{
			//���丮���� �����ֱ� ���� �ؽ��� URL�� �����Ѵ�.
			first_HS_URL[i] = HS_URL[i];
			first_HS_URL[i + 1] = '\0';
		}

		else
		{
			//���ϸ��� �����ֱ� ���� �ؽ��� URL�� �����Ѵ�.
			second_HS_URL[i - 3] = HS_URL[i];
			second_HS_URL[i - 3 + 1] = '\0';

		}



	}
	if (discrimination(first_HS_URL, second_HS_URL, URL) == 1)//���丮�� ������ ���� �������� ���ڿ��� ���ڷ� �޴� discrimination �Լ� ����
		misscount += 1;//�Լ��� ���Ѱ��� 1�̸� misscount����
	else
		hitcount += 1;// �Լ��� ��ȯ���� 2�̸� hitcount ����

	getHomeDir(dire);
	strcat(dire, "/cache/");
	strcat(dire, first_HS_URL);
	/*���丮�� ������ ��ġ�� ���丮 �̸��� �����Ѵ�.*/

	int num;
	if (0 <= (num = mkdir(dire, S_IRWXU | S_IRWXG | S_IRWXO)))
	{
		/*
		���� �̸��� ���� ���丮�� ���� ��� ���丮�� �����ϰ� �� ���丮�ȿ� ������ ������ �����Ѵ�.
		���� �̸��� ���� ���丮�� ���� ��� if���� �������� �ʴ´�.(���丮�� ����� ���ϵ� ������ ���ܾ� �ϱ�
		������ ���丮�� �ִ� ��� ������ �ʿ������� ���� �� �ۿ� ����.)
		�׷��� ������ ���� ��� ������ �����ϰ� ������ ���� ��� ������ �������� �ʴ� ����� �̷��� ������� �ذ��Ͽ���.
		*/
		strcat(dire, "/");
		strcat(dire, second_HS_URL); //������ ���丮�� ������ �����ϱ����� ���� ���� �����Ѵ�.
		int fd2;
		fd2 = open(dire, O_WRONLY | O_CREAT | O_EXCL, 0777);//������ ���丮�� ������ ����
		close(fd2);


	}
	else
	{
		strcat(dire, "/");
		strcat(dire, second_HS_URL); //�����ϴ� ���丮�� ������ �����ϱ����� ���� ���� �����Ѵ�.
		int num2;
		if (0 <= (num2 = open(dire, O_WRONLY | O_CREAT | O_EXCL, 0777)))//�����ϴ� ���丮�� �� ������ �����Ѵ�.
		{
			close(num2);

		}
		else
		{
			close(num2);
		}
	}

	if (misscount == 1)
	{
		return 0;

	}
	else if (hitcount == 1)
	{
		return 1;
	}


}

static void handler()
{
	pid_t pid;
	int status;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0);

}


int main()
{
	/////////////////////////////////////////////////////////////////////
	// main �Լ�												         
	// ==================================================================
	// 																	 
	// ����: main �Լ��� ���������� ���� �Ǹ� ���� ������ �Լ�����       
	// �̿��Ͽ� �� ������ ������ Construction Proxy Connection�� �����Ѵ�.
	// proxy ������ſ� �־� proxy ���� ������ �Ѵ�.       
	// 																	 
	// 																	 
	///////////////////////////////////////////////////////////////////////
	struct sockaddr_in server_addr, client_addr;
	int socket_fd, client_fd;
	int len, len_out;
	int status;
	pid_t pid;

	int opt = 1;

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("server : Can't open stream socket\n");
		return 0;
	}
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(PORTNO);

	if (bind(socket_fd, (struct socakaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("Server : Can't bind local address\n");
		return 0;
	}
	listen(socket_fd, 5);
	signal(SIGCHLD, (void*)handler);

	while (1)
	{
		struct in_addr inet_client_address;
		char buf[BUFFSIZE] = {0,};
		char response_header[BUFFSIZE] = { 0, };
		char response_message[BUFFSIZE] = { 0, };
		char tmp[BUFFSIZE] = { 0, };
		char method[20] = { 0, };
		char url[BUFFSIZE] = { 0, };
		char * tok = NULL;

		len = sizeof(client_addr);
		client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);
		if (client_fd < 0)
		{
			printf("Server : accept failed\n");
			return 0;
		}
		pid = fork();
		if (pid == -1)
		{
			close(client_fd);
			close(socket_fd);
			continue;
		}
		if (pid == 0)
		{
			//���ϵ� ���μ��� �� ���� ���μ����� �����̴�.
			time_t start, end;
			time(&start);
			inet_client_address.s_addr = client_addr.sin_addr.s_addr;
			printf("[%s : %d] client was connected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
			read(client_fd, buf, BUFFSIZE);
			strcpy(tmp, buf);
			puts("=================================================");
			printf("Request from [%s : %d]\n", inet_ntoa(inet_client_address), client_addr.sin_port);
			puts(buf);
			puts("=================================================");

			tok = strtok(tmp, " ");
			strcpy(method, tok);
			if (strcmp(method, "GET") == 0)// Get �޼ҵ��� request�϶� URL�� ���� cache����
			{
				tok = strtok(NULL, "/");
				tok = strtok(NULL, " ");
				strcpy(url, tok);
				int subprocess_misscount = 0;
				int subprocess_hitcount = 0;
				int check = sub_server_processing_helper(url);// ������ URL�� ���� 1-2�� main�Լ����� ������ ����� sub_server_processing_helper �Լ����� ���ڷ� �Ѱ���
				if (check > 0)
				{
					//Ŭ���̾�Ʈ���� hit�� ������
					subprocess_hitcount++;
					sprintf(response_message,
						"<h1>HIT</h1><br>"
						"%s:%d<br>"
						"kw2018202065", inet_ntoa(inet_client_address), client_addr.sin_port);
					sprintf(response_header,
						"HTTP/1.0 200 OK\r\n"
						"server:2018 simple web server\r\n"
						"Content - length:%lu\r\n"
						"content-type:text/html\r\n\r\n", strlen(response_message));

				}
				else if (check == 0)
				{
					//Ŭ���̾�Ʈ���� miss�� ������
					subprocess_misscount++;
					sprintf(response_message,
						"<h1>MISS</h1><br>"
						"%s:%d<br>"
						"kw2018202065", inet_ntoa(inet_client_address), client_addr.sin_port);

					sprintf(response_header,
						"HTTP/1.0 200 OK\r\n"
						"server:2018 simple web server\r\n"
						"Content - length:%lu\r\n"
						"content-type:text/html\r\n\r\n", strlen(response_message));
				}
				write(client_fd, response_header, strlen(response_header));
				write(client_fd, response_message, strlen(response_message));
				time(&end);
				int result = 0;
				result = (int)(end - start);
				bye(subprocess_hitcount, subprocess_misscount, result);// ���� ���μ����� ���Ṯ�� ����ϱ����� �Լ�
			}
			printf("[%s : %d] client was disconnected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
			close(client_fd);
			exit(0);
		}
		close(client_fd);
	}
	close(socket_fd);
	return 0;
}
