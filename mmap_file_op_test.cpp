#include "mmap_file_op.h"
#include <iostream>

using namespace std;
using namespace qiqi;


const static    largefile::MMapOption  mmap_option = {10240000, 4096, 4096};

int main(){
	
	int ret = 0;
	const char * filename = "mmap_file_op.txt";
	largefile::MMapFileOpton  *mmfo =  new largefile::MMapFileOpton(filename) ;
	
	int fd = mmfo->open_file();
	
	if(fd < 0){
		fprintf(stderr, "mmap_file_op  open file: %s failed!,reason:%s\n",filename, strerror(-fd));
		exit(-1);
	}
	
	 ret = mmfo->mmap_file(mmap_option);
	
	if(ret == largefile::TFS_ERROR){
		fprintf(stderr, "mmap_file failed. reason:%s\n",strerror(errno));
		mmfo->close_file();
		exit(-2);
	}
	
	char buff[128 + 1];
	memset(buff, '6', 128); 
	
	buff[127] = '\0';
	mmfo->pwrite_file(buff, 128 , 8);
	
	if(ret < 0) {
		if(ret == largefile::EXIT_DISK_OPER_INCOMPLETE){
			fprintf(stderr, "read or write length is less than required\n");
		}else{
			fprintf(stderr, "pwrite file failed, reason:%s\n", strerror(-ret));
		}	
	}
	
//	memset(buff , 0 , 18);
	 ret = mmfo->pread_file(buff, 128 , 8);
	if(ret < 0) {
		if(ret == largefile::EXIT_DISK_OPER_INCOMPLETE){
			fprintf(stderr, "read or write length is less than required\n");
		}else{
			fprintf(stderr, "pread file failed, reason:%s\n", strerror(-ret));
		}	
	}else{
		buff[128] = '\0';
		printf("buffer:%s\n", buff);
	}
	
	memset(buff, '8', 128); 
	buff[127] = '\0';
	mmfo->pwrite_file(buff, 128 , 4096);     //需要remap，因为写的范围4096 + 128超过了映射4096的大小
	
	ret = mmfo->flush_file();
	if(ret == largefile::TFS_ERROR){
		fprintf(stderr , "flush file failed .reason:%s\n",strerror(errno));
		
	}
	
	mmfo->munmap_file();
	mmfo->close_file();   //close（）哪怕失败了，只要程序结束了，操作系统会帮助回收资源
	
	return 0;
	
	
}