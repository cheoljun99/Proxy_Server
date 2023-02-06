//////////////////////////////////////////////////////////////////////
// File Name : proxy_cache.c								        
// Date : 2022/06/01												 
// Os : Ubuntu 16.04 LTS 64bits			
// Author : Park Cheol Jun											 
// Student ID : 2018202065											 
// ----------------------------------------------------------------- 
// Title : System Programming Assignment #3-2 (proxy server)		 
// Description :
// logging using threads.
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
#include<errno.h>
#include<netdb.h>
#include<signal.h>
#include<stdbool.h>
#include <sys/select.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<pthread.h>

#define BUFFSIZE 1024
#define PORTNO 39999

//스레드 함수에 인자값으로 넣어줄 데이터를 구조체를 정의하여 사용
typedef struct passing_thread_data_struct
{
	//쓰레드 함수에서 사용할 변수들을 구성하는 구조체이며
	//이는 서브 프로세스에서 구조체 변수를 선언하고 저장하여
	//쓰레드 함수에서 쓰인다.
	char passing_thread_data_hash_passing_url[BUFFSIZE]; // URL을 저장하는 변수
	int passing_thread_data_subprocess_misscount; // miss개수를 저장하는 변수
	int passing_thread_data_subprocess_hitcount ; // hit 개수를 저장하는 변수
	int passing_thread_data_hit_or_miss_check ; // hit인지 miss인지 판단한 데이터를 저장할 변수
	int passing_thread_data_result; // 종료시간을 저장할 변수

}passing_thread_data_struct;// 예명으로 사용한다.

int process_count = 0;//프로세스 개수를 위한 변수 (전역변수로 설정)
time_t global_start, global_end;// 프로그램 자체 시간을 위한 변수 (전역변수로 설정)
long global_processid; // 부모 프로세스 아이디 저장을 위한 변수 (전역변수로 설정) 
int semid; //부모프로세스에서 세마포어 아이디를 저장할 것이고 이를 전역변수가 받아 프로세스 모두가 사용가능하게 한다.

union semum //세마포어 사용 값들이 모인 연합
{
	int val;
	struct semid_ds* buf;
	unsigned short int* array;

}arg;

void p(int semid)
{
	struct sembuf pbuf;//세마포어 구조체 선언
	pbuf.sem_num = 0;// 세마포어 구조체에서 세마포어 순서 설정
	pbuf.sem_op = -1;//세마포어 값을 –1하는 연산 설정
	pbuf.sem_flg = SEM_UNDO;//프로그램이 종료되어도 자동으로 세마포어 삭제
	if ((semop(semid, &pbuf, 1)) == -1) {
		// 세마포어 값을 음수로 만들어 세마포어를 사용중으로 바꿈 크리티컬 섹션 이용 불가능
		perror("p : semop failed");
		exit(1);
	}
}

void v(int semid)
{
	struct sembuf vbuf;//세마포어 구조체 선언
	vbuf.sem_num = 0;// 세마포어 구조체에서 세마포어 순서 설정
	vbuf.sem_op = 1; // 세마포어 값을 +1하는 연산 설정
	vbuf.sem_flg = SEM_UNDO;//프로그램이 종료되어도 자동으로 세마포어 삭제
	if ((semop(semid, &vbuf, 1)) == -1)
	{
		//세마포어 값을 양수로 만들어 세마포어를 사용가능으로 바꿈 크리티컬 섹션 사용 가능
		perror("v : semop failed");
		exit(1);
	}
}

char* sha1_hash(char* input_url, char* hashed_url) {
	/////////////////////////////////////////////////////////////////////
	// sha1_hash														 	
	// ==================================================================
	// 인풋: char* -> input url										     
	//		 char* -> pre hashed url									 
	// 아웃풋: char* -> post hashed url									 
	// 																	 
	// 목적: 해쉬된 문자열을 반환하는 용도이다.							 
	// 																	 
	// 																	 
	/////////////////////////////////////////////////////////////////////

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
	//////////////////////////////////////////////////////////////////////
	// getHomeDir														  
	// ===================================================================
	// 인풋: char* ->pre home										      
	// 아웃풋: char* -> post home										  
	// 																	  
	// 목적: home디렉토리를 출력하기위한 함수이다.						  
	// 																	  
	// 																	  
	//////////////////////////////////////////////////////////////////////

	struct passwd* usr_info = getpwuid(getuid());

	strcpy(home, usr_info->pw_dir);

	return home;

}

