#ifndef FILE_OP_H_
#define FILE_OP_H_

#include "common.h"
namespace qiqi
{
	namespace largefile{
		class FileOption{
			public:
			FileOption(const std::string & file_name , const int open_flags = O_RDWR|O_LARGEFILE);   //专搞大文件
			~FileOption();
			
			int open_file();
			void close_file();
			
			int flush_file();   //把文件立即写入到磁盘里，write 先写到磁盘里，不会立即写入
			
			
			int unlink_file();   //删除文件
			
			virtual int pread_file(char * buf, const int32_t nbytes, const int64_t offset);   //大文件偏移量可能很大，所以用64位的，读取的字节假设读取的较小，用32位的表示		
			virtual int pwrite_file(const char* buf,const int32_t nbytes, const int64_t offset);//把buf中的内容写到文件 
			
			int write_file(const char* buf,const int32_t nbytes);
			
			int64_t get_file_size();
			
			int ftruncate_file(const int64_t length);    //缩小文件
			
			int seek_file(const int64_t offset);
			
			int get_fd() const{
				
				
			}
			
			protected:
				int fd_;
				int open_flags_;
				char * file_name_;
				
				int check_file();      //私自调用，也是起到打开的作用
				
				static const mode_t OPEN_MODE = 0644;
				static const  int32_t MAX_DISK_TIMES = 5;    //最多的读取次数 ，磁盘损坏，系统忙,超过
				
		};
				
	}
	
}






#endif    //FILE_OP_H_