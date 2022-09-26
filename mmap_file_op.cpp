#include "mmap_file_op.h"
#include "common.h"

static int debug = 1;
namespace qiqi{
	namespace largefile{
		int MMapFileOpton::mmap_file(const MMapOption & mmap_option){
			if(mmap_option.max_mmap_size_ < mmap_option.first_mmap_size_){  //做合法性检查
				return TFS_ERROR;
			}
			
			if(mmap_option.max_mmap_size_ <= 0 ){
				return TFS_ERROR;
			}
			
			int fd = check_file();
			if(fd < 0){
				fprintf(stderr, "MMapFileOpton::mmap_file - checking file failed!");
				return TFS_ERROR;
			}
			
			if(!is_mapped_){
				if(map_file_){
					delete map_file_;
				}
				map_file_  = new MMapFile(mmap_option , fd);
				is_mapped_ = map_file_->map_file(true);
			}
			
			if(is_mapped_){
				return TFS_SUCCESS;
			}else{
				return TFS_ERROR;
			}
		}
		
		
		int  MMapFileOpton::munmap_file(){
			if(is_mapped_ && map_file_ != NULL){
				delete map_file_;                //调用map_file的析构函数 ，map_file_指针指向了创建的对象
				is_mapped_ = false;
			}
			
			return TFS_SUCCESS;
		}
		
		void* MMapFileOpton:: get_map_data() const{
			if(is_mapped_){
				return map_file_->get_data();
			}
			return NULL;
		}
		
		int  MMapFileOpton::pread_file(char * buf, const int32_t size, const int64_t offset){
			if(is_mapped_ && size + offset > map_file_->get_size()){       //映射过来了，但是映射的数据不够读的
				if(debug){
					fprintf(stderr, "MMapFileOpton: pread size:%d, offset:%"__PRI64_PREFIX"d,map_file_size:%d,need remap\n",
					size, offset, map_file_->get_size());
				}
				
				map_file_->remap_file();
			}
			
			if(is_mapped_ && size + offset <=  map_file_->get_size()){
				memcpy(buf , (char*)map_file_->get_data() + offset , size);   //映射到内存中 + 偏移（相当于文件头+ 偏移）
				
				return TFS_SUCCESS;
			}
			
			//情况二：内存没有映射，
			return FileOption::pread_file(buf, size , offset);
		}
		
		int MMapFileOpton::pwrite_file(const char* buf,const int32_t size, const int64_t offset){
			if(is_mapped_ && size + offset > map_file_->get_size()){       //映射过来了，但是映射的数据不够写的
				if(debug){
					fprintf(stderr, "MMapFileOpton: pwrite size:%d, offset:%"__PRI64_PREFIX"d,map_file_size:%d,need remap\n",
					size, offset, map_file_->get_size());
				}
			
				map_file_->remap_file();
			}
			
			if(is_mapped_ && size + offset <=  map_file_->get_size()){         //映射过来了，足够的
				memcpy( (char*)map_file_->get_data() + offset ,buf, size);
				
				return TFS_SUCCESS;
			}
			
			//没映射或者映射不够
			return FileOption::pwrite_file(buf, size , offset);
		}
		
		int MMapFileOpton::flush_file(){
			if(is_mapped_){
				if(map_file_->sync_file()){          //会调用msync（） 将映射区的数据同步到磁盘 ASYNC表示异步，不需映射区更新，直接冲洗返回
					return TFS_SUCCESS;
				}else{
					return TFS_ERROR;
				}
			}
			
			return FileOption::flush_file();    //调用文件操作的fsync（）函数 ， 将改动的数据同步到磁盘  ，一个是文件操作，一个是系统调用
		}                                       //将缓冲区数据同步到磁盘文件！
	}
}