int discrimination(char* first_HS_URL, char* second_HS_URL, char* URL)
{
	//////////////////////////////////////////////////////////////////////
	// discrimination													  
	// ===================================================================
	// 인풋: char* ->first_HS_URL										  
	// 인풋: char* -> second_HS_UR										  
	// 인풋: char* -> URL												  
	//																	  
	// 아웃풋: Hit인경우 2반환											  
	// 아웃풋: Miss인경우 1반환								 			  
	// 																	  
	// 목적: Hit, Miss를 확인하기위한 함수								  
	//																	  
	//																	  
	//																	  
	//////////////////////////////////////////////////////////////////////
	char dire2[100];
	getHomeDir(dire2);
	strcat(dire2, "/cache/");
	strcat(dire2, first_HS_URL);

	DIR* dp = NULL;// 디렉토리를 열기위해 DIR*형 dp 변수 선언
	struct  dirent* entry = NULL; // 해쉬 디렉토리안에 해쉬 파일을 열기위해 dirent* 구조체 entry변수 선언


	if ((dp = opendir(dire2)) == NULL)// 해쉬 디렉토리가 없으면
	{
		
		return 1;//Miss인 경우 1을 반환
		

	}
	while ((entry = readdir(dp)) != NULL)//해쉬 디렉토리가 있으면
	{

		if (!strcmp(second_HS_URL, entry->d_name))// 해쉬파일이 같은지 확인
		{
			
			return 2;//Hit인 경우 2을 반환
			
		}

	}
	// 같은 이름의 해쉬 디렉토리는 있었지만 해쉬파일이 없는경우 
	return 1;//Miss인 경우 1을 반환

}
void bye_browser_subprocess(int hitcount, int misscount, int result)
{
	//////////////////////////////////////////////////////////////////////
	// bye_browser_subprocess													  			  
	// ===================================================================
	// 인풋: int -> hitcount hit갯수									  
	// 인풋: int -> misscount miss갯수									    
	// 인풋: int -> result 프로그램 실행한 시간(sec.)					  						  
	//																	  
	// 아웃풋: 없음							 			  				  
	// 																	  
	// 종료문을 형식에 맞게 입력하는 함수								  
	//																	  
	//																	  
	//																	  
	//////////////////////////////////////////////////////////////////////

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

int sub_server_processing_helper(char* input_url)
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

	char* URL = input_url;//URL에 input_url 인자 할당
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
	else if (hitcount == 1)
	{
		return 1;
	}
}

void remove_first_char(char* buf)
{
	//첫글자를 지워주는 함수
	//리턴값 없음

	int i = 0;
	for (i = 1; buf[i]; i++)//buf[i]가 참(널문자가 아님)이면 반복하여라.
	{
		buf[i - 1] = buf[i]; //buf[i] 문자를 buf[i-1]로 이동
	}
	//현재 i는 널문자가 있는 위치, i-1은 마지막 문자 위치
	buf[i - 1] = '\0';
}

char* get_ip_addr(char* addr)
{
	//올바른 URL입력시 ip를 가져오는 함수
	struct hostent* hent;
	char* haddr=NULL;
	int len = strlen(addr);

	if ((hent = (struct hostent*)gethostbyname(addr)) != NULL)
	{
		haddr = inet_ntoa(*((struct in_addr*)hent->h_addr_list[0]));

	}

	return haddr;
}
void bye_program(int global_result)
{
	//////////////////////////////////////////////////////////////////////
	// bye_program													  			  
	// ===================================================================
	// 							    
	// 인풋: int -> bye_program 프로그램 실행한 시간(sec.)					  						  
	//																	  
	// 아웃풋: 없음							 			  				  
	// 																	  
	// 프로그램 종료문을 형식에 맞게 입력하는 함수								  
	//																	  
	//																	  
	//																	  
	//////////////////////////////////////////////////////////////////////

	char dire[100];
	getHomeDir(dire);
	strcat(dire, "/logfile/");
	strcat(dire, "logfile.txt");
	int fd;
	fd = open(dire, O_WRONLY | O_APPEND, 0777);
	// 아래는 종료문을 형식에 맞게 입력하는 과정
	char st[100];
	memset(st, 0, 100);

	write(fd, "**SERVER** [Terminated] run time: ", strlen("**SERVER** [Terminated] run time: "));
	sprintf(st, "%d", global_result);
	write(fd, st, strlen(st));
	write(fd, " sec. ", strlen(" sec. "));
	write(fd, "#sub process: ", strlen("#sub process: "));
	sprintf(st, "%d", process_count);
	write(fd, st, strlen(st));
	write(fd, "\n", strlen("\n"));
	close(fd);
	return;
}

