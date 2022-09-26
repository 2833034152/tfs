#ifndef MMAP_FILE_OP_H_
#define MMAP_FILE_OP_H_

#include "common.h"
#include "file_op.h"
#include "mmap_file.h"

namespace qiqi{
	namespace largefile{
		class MMapFileOpton:public FileOption{
			public:
			MMapFileOpton(const std::string& file_name , const int open_flags = O_CREAT | O_RDWR | O_LARGEFILE):
				FileOption(file_name  , open_flags), map_file_(NULL), is_mapped_(false) {
					
				}
			
			~MMapFileOpton(){
				if(map_file_){
					delete map_file_;
					map_file_ = NULL;
				}
			}
			
			int  mmap_file(const MMapOption & mmap_option);
			int  munmap_file();
			
			int  pread_file(char * buf, const int32_t size, const int64_t offset);
			int pwrite_file(const char* buf,const int32_t size, const int64_t offset);
			
			
			
			void* get_map_data() const;
			int flush_file();
			
			private:
				MMapFile *map_file_;    //映射到内存的文件指针
				bool is_mapped_;
			
			
		};
		
		
	}
}
#endif     //MMAP_FILE_OP_H_