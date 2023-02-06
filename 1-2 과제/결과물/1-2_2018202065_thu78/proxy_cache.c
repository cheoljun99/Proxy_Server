///////////////////////////////////////////////////////////////////////
// File Name : proxy_cache.c								         //
// Date : 2022/03/27												 //
// Os : Ubuntu 16.04 LTS 64bits										 //
// Author : Park Cheol Jun											 //
// Student ID : 2018202065											 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #1-2 (proxy server)		 //
// Description : HIT / MISS 판별				         			 //
///////////////////////////////////////////////////////////////////////

#include<stdio.h>
#include<fcntl.h> 
#include<unistd.h>
#include<string.h>
#include<dirent.h>
#include<openssl/sha.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<pwd.h>
#include<string.h>
#include<time.h>






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

int discrimination(char* first_HS_URL, char* second_HS_URL,char * URL)
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
		write(fd3, "[Miss]", strlen("[Miss]"));
		write(fd3, URL, strlen(URL));
		write(fd3, "-[", strlen("-["));
		time_t now;
		struct tm* ltp;
		time(&now);
		ltp = localtime(&now);
		char st[100];
		memset(st, 0, 100);
		sprintf(st, "%d", (ltp->tm_year+1900));
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
			write(fd4, "[Hit]", strlen("[Hit]"));
			write(fd4, first_HS_URL, strlen(first_HS_URL));
			write(fd4, "/", strlen("/"));
			write(fd4, second_HS_URL, strlen(second_HS_URL));
			write(fd4, "-[", strlen("-["));
			time_t now2;
			struct tm* gtp;
			time(&now2);
			gtp = localtime(&now2);
			char st2[100];
			memset(st2, 0, 100);
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
	write(fd5, "[Miss]", strlen("[Miss]"));
	write(fd5, URL, strlen(URL));
	write(fd5, "-[", strlen("-["));
	time_t now3;
	struct tm* etp;
	time(&now3);
	etp = localtime(&now3);
	char st3[100];
	memset(st3, 0, 100);
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
	write(fd5, "[Terminated] ", strlen("[Terminated] "));
	write(fd5, "run time: ", strlen("run time: "));
	char st3[100];
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

int main(void)
{
	///////////////////////////////////////////////////////////////////////
	// main 함수														 //	
	// ==================================================================//
	// 																	 //
	// 목적: main 함수로 리눅스에서 실행 되며 위에 정의한 함수들을       //
	// 이용하여 본 과제의 목적인 해쉬화된 URL을 Cache에 저장하는것이     //
	// 목표다.                        		                             //
	// 																	 //
	// 																	 //
	///////////////////////////////////////////////////////////////////////



	char URL[100];//URL을 scanf로 받을 때를 위해 정의 한 변수 넉넉하게 100으로 초기화

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







	while (1)// bye 코멘트가 들어가기 전까지 실행한다.
	{
		printf("input url > ");

		scanf("%s", URL);

		if (!strcmp(URL, "bye"))//bye 코멘트가 들어가면 실행 종료.
		{
			time(&end);
			int result=0;
			result = (int)(end - start);
			bye(hitcount, misscount, result);

			return 0;

		}
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
			fd2=open(dire, O_WRONLY | O_CREAT | O_EXCL, 0777);//생성한 디렉토리에 파일을 생성
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









	}







}