static void handler(int sig)
{
	//핸들러 함수
	if (sig == SIGCHLD)
	{
		//차일드 프로세스가 종료되었을 때
		//차일드 프로세스가 잘 죽었는지 확인을 위한 출력문 //printf("================child die=========================\n");
		pid_t pid;
		int status;
		while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
	}

	if (sig == SIGALRM)
	{
		printf("================No Responese=========================\n");
		//핸들러에서 시그알람일 때 만약 서브프로세스를 종료한다면 사용//bye_browser_subprocess(0,1,10);
		//핸들러에서 시그알람일 때 만약 서브프로세스를 종료한다면 사용//exit(0);
	}

	if (sig ==SIGINT)
	{
		long detect_processid = (long)getpid();//현재 프로세스 아이디를 저장
		//현재 프로세스 아이디를 출력하는 출력문 //printf("detect_processid: %ld \n", detect_processid);
		if (detect_processid != global_processid)// 전역변수로 저장한 부모 프로세스 아이디와 비교 해서 다르면  
		{
			//자식 프로세스임으로 종료!
			exit(0);
		}
		time(&global_end);
		int global_result = 0;
		global_result = (int)(global_end - global_start);// 프로그램 동작 시간 기록
		printf("\n");
		bye_program(global_result);//프로그램 종료문 출력!

		if ((semctl(semid, 0, IPC_RMID, arg)) == -1)
		{
			perror("semctl failed");
			exit(1);

		}
		else
		{
			// 시그인트의 경우 세마포어를 삭제 하였는지 확인하기위한 출력문 //printf("semaphore delete\n");
		}

		exit(0);//부모 프로세스 즉 프로그램 종료!
	}

}


int sub_process_webserver_communication(char * host_url, char * request)
{
	///////////////////////////////////////////////////////////////////////
	// sub_process_webserver_communication 함수											
	// ==================================================================
	// 인풋 : host_url, request
	// 아웃풋 : 정상인 경우 웹서버의 파일 디스크립터값을 반환 오류일 경우 -1
	// 																	 
	// 목적:차일드 프로세스가 서버와 통신을 할 수 있게 하는 함수이다. 
	// 차일드 프로세스 즉 프록시 서버가 클라이언트가 되어 웹서버에 컨넥트
	// 를 요청하고 request를 보내고 웹서버로 부터 requst를 받아냄 
	//                                 		                             
	// 																	 
	// 																	 
	///////////////////////////////////////////////////////////////////////

	int server_fd, len;
	int error = 0;
	char buf[BUFFSIZE] = { 0, };
	struct sockaddr_in server_addr;
	char* haddr = NULL;	
	haddr=get_ip_addr(host_url);

	if (haddr == NULL)//URL에서 아이피를 가져올 수 없는 경우
	{
		//fail to accept address for webserver.
		return -1;
	}
	
	if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)//웹서버 소켓을 만들 수  없는 경우
	{
		//can't create socket.
		return -1;

	}

	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(haddr);
	server_addr.sin_port = htons(80);
	
	if (connect(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		//can't connect.
		close(server_fd);
		return -1;
	}
	else
	{
		//connect된걸 확인하고 랜선을 끊기 위해
		//connect된것을 확인하는 출력문 //printf("connect success.\n");
		return server_fd;
	}
	return server_fd;
}

int read_with_timeout(int fd, char* buf, int buf_size, int timeout_ms)
{
	//웹서버 소켓을 통해 사용하는 read를 타임아웃 설정을 하여
	//read_with_timeout으로 재정의
	int rx_len = 0;
	struct    timeval  timeout;
	fd_set  	readFds;

	// recive time out config
	// Set 1ms timeout counter
	timeout.tv_sec = 0;
	timeout.tv_usec = timeout_ms * 1000;

	FD_ZERO(&readFds);
	FD_SET(fd, &readFds);
	select(fd + 1, &readFds, NULL, NULL, &timeout);

	if (FD_ISSET(fd, &readFds))
	{
		rx_len = read(fd, buf, buf_size);
	}

	return rx_len;
}


