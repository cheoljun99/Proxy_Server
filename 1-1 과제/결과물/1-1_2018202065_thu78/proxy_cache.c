///////////////////////////////////////////////////////////////////////
// File Name : proxy_cache.c								         //
// Date : 2022/03/27												 //
// Os : Ubuntu 16.04 LTS 64bits										 //
// Author : Park Cheol Jun											 //
// Student ID : 2018202065											 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #1-1 (proxy server)		 //
// Description : SHA1을 이용하여 cache 구현 				         //
///////////////////////////////////////////////////////////////////////


#include<stdio.h>
#include <fcntl.h> 
#include<unistd.h>
#include<string.h>
#include<openssl/sha.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<pwd.h>
#include<string.h>





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

	SHA1(input_url, 40, hashed_160bits);



	for (i = 0; i < sizeof(hashed_160bits); i++)

		sprintf(hashed_hex + i * 2, "%02x", hashed_160bits[i]);

	strcpy(hashed_url, hashed_hex);

	return hashed_url;



}

char* getHomeDir(char* home)
{
	///////////////////////////////////////////////////////////////////////
	// getHomeDir														 //	
	// ==================================================================//
	// 인풋: char* ->pre home										     //
	// 아웃풋: char* -> post home										 //
	// 																	 //
	// 목적: home디렉토리를 출력하기위한 함수이다.						 //
	// 																	 //
	// 																	 //
	///////////////////////////////////////////////////////////////////////

	struct passwd* usr_info = getpwuid(getuid());

	strcpy(home, usr_info->pw_dir);

	return home;

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

	char dire[100]; // 사용하고자 하는 디렉토리 위치를 저장할 변수

	getHomeDir(dire);

	strcat(dire, "/cache/");// 프로그램 시작할 떄 루트 디렉토리에 cache폴더 생성하기위해 dire변수를 사용하여 디렉토리 위치 저장

	umask(0000);// 초기설정된 umask값을 변경
	mkdir(dire, S_IRWXU | S_IRWXG | S_IRWXO);//cache폴더 생성



	while (1)// bye 코멘트가 들어가기 전까지 실행한다.
	{
		printf("input url > ");

		scanf("%s", URL);

		if (!strcmp(URL, "bye"))//bye 코멘트가 들어가면 실행 종료.
		{

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
			open(dire, O_WRONLY | O_CREAT | O_EXCL, 0777);//생성한 디렉토리에 파일을 생성

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
			}






		}









	}







}
