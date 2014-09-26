#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>


/* 상수 정의 */
#define MAXLINE		1024
#define MAXARGS		128
#define MAXPATH		1024
#define MAXTHREAD	10

#define DEFAULT_FILE_MODE	0664
#define DEFAULT_DIR_MODE	0775


/* 전역 변수 정의 */
char prompt[] = "myshell> ";
const char delim[] = " \t\n";
int redirectFlag = 0;


/* 전역 변수 선언 */


/* 함수 선언 */
void myshell_error(char *err_msg);
void process_cmd(char *cmdline);
int parse_line(char *cmdline, char **argv);
int builtin_cmd(int argc, char **argv);

// 내장 명령어 처리 함수
int list_files(int argc, char **argv);
int copy_file(int argc, char **argv);
int remove_file(int argc, char **argv);
int move_file(int argc, char **argv);
int change_directory(int argc, char **argv);
int print_working_directory(char **argv);
int make_directory(int argc, char **argv);
int remove_directory(int argc, char **argv);
int copy_directory(int argc, char **argv);
int list_all(int argc, char **argv);
int changeFileDescriptor(char ** argv);

//예외 처리 함수
int file_open_check(FILE* stream);
int file_close_check(int file_state);


/*
 * main - MyShell's main routine
 */
int main()
{
	char cmdline[MAXLINE];

	/* 명령어 처리 루프: 셸 명령어를 읽고 처리한다. */
	while (1) {
		// 프롬프트 출력
		printf("%s", prompt);
		fflush(stdout);

		// 명령 라인 읽기
		if (fgets(cmdline, MAXLINE, stdin) == NULL) {
			return 1;
		}

		// 명령 라인 처리
		process_cmd(cmdline);

		fflush(stdout);
	}

	/* 프로그램이 도달하지 못한다. */
	return 0;
}


/*
 * process_cmd
 *
 * 명령 라인을 인자 (argument) 배열로 변환한다.
 * 내장 명령 처리 함수를 수행한다.
 * 내장 명령이 아니면 자식 프로세스를 생성하여 지정된 프로그램을 실행한다.
 * 파이프(|)를 사용하는 경우에는 두 개의 자식 프로세스를 생성한다.
 */
void process_cmd(char *cmdline)
{
	int argc;
	int i;
	char *argv[MAXARGS];

	// 명령 라인을 해석하여 인자 (argument) 배열로 변환한다.
	argc = parse_line(cmdline, argv);

	/* 내장 명령 처리 함수를 수행한다. */
	if (builtin_cmd(argc, argv) == 0) {

		// 내장 명령 처리를 완료하고 리턴한다.
		return;
	}



	/*
	 * 자식 프로세스를 생성하여 프로그램을 실행한다.
	 */

	// 프로세스 생성


	// 자식 프로세스에서 프로그램 실행


	// 파이프 실행이면 자식 프로세스를 하나 더 생성하여 파이프로 연결


	// foreground 실행이면 자식 프로세스가 종료할 때까지 기다린다.

	return;
}


/*
 * parse_line
 *
 * 명령 라인을 인자(argument) 배열로 변환한다.
 * 인자의 개수(argc)를 리턴한다.
 * 파이프와 백그라운드 실행, 리다이렉션을 해석하고 flag와 관련 변수를 
 *   설정한다.
 */
int parse_line(char *cmdline, char **argv)
{
	// delimiter 문자를 이용하여 cmdline 문자열 분석 
	int count = 1;
	
	argv[0] = strtok(cmdline, delim);

	
	while(1) {
		argv[count] = strtok(NULL, delim);
		if(argv[count] == NULL) break;
		if(!strcmp(argv[count], ">")) {
			redirectFlag = count+1;
		}
		count++;
	}

	return count;
}




/*
 * builtin_cmd
 *
 * 내장 명령을 수행한다.
 * 내장 명령이 아니면 1을 리턴한다.
 */
int builtin_cmd(int argc, char **argv)
{

	// 내장 명령어 문자열과 argv[0]을 비교하여 각각의 처리 함수 호출
	if ((!strcmp(argv[0], "quit") || (!strcmp(argv[0], "exit")))) {
		exit(0);
	} else if(!strcmp(argv[0], "ls")) {
		list_files(argc, argv);
	} else if(!strcmp(argv[0], "cp")) {
		copy_file(argc, argv);
	} else if(!strcmp(argv[0], "rm")) {
		remove_file(argc, argv);
	} else if(!strcmp(argv[0], "mv")) {
		move_file(argc, argv);
	} else if(!strcmp(argv[0], "cd")) {
		change_directory(argc, argv);
	} else if(!strcmp(argv[0], "pwd")) {
		print_working_directory(argv);
	} else if(!strcmp(argv[0], "mkdir")) {
		make_directory(argc, argv);
	} else if(!strcmp(argv[0], "rmdir")) {
		remove_directory(argc, argv);
	} else if(!strcmp(argv[0], "ll")) {
		list_all(argc, argv);
	}



	// 내장 명령어가 아님.
	return 1;
}


/*
 * 
 * 내장 명령 처리 함수들
 * argc, argv를 인자로 받는다.
 * 
 */
int list_files(int argc, char **argv)
{

	DIR *dp = NULL;
	struct dirent * entry = NULL;
	char path[MAXPATH];
	int saved_stdout;

	if(argc == 1 || !strcmp(argv[1], ">")) {
		strcpy(path, ".");
	}
	else strcpy(path, argv[1]);

	dp = opendir(path);

	if(redirectFlag) {
		saved_stdout = changeFileDescriptor(argv);
	}

	while((entry = readdir(dp)) != NULL) 
	{
		printf("%s \n", entry->d_name);
	}

	if(redirectFlag) {
		dup2(saved_stdout, 1);
		close(saved_stdout);
		redirectFlag = 0;
	}
	closedir(dp);

	return 0;
}


