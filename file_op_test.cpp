#include "file_op.h"
#include "common.h"

using namespace std;
using namespace qiqi;

int main(void){
	const char* file_name = "file_op.txt";
	largefile::FileOption  * fileOP = new largefile::FileOption(file_name ,  O_CREAT| O_RDWR|O_LARGEFILE);
	
	int fd = fileOP->open_file();
	
	if(fd < 0){
		fprintf(stderr, "open file failed, reason: %s\n", strerror(-fd));
		exit(-1);
	}
	
	char buff[85];
	memset(buff , '6', 64);
	
	int ret = fileOP->pwrite_file(buff, 64 , 128);
	if(ret < 0) {
		if(ret == largefile::EXIT_DISK_OPER_INCOMPLETE){
			fprintf(stderr, "read or write length is less than required\n");
		}else{
			fprintf(stderr, "pwrite file failed, reason:%s\n", strerror(-ret));
		}	
	}
	
	memset(buff , 0 , 64);
	 ret = fileOP->pread_file(buff, 64 , 128);
	if(ret < 0) {
		if(ret == largefile::EXIT_DISK_OPER_INCOMPLETE){
			fprintf(stderr, "read or write length is less than required\n");
		}else{
			fprintf(stderr, "pread file failed, reason:%s\n", strerror(-ret));
		}	
	}else{
		buff[64] = '\0';
		printf("buffer:%s\n", buff);
	}
	
	ret = fileOP->write_file(buff, 10);
	if(ret < 0) {
		if(ret == largefile::EXIT_DISK_OPER_INCOMPLETE){
			fprintf(stderr, "read or write length is less than required\n");
		}else{
			fprintf(stderr, "write file failed, reason:%s\n", strerror(-ret));
		}	
	
	}
	
	
	fileOP->close_file();
	return 0;
}