void write_log_about_hit_or_miss(char * host_url, int hit_or_miss_check)
{
	//로그파일에 hit or miss를 입력하기 위한 함수
	//인자로 호스트 유알엘과 힛 혹은 미스 개수를 입력받는다
	char* URL = host_url;//URL에 input_url 인자 할당
	char HS_URL[100]; // 해쉬화된 URL을 저장할 변수 넉넉하게 100으로 초기화
	char first_HS_URL[4]; // 해쉬화된 URL에서 폴더명을 위해 앞에 3글자를 추출해서 저장할 변수
	char second_HS_URL[100]; // 해쉬화된 URL에서 파일명을 위해 앞에 3글자 제외해서 저장할 변수
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

	char dire[100];
	getHomeDir(dire);
	strcat(dire, "/cache/");
	strcat(dire, first_HS_URL);

	if (hit_or_miss_check > 0)
	{
		//hit인 경우임으로 로그파일에 hit을 기록
		getHomeDir(dire);
		strcat(dire, "/logfile/");
		strcat(dire, "logfile.txt");
		int fd;
		fd = open(dire, O_WRONLY | O_APPEND, 0777);

		char st[100];
		memset(st, 0, 100);

		write(fd, "[Hit]", strlen("[Hit]"));

		write(fd, "ServerPID : ", strlen("ServerPID : "));
		sprintf(st, "%ld", ((long)getpid()));
		write(fd, st, strlen(st));
		write(fd, " | ", strlen(" | "));

		write(fd, first_HS_URL, strlen(first_HS_URL));
		write(fd, "/", strlen("/"));
		write(fd, second_HS_URL, strlen(second_HS_URL));
		write(fd, "-[", strlen("-["));
		time_t now2;
		struct tm* gtp;
		time(&now2);
		gtp = localtime(&now2);
		sprintf(st, "%d", (gtp->tm_year + 1900));
		write(fd, st, strlen(st));
		write(fd, "/", strlen("/"));
		sprintf(st, "%d", (gtp->tm_mon + 1));
		write(fd, st, strlen(st));
		write(fd, "/", strlen("/"));
		sprintf(st, "%d", (gtp->tm_mday));
		write(fd, st, strlen(st));
		write(fd, ", ", strlen(", "));
		sprintf(st, "%d", (gtp->tm_hour));
		write(fd, st, strlen(st));
		write(fd, ":", strlen(":"));
		sprintf(st, "%d", (gtp->tm_min));
		write(fd, st, strlen(st));
		write(fd, ":", strlen(":"));
		sprintf(st, "%d", (gtp->tm_sec));
		write(fd, st, strlen(st));
		write(fd, "]\n", strlen("]\n"));
		write(fd, "[Hit]", strlen("[Hit]"));
		write(fd, URL, strlen(URL));
		write(fd, "\n", strlen("\n"));
		close(fd);
		

	}
	else if (hit_or_miss_check == 0)
	{
		//miss인 경우임으로 로그파일에 miss를 기록
		getHomeDir(dire);
		strcat(dire, "/logfile/");
		strcat(dire, "logfile.txt");
		int fd;
		fd = open(dire, O_WRONLY | O_APPEND, 0777);
		char st[100];
		memset(st, 0, 100);
		write(fd, "[Miss] ", strlen("[Miss] "));

		write(fd, "ServerPID : ", strlen("ServerPID : "));
		sprintf(st, "%ld", ((long)getpid()));
		write(fd, st, strlen(st));
		write(fd, " | ", strlen(" | "));


		write(fd, URL, strlen(URL));
		write(fd, "-[", strlen("-["));
		time_t now;
		struct tm* ltp;
		time(&now);
		ltp = localtime(&now);
		sprintf(st, "%d", (ltp->tm_year + 1900));
		write(fd, st, strlen(st));
		write(fd, "/", strlen("/"));
		sprintf(st, "%d", (ltp->tm_mon + 1));
		write(fd, st, strlen(st));
		write(fd, "/", strlen("/"));
		sprintf(st, "%d", (ltp->tm_mday));
		write(fd, st, strlen(st));
		write(fd, ", ", strlen(", "));
		sprintf(st, "%d", (ltp->tm_hour));
		write(fd, st, strlen(st));
		write(fd, ":", strlen(":"));
		sprintf(st, "%d", (ltp->tm_min));
		write(fd, st, strlen(st));
		write(fd, ":", strlen(":"));
		sprintf(st, "%d", (ltp->tm_sec));
		write(fd, st, strlen(st));
		write(fd, "]\n", strlen("]\n"));
		close(fd);
	}


}