int copy_file(int argc, char **argv)
{
	FILE* fromStream;
	FILE* toStream;
	int input;
	int file_state;
	fromStream = fopen(argv[1], "rb");
	toStream = fopen(argv[2], "wb");
	
	if(file_open_check(fromStream) || file_open_check(toStream))
	{
		printf("usage : cp + old_file + new_file \n");
		return 0;
	}

	while(input !=EOF) 
	{
		input = fgetc(fromStream);
		fputc(input, toStream);
	}

	file_state = fclose(fromStream);
	file_close_check(file_state);

	file_state = fclose(toStream);
	file_close_check(file_state);

	return 0;
}


int remove_file(int argc, char **argv)
{
	int rmResult = remove(argv[1]);
	if(rmResult == -1) 
	{
		printf("usage : cp + remove_file \n");
		return 0;
	}

	return 0;
}


int move_file(int argc, char **argv)
{
	FILE* fromStream;
	FILE* toStream;
	int input;
	int file_state;
	int value;
	printf("argv[2] : %s \n", argv[2]);
	fromStream = fopen(argv[1], "rb");
	toStream = fopen(argv[2], "wb");
	if(file_open_check(fromStream) || file_open_check(toStream))
	{
		printf("usage : mv + start_pos_file + end_pos_file \n");
		return 0;
	}


	while(input !=EOF) 
	{
		input = fgetc(fromStream);
		fputc(input, toStream);
	}
	value = remove_file(argc, argv);


	file_state = fclose(fromStream);
	file_close_check(file_state);

	file_state = fclose(toStream);
	file_close_check(file_state);

	return 0;
}


int change_directory(int argc, char **argv)
{
	int cdResult = chdir(argv[1]);
	if(cdResult == -1)
	{
		printf("usage : cd + path \n");
		return 0;
	}
	return 0;
}


int print_working_directory(char **argv)
{
	char buff[MAXPATH];
	char* pwdResult;
	int saved_stdout;

	if(redirectFlag) saved_stdout = changeFileDescriptor(argv);

	pwdResult = getcwd(buff, MAXPATH);
	if(!strcmp(pwdResult, "-1"))
	{
		printf("pwd error");
		return 0;
	}
	printf("%s\n", buff);

	if(redirectFlag) {
		dup2(saved_stdout, 1);
		close(saved_stdout);
	}
	return 0;
}


int make_directory(int argc, char **argv)
{
	int mkdirResult = mkdir(argv[1], 0755);
	if(mkdirResult == -1)
	{
		printf("usage : mkdir + folder_name. not overlapping \n");
		return 0;
	}
	return 0;
}


int remove_directory(int argc, char **argv)
{
	int rmdirResult = rmdir(argv[1]);
	if(rmdirResult == -1)
	{
		printf("usage : rmdir + folder_name\n");
		return 0;
	}
	return 0;
}


int copy_directory(int argc, char **argv)
{
	return 0;
}

int list_all(int argc, char **argv)
{
	DIR *dp = NULL;
	struct dirent * entry = NULL;
	char path[MAXPATH];
	char detailPath[MAXPATH];
	char monthBuf[20];
	struct stat statBuf;
	mode_t file_mode;
	struct passwd *pw;
	struct group *gr;
	int d;

	if(argc == 1) {
		strcpy(path, ".");
	}
	else strcpy(path, argv[1]);

	dp = opendir(path);
	strcpy(detailPath, path);

	while((entry = readdir(dp)) != NULL) 
	{
		if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
		strcat(detailPath, "/");
		strcat(detailPath, entry->d_name);
		stat(detailPath, &statBuf);
		file_mode = statBuf.st_mode;
		if(file_mode & S_IFDIR) printf("d");
		else printf("-");
		if(file_mode & S_IRUSR) printf("r");
		else printf("-");
		if(file_mode & S_IWUSR) printf("w");
		else printf("-");
		if(file_mode & S_IXUSR) printf("x");
		else printf("-");
		if(file_mode & S_IRGRP) printf("r");
		else printf("-");
		if(file_mode & S_IWGRP) printf("w");
		else printf("-");
		if(file_mode & S_IXGRP) printf("x");
		else printf("-");
		if(file_mode & S_IROTH) printf("r");
		else printf("-");
		if(file_mode & S_IWOTH) printf("w");
		else printf("-");
		if(file_mode & S_IXOTH) printf("x");
		else printf("-");
		printf("  ");
		pw = getpwuid(statBuf.st_uid);
		gr = getgrgid(statBuf.st_gid);

		printf("%3d  ", statBuf.st_nlink);
		printf("%s  ", pw->pw_name);
		printf("%s  ", gr->gr_name);
		printf("%9lld   ", statBuf.st_size);

		d= strftime(monthBuf, 20, "%b  %e  %r", localtime(&statBuf.st_ctime));

		printf("%s  ", monthBuf);
		printf("%s\n", entry->d_name);

		strcpy(detailPath, path);
	}

	closedir(dp);

	return 0;
}

int file_close_check(int file_state)
{
	if(file_state == EOF)
	{
		printf("file close error\n");
		return 1;
	}
	return 0;
}

int file_open_check(FILE* stream)
{
	if(stream == NULL) {
		printf("file open error\n");
		return 1;
	}
	return 0;
}

int changeFileDescriptor(char ** argv)
{
	int saved_stdout, fd;

	if((fd = creat(argv[redirectFlag], 0644)) < 0)
	{
		printf("usage : type file route after > ");
		return 0;
	}

	saved_stdout = dup(1);

	dup2(fd, 1);

	return saved_stdout;
}