#include "file_op.h"
#include "common.h"

namespace qiqi{
	namespace largefile{
		FileOption::FileOption(const std::string & file_name , const int open_flags ):
			fd_(-1),open_flags_(open_flags) {
			file_name_ = strdup(file_name.c_str());       //重新分配一个内存，并且复制file_name的内容给他,涉及到动态内存
			
		}
		FileOption:: ~FileOption(){
			if(fd_ > 0){
				::close(fd_);                        //头文件 <unistd.h>  ::防止和其他人写的close重复，使用全局的，不适用作用域内的
			}
			if(NULL != file_name_){
				free(file_name_);
				file_name_ = NULL;
			}
			
		}
		
		int FileOption::open_file(){
			if(fd_ > 0){
				close(fd_);
				fd_ = -1;
			}
			
			fd_ = ::open(file_name_ ,open_flags_, OPEN_MODE);
			if(fd_ < 0){
				return -errno;
			}
			
			return fd_;
		}
		
		void FileOption::close_file(){
			if(fd_ < 0){
				return ;
			}
			
			::close(fd_);
			fd_ = -1;
		}
		
		int64_t FileOption::get_file_size(){
			int fd = check_file();                  //私下打开了文件
			
			if(fd < 0){
				return -1;
			}
			
			struct stat statbuf;
			if(fstat(fd , &statbuf) != 0){
				return -1;
			}
			
			return statbuf.st_size;
		}
		
		int FileOption::check_file(){
			if(fd_ < 0){
				fd_ = open_file();
			}
			
			return fd_;
			
		}
		
		int FileOption::ftruncate_file(const int64_t length){
			int fd = check_file();                  //私下打开了文件
			
			if(fd < 0){
				return fd;
			}
			
			return ftruncate(fd_,length);
		}
		
		int FileOption::seek_file(const int64_t offset){
			int fd = check_file();                  //私下打开了文件 ,先检查文件是否已经打开了
			
			if(fd < 0){
				return fd;
			}
			
			return lseek(fd , offset, SEEK_SET);   //比fseek多返回个移动的位置
		}
		
		int FileOption::flush_file(){
			if(open_flags_ & O_SYNC){     //如歌已经是同步方式读写，直接返回即可，对文件的一切操作，都会立即同步到磁盘
				return 0;
			}
			
			int fd = check_file();
			if(fd < 0){
				return fd;
			}
			
			return fsync(fd);    //将缓冲区数据写进内存(磁盘)
			
		}
		
		int FileOption::unlink_file(){
			close_file();                     //先确保文件已经被关闭
			return ::unlink(file_name_);
		}
		
		int FileOption::pread_file(char * buf, const int32_t nbytes, const int64_t offset){
			//return pread64(fd_, buf , nbytes, offset);  要做好多次读取的准备
			int32_t left = nbytes;
			int64_t read_offset = offset;
			int32_t read_len = 0;
			char * p_tmp = buf;
			
			int i = 0;
			while(left > 0){
				i++;

				if(i>= MAX_DISK_TIMES){
					break;
				}
				
				if(check_file() < 0){
					return -errno;
					
				}
				
				read_len = ::pread(fd_, p_tmp, left, read_offset);
				if(read_len < 0){
					read_len = -errno;
					if(-read_len == EINTR || -read_len == EAGAIN){         //本次中断（可能由于系统繁忙）|| 再来一次
						continue;
					}else if(-read_len == EBADF){
						fd_ = -1;
						return read_len;
					}else{
						return read_len;
					}
		
				}else if(read_len == 0){
					break;
				}
				
				left -= read_len;
				p_tmp += read_len;
				read_offset += read_len;
			}
			
			if(0 != left){
				return EXIT_DISK_OPER_INCOMPLETE;
			}
			
			return TFS_SUCCESS;
			
			
		}
		
		int FileOption::pwrite_file(const char* buf,const int32_t nbytes, const int64_t offset){
			
			int32_t left = nbytes;
			int64_t write_offset = offset;
			int32_t written_len = 0;
			const char * p_tmp = buf;    
			
			int i = 0;
			while(left > 0){
				i++;

				if(i>= MAX_DISK_TIMES){
					break;
				}
				
				if(check_file() < 0){
					return -errno;
					
				}
				
				written_len = ::pwrite(fd_, p_tmp, left, write_offset);
				if(written_len < 0){
					written_len = -errno;
					if(-written_len == EINTR || -written_len == EAGAIN){         //本次中断（可能由于系统繁忙）|| 再来一次
						continue;
					}else if(-written_len == EBADF){
						fd_ = -1;
						//return written_len;
						continue;
					}else{
						return written_len;
					}
		
				}else if(written_len == 0){                          //读的话有可能为0，写的话不可能为0，这句话不写也行
					break;
				}
				
				left -= written_len;
				p_tmp += written_len;
				write_offset += written_len;
			}
			
			if(0 != left){
				return EXIT_DISK_OPER_INCOMPLETE;
			}
			
			return TFS_SUCCESS;
		}
		
		int FileOption::write_file(const char* buf,const int32_t nbytes){
			int32_t left = nbytes;
			int32_t written_len = 0;
			const char * p_tmp = buf;
			
			int i = 0;
			while(left > 0){
				i++;

				if(i>= MAX_DISK_TIMES){
					break;
				}
				
				if(check_file() < 0){
					return -errno;
					
				}
			written_len = write(fd_ , p_tmp, left);
			
			if(written_len < 0){
					written_len = -errno;
					if(-written_len == EINTR || -written_len == EAGAIN){         //本次中断（可能由于系统繁忙）|| 再来一次
						continue;
					}else if(-written_len == EBADF){
						fd_ = -1;
						//return written_len;
						continue;
					}else{
						return written_len;
					}
		
				}
				
				left -= written_len;
				p_tmp += written_len;
				
			}
			
			if(0 != left){
				return EXIT_DISK_OPER_INCOMPLETE;
			}
			
			return TFS_SUCCESS;
		}
	}	
}