void* thread_function(void* passing_thread_data)
{
	//스레드함수이며
	//인자는 void형 포인터 자료형 이는 구조체 변수의 void*형으로 타입 케스트를 통해
	//인자로 넘겨줄 것이다.

	printf("*PID# %ld create the *TID# %ld.\n", (long)getpid(), (long)pthread_self());
	passing_thread_data_struct* passing_thread_data_in_thread_function
		=(passing_thread_data_struct*)passing_thread_data;

	//Hit or miss를 저장할 함수
	write_log_about_hit_or_miss(
		passing_thread_data_in_thread_function->passing_thread_data_hash_passing_url,
		passing_thread_data_in_thread_function->passing_thread_data_hit_or_miss_check);

	// 서브 프로세스의 종료문을 출력하기위한 함수
	bye_browser_subprocess(passing_thread_data_in_thread_function->passing_thread_data_subprocess_hitcount,
		passing_thread_data_in_thread_function->passing_thread_data_subprocess_misscount,
		passing_thread_data_in_thread_function->passing_thread_data_result);


	printf("*TID# %ld is exited.\n", (long)pthread_self());


}



int main()
{
	/////////////////////////////////////////////////////////////////////
	// main 함수												         
	// ==================================================================
	// 																	 
	// 목적: main 함수로 리눅스에서 실행 되며 위에 정의한 함수들을       
	// 이용하여 본 과제의 목적인 Construction Proxy Connection을 구현한다.
	// proxy 서버통신에 있어 proxy 서버 역할을 한다.       
	// 																	 
	// 																	 
	///////////////////////////////////////////////////////////////////////
	time(&global_start);
	global_processid = (long)getpid();
	//부모프로세스의 아이디 즉 프로그램 아이디를 출력하는 출력문 //printf("global_processid : %ld \n", global_processid);

	pthread_t thread_id;//스레드 아이디를 생성한다.
	int err; //스레드 생성에 실패하면 에러를 저장하기 위해 생성
	void* thread_return; //스레드 함수의 리턴값을 저장할 변수

	/* semaphore를 지역변수로 사용할 때 
	int semid;
	union semum
	{
		int val;
		struct semid_ds* buf;
		unsigned short int* array;


	}arg;
	*/

	if ((semid = semget((key_t)PORTNO, 1, IPC_CREAT | 0666)) == -1)
	{
		perror("semget failed");
		exit(1);
	}
	arg.val = 1;
	if ((semctl(semid, 0, SETVAL, arg)) == -1)
	{
		perror("semctl failed");
		exit(1);
	}

	char dire[100]; // 사용하고자 하는 디렉토리 위치를 저장할 변수
	getHomeDir(dire);
	strcat(dire, "/logfile/");// 프로그램 시작할 때 루트 디렉토리에 logfile폴더 생성하기위해 dire변수를 사용하여 디렉토리 위치 저장
	mkdir(dire, S_IRWXU | S_IRWXG | S_IRWXO);//logfile폴더 생성
	strcat(dire, "logfile.txt");// 프로그램 시작할 때 logfile.txt 생성하기위해 dire변수를 사용하여 디렉토리 위치 저장
	int fd;
	fd = open(dire, O_WRONLY | O_CREAT | O_EXCL, 0777);//logfile.txt 생성
	close(fd);


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

	if (bind(socket_fd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("Server : Can't bind local address\n");
		return 0;
	}

	listen(socket_fd, 5);
	signal(SIGCHLD, (void*)handler);
	signal(SIGALRM, (void*)handler);
	signal(SIGINT, (void*)handler);

	while (1)
	{
		struct in_addr inet_client_address;
		char buf[BUFFSIZE] = {0,};
		char response_header[BUFFSIZE] = { 0, };
		char response_message[BUFFSIZE] = { 0, };
		char tmp[BUFFSIZE] = { 0, };
		char method[20] = { 0, };
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
			//차일드 프로세스 즉 서브 프로세스의 실행이다.
			time_t start, end;
			time(&start);
			inet_client_address.s_addr = client_addr.sin_addr.s_addr;
			//printf("[%s : %d] client was connected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
			read(client_fd, buf, BUFFSIZE);
			strcpy(tmp, buf);
			//puts("=================================================");
			//printf("Request from [%s : %d]\n", inet_ntoa(inet_client_address), client_addr.sin_port);
			//puts(buf);
			//puts("=================================================");

			tok = strtok(tmp, " ");
			strcpy(method, tok);
			if (strcmp(method, "GET") == 0)// Get 메소드의 request일때 URL을 통해 cache생성
			{

				char hash_passing_url[BUFFSIZE] = { 0, };
				char getip_passing_url[BUFFSIZE] = { 0, };
				int subprocess_misscount = 0;
				int subprocess_hitcount = 0;
				int hit_or_miss_check = 0;
				int result = 0;
				passing_thread_data_struct passing_thread_data;

				tok = strtok(NULL, "/");
				tok = strtok(NULL, " ");
				strcpy(hash_passing_url, tok);

				remove_first_char(hash_passing_url);
				remove_first_char(tok);
				tok = strtok(tok, "/");
				strcpy(getip_passing_url, tok);
				//hash_passing_url이 첫 글자 /가 사라졋는지 확인을 위한 출력문 //printf("hash_passing_url is %s\n",hash_passing_url);
				//getip_passing_url가 정확인 tok 됬는지 확인을 위한 출력문 //printf("getip_passing_url is %s\n", getip_passing_url);

				/*
				if (strcmp(tok, "detectportal.firefox.com")==0)
				{
					//테스트를 위한 예외처리

					//printf("[%s : %d] client was disconnected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
					close(client_fd);
					exit(0);

				}
				*/

				 hit_or_miss_check = sub_server_processing_helper(hash_passing_url);// 추출한 URL을 과제 1-2의 main함수였지만 현재는 sub_server_processing_helper 함수에게 인자로 넘겨줌


				if (hit_or_miss_check > 0)
				{
					//Hit일 경우
					//클라이언트에게 hit을 전달함
					subprocess_hitcount++;
					char dire[100];
					char HS_URL[100];
					char first_HS_URL[4];
					char second_HS_URL[100];
					sha1_hash(hash_passing_url, HS_URL);//SHA1을 하기위해 입력받은 문자열과 아웃풋을 저장할 변수를 인자로하여 함수 실행.
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
					getHomeDir(dire);
					strcat(dire, "/cache/");
					strcat(dire, first_HS_URL);
					strcat(dire, "/");
					strcat(dire, second_HS_URL); //존재하는 디렉토리에 파일을 생성하기위해 파일 명을 저장한다.
					int cache_file_fd = 0;
					printf("*PID# %ld is waiting for the semaphore.\n", (long)getpid());
					p(semid);
					printf("*PID# %ld is in the critical zone.\n", (long)getpid());
					//sleep(2);
					if ((cache_file_fd = open(dire, O_RDWR | O_APPEND,0777)) >= 0)
					{
						//디렉토리 내부에 해쉬된 파일이 있는지 확인을 위한 출력문 성공일 경우 //printf("open succes\n");
					}
					else
					{
						//실패일 경우
						printf("open error\n");
						close(cache_file_fd);
					}
			
					int read_num = 0;
					while ((read_num = read(cache_file_fd, buf, BUFFSIZE)) > 0)
					{
						//recieve HTTP
						//read의 반환값을 출력하기 위한 출력문 //printf("read_num is %d\n", read_num);
						write(client_fd, buf, read_num);
						bzero(buf, BUFFSIZE);
					}
					close(cache_file_fd);
					time(&end);
					result = (int)(end - start);

					strcpy(passing_thread_data.passing_thread_data_hash_passing_url, hash_passing_url);
					passing_thread_data.passing_thread_data_hit_or_miss_check = hit_or_miss_check;
					passing_thread_data.passing_thread_data_subprocess_hitcount = subprocess_hitcount;
					passing_thread_data.passing_thread_data_subprocess_misscount = subprocess_misscount;
					passing_thread_data.passing_thread_data_result = result;

					err = pthread_create(&thread_id, NULL, thread_function, (void*)&passing_thread_data);//쓰레드 아이디 변수에 쓰레드 생성후 해당 아이디 담기
					if (err != 0)
					{
						printf("pthread_create() error.\n");
						continue;

					}
					/*
					else
					{
						//디버깅을 위해서 
						printf("miss일때 메인함수에서 pthread_create 후에 메인에서 출력!\n");
						printf("서브프로세스에서 출력한 생성한 *TID# %ld.\n", (long)thread_id);
						printf("서브프로세스에서 출력한 자신의 *TID# %ld.\n", (long)pthread_self());
					}
					*/
					pthread_join(thread_id, &thread_return);// 생성한 해당 쓰레드 아이디가 종료되길 기다린다. 
					printf("*PID# %ld exited the critical zone.\n", (long)getpid());
					v(semid);
				}
				else if (hit_or_miss_check == 0)
				{
					//클라이언트에게 miss를 전달함
					subprocess_misscount++;
					int server_fd ;
					if ((server_fd=sub_process_webserver_communication(getip_passing_url, buf)) < 0)//웹서버와 통신을 하는 함수!!!!
					{
						//웹서버와 통신의 실패를 출력하는 함수 //printf("sub process fails webserver comunication in read or connect function and get ip addr\n");
					}
					else
					{
						char dire[100];
						char HS_URL[100];
						char first_HS_URL[4];
						char second_HS_URL[100];
						sha1_hash(hash_passing_url, HS_URL);//SHA1을 하기위해 입력받은 문자열과 아웃풋을 저장할 변수를 인자로하여 함수 실행.
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
						getHomeDir(dire);
						strcat(dire, "/cache/");
						strcat(dire, first_HS_URL);
						strcat(dire, "/");
						strcat(dire, second_HS_URL);
						FILE* pFile =0;
						int cache_file_fd = 0;

						printf("*PID# %ld is waiting for the semaphore.\n", (long)getpid());
						p(semid);
						printf("*PID# %ld is in the critical zone.\n", (long)getpid());
						//sleep(2);
						if ((cache_file_fd = open(dire, O_RDWR | O_APPEND,0777)) >= 0)
						{
							//캐시파일의 파일 오픈이 되는지 확인을 위한 출력문 //printf("open succes\n");
							close(cache_file_fd);
						}
						else
						{
							printf("open error\n");
							close(cache_file_fd);
						}
						//핸들러가 실행할 때 대기상태를 벗어나기위한 부분
						struct sigaction act, oact;
						act.sa_handler = handler;
						sigemptyset(&act.sa_mask);
						act.sa_flags &= ~SA_RESTART;
						sigaction(SIGALRM, &act, &oact);
		
						write(server_fd, buf, strlen(buf));

						alarm(15);//알람 15초 설정

						bzero(buf, BUFFSIZE);
						int read_num = 0;
						int read_num_sum=0;
						int check_num = 0;
						int Content_Length_int = 0;
						int Content_Length_before_entity_body = 0;
						bool chunck_check = false;
						int newline_count = 0;

						/* 
						//논블럭킹 설정 이번 과제에서는 사용안함
						if (fcntl(server_fd, F_SETFL, fcntl(server_fd, F_GETFL) | O_NONBLOCK) < 0)
						{
							handle error
						}
						*/

						while ((read_num= read_with_timeout(server_fd, buf, BUFFSIZE,5000))>0)
						{
							//recieve HTTP
							alarm(0);// read값을 반환하여 while문을 실행 시킨경우 알람해제

							read_num_sum += read_num;
							check_num += 1;
							
							if (check_num == 1)
							{
								char tmp1[BUFFSIZE] = { 0, };
								char tmp2[BUFFSIZE] = { 0, };
								char* tok1 = NULL;
								char* tok2 = NULL;
								strcpy(tmp1, buf);
								strcpy(tmp2, buf);
								tok1 = strtok(tmp1, "\n");
								Content_Length_before_entity_body += strlen(tok1);
								newline_count++;
								while (strcmp(tok1, "\r") != 0)
								{
									tok1 = strtok(NULL, "\n");
									Content_Length_before_entity_body += strlen(tok1);
									newline_count++;
								}
								tok2 = strtok(tmp2, " ");
								while(strcmp(tok2, "Content-Length:") != 0)// Content-Length: 글자 탐색
								{
									if (strcmp(tok2, "Transfer-Encoding:") == 0)//Transfer-Encoding: 글자 탐색
									{
										chunck_check = true; 
										break;
									}
									tok2 = strtok(NULL, "\n");
									tok2 = strtok(NULL, " ");
								}
								tok2 = strtok(NULL, "\r");
								char* Content_Length_char = NULL;
								Content_Length_char = tok2;
								Content_Length_int = atoi(Content_Length_char);
								//컨텐츠 길이의 숫자가 알맞게 추출되었는지 확인을 위한 출력문 //printf("Content_Length_int is %d \n", Content_Length_int);
								//entity body 전까지의 리스폰스 메세지의 길이를 알맞게 계산했는 지 확인을 위한 출력문 //printf("Content_Length_before_entity_body is %d \n", Content_Length_before_entity_body);
					
							}
							//read의 반환값을 출력을 위한 출력문 //printf("read_num is %d\n", read_num);
							write(client_fd, buf, read_num);
							pFile = fopen(dire,"ab");
							fwrite(buf,1, read_num,pFile);
							fclose(pFile);
							bzero(buf, BUFFSIZE);
							if ((Content_Length_int + Content_Length_before_entity_body+ newline_count <= read_num_sum)&&(chunck_check==false))
							{

								/*
								//추출한 read_num의 길이의 합이 엔티티 바디 길이 포함 헤더 길이 포함과 같은지 확인
								printf("newline_count is %d\n", newline_count);
								printf("Content_Length_int is %d \n", Content_Length_int);
								printf("Content_Length_before_entity_body is %d \n", Content_Length_before_entity_body);
								printf("read_num_sum is %d\n", read_num_sum);
								printf("Content_Length_int + Content_Length_before_entity_body+ newline_count is %d \n", newline_count+ Content_Length_int + Content_Length_before_entity_body);
								*/

								alarm(0);// content length를 포함하여 content length를 알수 있을 때 read_with_time함수가 기다리는 값보다 더 빨리 종료 되게 함으로
										 // 알람을 0으로 만듬
								close(server_fd);
								break;
							}
							alarm(15);// read를 다시실행 할 경우를 대비하여 알람 15초 설정 이는 while값을 벗어나면 해제 된다.
						}
						//while 값을 종료한 지점의 read_num 값이 0인지 확인 //printf("while end point read_num is %d\n", read_num);
						if (read_num == 0)
						{
							//read가 안전하게 끝나면 알람 해제
							alarm(0);
							close(server_fd);
						}
						else if (read_num == -1)
						{
							//웹서버에서 read를 실패할 경우 no reasponse
							//printf("while end point read_num is %d\n", read_num);
							close(server_fd);
						}
						close(server_fd);
						time(&end);
						result = (int)(end - start);

						strcpy(passing_thread_data.passing_thread_data_hash_passing_url, hash_passing_url);
						passing_thread_data.passing_thread_data_hit_or_miss_check = hit_or_miss_check;
						passing_thread_data.passing_thread_data_subprocess_hitcount = subprocess_hitcount;
						passing_thread_data.passing_thread_data_subprocess_misscount = subprocess_misscount;
						passing_thread_data.passing_thread_data_result = result;

						err = pthread_create(&thread_id, NULL, thread_function,(void*)&passing_thread_data);//쓰레드 아이디 변수에 쓰레드 생성후 해당 아이디 담기
						if (err != 0)
						{
							printf("pthread_create() error.\n");
							continue;

						}
						/*
						else 
						{
							//디버깅을 위해서 
							printf("miss일때 메인함수에서 pthread_create 후에 메인에서 출력!\n");
							printf("서브프로세스에서 출력한 생성한 *TID# %ld.\n", (long)thread_id);
							printf("서브프로세스에서 출력한 자신의 *TID# %ld.\n", (long)pthread_self());
						}
						*/
						pthread_join(thread_id, &thread_return);// 
						printf("*PID# %ld exited the critical zone.\n", (long)getpid());// 생성한 해당 쓰레드 아이디가 종료되길 기다린다. 
						v(semid);

					}
				}
			}
			//printf("[%s : %d] client was disconnected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
			close(client_fd);
			exit(0);
		}
		close(client_fd);
		process_count++;
		//현재까지의 프로세스 개수를 세어주는 출력문 //printf("process_count is %d", process_count);
	}
	close(socket_fd);
	return 0;
}
