#include <stdio.h>
#include "mmap_file.h" 


static int debug = 1;
namespace qiqi{
	namespace largefile{
		MMapFile::MMapFile():
		size_(0) , fd_(-1) , data_(NULL) {
		
		} 
		
		MMapFile::MMapFile(const int fd):                                     //按照文件fd映射
		size_(0) , fd_(fd) , data_(NULL) {
		
		} 
		
		MMapFile::MMapFile(const MMapOption & mmap_option , const int fd):    //按照映射选项进行映射
		size_(0) , fd_(fd) , data_(NULL) {
			mmap_file_option_.max_mmap_size_ =  mmap_option.max_mmap_size_;
			mmap_file_option_.first_mmap_size_  = mmap_option.first_mmap_size_;
			mmap_file_option_.per_mmap_size_  = mmap_option.per_mmap_size_;
		} 
		
		 MMapFile::~MMapFile(){
             if(data_) {
                if(debug) printf("mmap file destruct, fd:%d, maped size:%d, data:%p\n",fd_,size_,data_);
                msync(data_ , size_ , MS_SYNC);      //同布方式同步数据
                munmap(data_ ,size_);
                
                data_ = NULL;
                size_ = 0;
                fd_  = -1;
                
                mmap_file_option_.max_mmap_size_ = 0;
                mmap_file_option_.first_mmap_size_ = 0;
                mmap_file_option_.per_mmap_size_ = 0;
             } 
			
		 }
		 
		bool MMapFile::sync_file(){                         //是否成功同步数据
			if(data_ != NULL && size_ > 0) {
				return msync(data_ , size_ , MS_ASYNC)== 0;     //系统调用吧，成功同步数据返回0,异步的同步效率更高
			}
			
			return true; 
		 }
		 
		 bool MMapFile::map_file(const bool write ){
			int flags = PROT_READ;    //默认是可读
			
			if(write){
				flags |= PROT_WRITE;
			}
			
			if(fd_ < 0){
				return false;
			}
			
			if(mmap_file_option_.max_mmap_size_ == 0){
				return false;
			}
			
			if(size_ < mmap_file_option_.max_mmap_size_ ){           //获得映射的内存大小
				size_ = mmap_file_option_.first_mmap_size_ ;
			}else{
				size_ = mmap_file_option_.max_mmap_size_ ;
			}
			
			if(!ensure_file_size(size_)){
				fprintf(stderr , "ensure_file_size erorr,fd:%d, size:%d\n" , fd_,size_);
				
			}                                 // 调整文件和内存大小一样，同步
			
			data_ = mmap(0 , size_ ,flags , MAP_SHARED , fd_ , 0);
			
			if(data_ == MAP_FAILED){         //如果内存映射出错
				fprintf(stderr , "map file failed:%s\n" , strerror(errno)); 
				data_ = NULL;
				size_ = 0;
				fd_ = -1;	
			
				return false;
			}
			
			
			
			if(debug) printf("map file success , fd:%d, data:%p ,maped size:%d\n" , fd_ , data_ , size_);
			return true;
		 }
		 
		 void * MMapFile:: get_data() const{
			return data_;
		 }
		 
		 int32_t MMapFile:: get_size() const{
			return size_;
		 }
		 
		bool MMapFile:: munmap_file(){
			if(munmap(data_ , size_ ) == 0){
				return true;
			}else{
				return false;
			}
		 }
		 
		bool  MMapFile:: ensure_file_size(const int32_t size){  //把文件调整到指定大小
			struct stat s;
			if(fstat(fd_ , &s) < 0) {      //读取文件的状态信息，根据文件fd，获得状态stat
				fprintf(stderr , "fsta error, error desc:%s\n" , strerror(errno));
				return false;
			}
			
			if(s.st_size < size) {
				if(ftruncate(fd_ , size) < 0 ) {      //调整文件按大小
					fprintf(stderr , "ftruncate erorr,fd_:%d, size:%d, size_:%d, errno: %d, EINVAL: %d,error desc:%s\n",fd_,size,size_ ,errno,   
					EINVAL
					
					 ,strerror(errno));
					return false;
				}
			
			}
			
			return true;
		}
			
		bool MMapFile::  remap_file(){    //重新映射,再次增加4K
			if(data_ == NULL || fd_ < 0){                             //之前映射的数据和文件fd不符合要求时
				fprintf(stderr , "remap_file failed\n");
				return false;
			}
			
			if(size_ == mmap_file_option_.max_mmap_size_){            //之前映射的大小已经最大时
				fprintf(stderr, "already mmaped max size, now size:%d, max size:%d\n", size_ , mmap_file_option_.max_mmap_size_);
				return false;
			
			}
			 
			int32_t new_size  = size_ + mmap_file_option_.per_mmap_size_;
			if(new_size > mmap_file_option_.max_mmap_size_){
				new_size =  mmap_file_option_.max_mmap_size_;
			
			}
			
			//确定好映射的新的内存大小时，就得和文件同步了
			if(!ensure_file_size(new_size)){
				fprintf(stderr, "ensure file size failed in remap file,size:%d\n", new_size);	
				return false;	
			}
			
			if(debug) printf("rmmap file start, fd:%d, old data:%p, now size:%d, new size:%d\n",fd_, data_, size_, new_size);
			
			
		    
		 
			 void* new_map_data = mremap(data_, size_,new_size, MREMAP_MAYMOVE);
			 if(new_map_data == MAP_FAILED) {
				 fprintf(stderr, "mremap file failed, new_size:%d,err desc:%s\n",new_size, strerror(errno));
				 return false;
			 }else{
				 if(debug) printf("mrmmap file success, fd:%d, old data:%p, new data:%p, now size:%d, new size:%d\n",fd_, data_, new_map_data, size_, new_size);
			 }
			 
			 data_ = new_map_data;
			 size_ = new_size;
			return true;
		 }
	}

}