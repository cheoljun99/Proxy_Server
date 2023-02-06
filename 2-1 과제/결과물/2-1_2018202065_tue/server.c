//////////////////////////////////////////////////////////////////////
// File Name : server.c								        
// Date : 2022/04/27												 
// Os : Ubuntu 16.04 LTS 64bits										 
// Author : Park Cheol Jun											 
// Student ID : 2018202065											 
// ----------------------------------------------------------------- 
// Title : System Programming Assignment #2-1 (proxy server)		 
// Description : Socket Programming				         		 
///////////////////////////////////////////////////////////////////////



#include <stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/wait.h>
#include <arpa/inet.h>

#include<fcntl.h> 
#include<dirent.h>
#include<openssl/sha.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<pwd.h>
#include<time.h>

#define BUFFSIZE 1024
#define PORTNO 40000


char* sha1_hash(char* input_url, char* hashed_url) {
	///////////////////////////////////////////////////////////////////////
	// sha1_hash														 //	
	// ==================================================================//
	// 인풋: char* -> input url										     //
	//		 char* -> pre hashed url									 //
	// 아웃풋: char* -> post hashed url									 //
	// 																	 //
	// 목적: 해쉬된 문자열을 반환하는 용도이다.							 //
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
	// 인풋: char* ->pre home										      //
	// 아웃풋: char* -> post home										  //
	// 																	  //
	// 목적: home디렉토리를 출력하기위한 함수이다.						  //
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
	// 인풋: char* ->first_HS_URL										  //
	// 인풋: char* -> second_HS_UR										  //
	// 인풋: char* -> URL												  //
	//																	  //
	// 아웃풋: Hit인경우 2반환											  //
	// 아웃풋: Miss인경우 1반환								 			  //
	// 																	  //
	// 목적: Hit, Miss를 확인하기위한 함수								  //
	//																	  //
	//																	  //
	//																	  //
	////////////////////////////////////////////////////////////////////////
	char dire2[100];
	getHomeDir(dire2);
	strcat(dire2, "/cache/");
	strcat(dire2, first_HS_URL);

	DIR* dp = NULL;// 디렉토리를 열기위해 DIR*형 dp 변수 선언
	struct  dirent* entry = NULL; // 해쉬 디렉토리안에 해쉬 파일을 열기위해 dirent* 구조체 entry변수 선언


	if ((dp = opendir(dire2)) == NULL)// 해쉬 디렉토리가 없으면
	{
		//형식에 맞춰 텍스트 파일에 작성한다.
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
		return 1;//Miss인 경우 1을 반환

	}
	while ((entry = readdir(dp)) != NULL)//해쉬 디렉토리가 있으면
	{

		if (!strcmp(second_HS_URL, entry->d_name))// 해쉬파일이 같은지 확인
		{
			//해쉬파일이 같으면 형식에 맞춰 출력
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
			return 2;//Hit인 경우 2을 반환
		}

	}

	// 같은 이름의 해쉬 디렉토리는 있었지만 해쉬파일이 없는경우 
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
	return 1;//Miss인 경우 1을 반환


}
void bye(int hitcount, int misscount, int result)
{
	////////////////////////////////////////////////////////////////////////
	// bye													  			  //
	// ===================================================================//
	// 인풋: int -> hitcount hit갯수									  //
	// 인풋: int -> misscount miss갯수									  //  
	// 인풋: int -> result 프로그램 실행한 시간(sec.)					  //						  
	//																	  //
	// 아웃풋: 없음							 			  				  //
	// 																	  //
	// 종료문을 형식에 맞게 입력하는 함수								  //
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
	// 아래는 종료문을 형식에 맞게 입력하는 과정
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

int sub_server_processing_helper(char * input_url)
{
	///////////////////////////////////////////////////////////////////////
	// sub_server_processing_helper 함수											
	// ==================================================================
	// 인풋 : input_url
	// 아웃풋 : Hit인 경우 1반환, miss인 경우 0반환 
	// 																	 
	// 목적: 클라이언트의 접속이 확인된 이후 차일드 프로세스 즉
	// 서브프로세스를 생성하고
	// sub_server_processing함수를 통해 cache를 구현하는 것을 목적으로 한다.
	// 이 함수의 기능은 1-2과제에서 구현한 main함수의 기능이다.
	// 차이점이 있다면 위 함수를 사용하는 상위 함수 즉 main함수의 차일드
	// 프로세스에서 클라이언트에게 bye라는 종료문이 들어오기전까지 리퀘스트
	// 를 반복해서 받기 때문에 기존에 1-2의 main함수에서 구현한것과 달리 
	// 위 함수에서 무한 반복문을
	// 사용하지 않고 위 함수를 호출하는 main함수의 차일드프로세스에서
	// 리퀘스트의 반복을 통해서 무한이 실행하는 차이점이 있다.
	// 즉 위 함수에서는 cache 적중 판단의 과정을 단 한번 수행함.                                       		                             
	// 																	 
	// 																	 
	///////////////////////////////////////////////////////////////////////



	char * URL=input_url;//URL을 scanf로 받을 때를 위해 정의 한 변수 넉넉하게 100으로 초기화

	char HS_URL[100]; // 해쉬화된 URL을 저장할 변수 넉넉하게 100으로 초기화

	char first_HS_URL[4]; // 해쉬화된 URL에서 폴더명을 위해 앞에 3글자를 추출해서 저장할 변수

	char second_HS_URL[100]; // 해쉬화된 URL에서 파일명을 위해 앞에 3글자 제외해서 저장할 변수

	int hitcount = 0;
	int misscount = 0;
	time_t start, end;
	time(&start);


	char dire[100]; // 사용하고자 하는 디렉토리 위치를 저장할 변수

	getHomeDir(dire);

	strcat(dire, "/cache/");// 프로그램 시작할 때 루트 디렉토리에 cache폴더 생성하기위해 dire변수를 사용하여 디렉토리 위치 저장

	umask(0000);// 초기설정된 umask값을 변경
	mkdir(dire, S_IRWXU | S_IRWXG | S_IRWXO);//cache폴더 생성

	getHomeDir(dire);

	strcat(dire, "/logfile/");// 프로그램 시작할 때 루트 디렉토리에 logfile폴더 생성하기위해 dire변수를 사용하여 디렉토리 위치 저장

	mkdir(dire, S_IRWXU | S_IRWXG | S_IRWXO);//logfile폴더 생성

	strcat(dire, "logfile.txt");// 프로그램 시작할 때 logfile.txt 생성하기위해 dire변수를 사용하여 디렉토리 위치 저장

	int fd1;
	fd1 = open(dire, O_WRONLY | O_CREAT | O_EXCL, 0777);//logfile.txt 생성
	close(fd1);



	sha1_hash(URL, HS_URL);//SHA1을 하기위해 입력받은 문자열과 아웃풋을 저장할 변수를 인자로하여 함수 실행.
	int i;
	for (i = 0; HS_URL[i] != '\0'; i++)
	{
		if (i < 3)
		{
			//디렉토리명을 정해주기 위해 해쉬된 URL을 분해한다.
			first_HS_URL[i] = HS_URL[i];
			first_HS_URL[i + 1] = '\0';
		}

		else
		{
			//파일명을 정해주기 위해 해쉬된 URL을 분해한다.
			second_HS_URL[i - 3] = HS_URL[i];
			second_HS_URL[i - 3 + 1] = '\0';

		}



	}
	if (discrimination(first_HS_URL, second_HS_URL, URL) == 1)//디렉토리와 파일을 위해 나눠놓은 문자열을 인자로 받는 discrimination 함수 실행
		misscount += 1;//함수의 반한값이 1이면 misscount증가
	else
		hitcount += 1;// 함수의 반환값이 2이면 hitcount 증가

	getHomeDir(dire);
	strcat(dire, "/cache/");
	strcat(dire, first_HS_URL);
	/*디렉토리를 저장할 위치와 디렉토리 이름을 저장한다.*/

	int num;
	if (0 <= (num = mkdir(dire, S_IRWXU | S_IRWXG | S_IRWXO)))
	{
		/*
		같은 이름을 가진 디렉토리가 없을 경우 디렉토리를 생성하고 그 디렉토리안에 파일을 무조건 생성한다.
		같은 이름을 가진 디렉토리가 있을 경우 if문을 실행하지 않는다.(디렉토리가 생기면 파일도 무조건 생겨야 하기
		때문에 디렉토리가 있는 경우 파일은 필연적으로 있을 수 밖에 없다.)
		그래서 파일이 없을 경우 파일을 생성하고 파일이 있을 경우 파일을 생성하지 않는 방법을 이러한 방식으로 해결하였다.
		*/
		strcat(dire, "/");
		strcat(dire, second_HS_URL); //생성한 디렉토리에 파일을 생성하기위해 파일 명을 저장한다.
		int fd2;
		fd2 = open(dire, O_WRONLY | O_CREAT | O_EXCL, 0777);//생성한 디렉토리에 파일을 생성
		close(fd2);


	}
	else
	{
		strcat(dire, "/");
		strcat(dire, second_HS_URL); //존재하는 디렉토리에 파일을 생성하기위해 파일 명을 저장한다.
		int num2;
		if (0 <= (num2 = open(dire, O_WRONLY | O_CREAT | O_EXCL, 0777)))//존재하는 디렉토리에 새 파일을 생성한다.
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
	else if(hitcount== 1)
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
	// main 함수												         
	// ==================================================================
	// 																	 
	// 목적: main 함수로 리눅스에서 실행 되며 위에 정의한 함수들을       
	// 이용하여 본 과제의 목적인 socket programing을 구현한다.
	// proxy 서버통신에 있어 서버 역할을 한다.       
	// 																	 
	// 																	 
	///////////////////////////////////////////////////////////////////////
	struct sockaddr_in server_addr, client_addr;
	int socket_fd, client_fd;
	int len, len_out;
	int state;
	char buf[BUFFSIZE];
	pid_t pid;

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("server : Can't open stream socket\n");
		return 0;

	}
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNO);

	if (bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("server : Can't bind local address\n");
		close(socket_fd);
		return 0;

	}
	listen(socket_fd, 5);
	signal(SIGCHLD, (void*)handler);
	while (1)
	{
		bzero((char*)&client_addr, sizeof(client_addr));
		len = sizeof(client_addr);
		client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);//클라이언트의 컨넥트를 대기한다.
		

		if (client_fd < 0)
		{
			printf("Server : accept failed %d\n", getpid());
			close(socket_fd);
			return 0;

		}

		printf("[%s : %d] client was connected\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
		pid = fork();//클라이언트가 컨넥트 되면 차일드 프로세스생성 즉 서브 프로세스생성. 다중 클라이언트를 위해 사용이 가능하다.

		if (pid == -1)
		{
			close(client_fd);
			close(socket_fd);
			continue;
		}
		if (pid == 0)
		{
			//차일드 프로세스 즉 서브 프로세스의 실행이다.
			time_t start, end;
			time(&start);

			int subprocess_misscount = 0;
			int subprocess_hitcount = 0;
			while ((len_out = read(client_fd, buf, BUFFSIZE)) > 0)// bye 종료문이 들어올 때 까지 클라이언트에게 리퀘스트를 받음 
			{
				char* original_buf = buf;
				char * modified_buf = strtok(buf, "\n");
		
				if (!strcmp(modified_buf, "bye"))
				{
					break;

				}
				int check = sub_server_processing_helper(modified_buf);
				if (check> 0)
				{
					subprocess_hitcount++;
					write(client_fd, "HIT\n", sizeof("HIT\n")); //클라이언트에게 hit을 전달함
				}
				else if (check == 0)
				{
					subprocess_misscount++;
					write(client_fd, "MISS\n", sizeof("MISS\n"));//클라이언트에게 miss를 전달함
				}

				
				


			}
			time(&end);
			int result = 0;
			result = (int)(end - start);
			bye(subprocess_hitcount, subprocess_misscount, result);// 서브 프로세스의 종료문을 출력하기위한 함수
			printf("[%s : %d] client was disconnected\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
			close(client_fd);
			exit(0);

		}
		close(client_fd);


	}
	close(socket_fd);

	return 0;
}

