#include <iostream>
#include "mmap_file.h"
#include <string>

//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE
//#define _FILE_OFFSET_BITS 64
using namespace  std;
using namespace  qiqi;

static const mode_t  OPEN_MODE = 0644;

const static    largefile::MMapOption  mmap_option = {10240000, 4096, 4096};
int open_file(string file_name, int open_flags){
	int fd = open(file_name.c_str(), open_flags, OPEN_MODE);  //成功返回>0
	
	if(fd < 0){
		return -errno;
		
	}
	return fd;
	
}

int  main(void){

	
	
	const char* filename = "./mapfile_test.txt";
	int fd = open_file(filename, O_RDWR| O_CREAT| O_LARGEFILE);  //表示超大文件，例如超过1T
//	printf("fd:%d\n",fd);

	
	if(fd < 0){	
		fprintf(stderr, "open file failed: %s, error desc: %s\n", filename, strerror(-fd));
		return -1;
	}
	
	largefile::MMapFile * map_file = new largefile::MMapFile(mmap_option , fd);
//	printf("fd:%d\n",fd);
	bool is_mapped = map_file->map_file(true);
	
	if(is_mapped){
		map_file->remap_file();
		
		memset(map_file->get_data(), '0', map_file->get_size());
		map_file->sync_file();               //使得内存和磁盘数据同步
		map_file->munmap_file();
	}else{
		fprintf(stderr,"map file failed\n");
	}
//	map_file->remap_file();
	close(fd);
	
	return 0;
	
}