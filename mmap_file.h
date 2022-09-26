#ifndef MMAP_FILE_H
#define MMAP_FILE_H

#include <unistd.h>
#include "common.h"
namespace qiqi{
	namespace largefile{
		
		
		
		class MMapFile{
			public:
			 MMapFile();
			 
			 explicit MMapFile(const int fd);
			 
			 MMapFile(const MMapOption & mmap_option, const int fd);
			 
			 ~MMapFile();
			 
			 bool sync_file();
			 bool map_file(const bool write = false);   //将文件映射到内存，同时设置访问权限
			 void * get_data() const;  //获取映射到内存的数据，在内存中的首地址
			 int32_t get_size() const; //获取映射到内存的数据的大小
			 
			 bool munmap_file();    //解除映射
			 bool remap_file();     //重新映射
			 
			 private:
				bool  ensure_file_size(const int32_t size);   //给映射到内存的数据扩容
				
			 private:
				int32_t size_;
				int fd_;
				void *data_;
				
				struct MMapOption mmap_file_option_;
		
		
		
		};
	
	
	}
}




#endif