#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <string>
#include <string.h>          //strerror

#include <fcntl.h>         //open所需要的三个头文件
#include <sys/types.h>
#include <sys/stat.h>

#include <stdint.h>      //int32_t需要的头文件

#include <errno.h>

#include <stdio.h>       //认识stderr这个文件描述符  还有fprintf
#include <sys/mman.h>    //mmap和munmap 

#include <unistd.h>
#include <stdlib.h>      //free函数使用

#include <inttypes.h>    // __PRIX_64 用的上  
#include <assert.h>

namespace qiqi{
	namespace largefile{
		const int32_t TFS_SUCCESS = 0;
		const int32_t TFS_ERROR = -1;                    //mmap_file_op文件里面用到
		const int32_t EXIT_DISK_OPER_INCOMPLETE = -8012; //read or write length is less than required
		const int32_t EXIT_INDEX_ALREADY_LOADED_ERROR = -8013;
		const int32_t EXIT_META_UNEXPECTED_FOUND_ERROR = -8014;  //meta found in index when insert  非预期的发现，未创建就存在了
	    const int32_t EXIT_INDEX_CORRUPT_ERROR = -8015;   //索引文件损坏
		const int32_t EXIT_BLOCKID_CONFLICT_ERROR = -8016;  //块id
		const int32_t EXIT_BUCKET_CONFIGURE_ERROR = -8017;
		const int32_t EXIT_META_NOT_FOUND_ERROR = -8018; //没找到节点
		const int32_t EXIT_BLOCKID_ZERO_ERROR = -8019;


		static const std::string MAINBLOCK_DIR_PREFIX = "/mainblock/";    //window是反斜杠符
		static const std::string INDEX_DIR_PREFIX = "/index/";       
		static const mode_t DIR_MODE =  0755;

		enum 	OperType{
			C_OPER_INSERT = 1,
			C_OPER_DELETE 
		};
		struct MMapOption{
			int32_t max_mmap_size_;
			int32_t first_mmap_size_;
			int32_t per_mmap_size_;
		};
		
		struct BlockInfo{                              //相当于c++里面的类
			uint32_t block_id_;
			int32_t version_;
			int32_t file_count_;
			int32_t size_;
			int32_t del_file_count_;
			int32_t del_size_;
			int32_t seq_no_;
			
			BlockInfo(){
			memset(this, 0, sizeof(BlockInfo));     //超级值得借鉴， 把内部数据全部置为0
			}
		
			inline bool operator==(const BlockInfo& rhs){
				return block_id_ == rhs.block_id_ && 
					   file_count_ == rhs.file_count_&&
					   size_ == rhs.size_&&
					   del_file_count_ == rhs.del_file_count_&&
					   del_size_ == rhs.del_size_&&
					   seq_no_ == rhs.seq_no_;
					
			}
		
			
		   
		};
		
		struct MetaInfo{
			public:
			MetaInfo(){
				init();
			}
			
			MetaInfo(const uint64_t fileid, const int32_t in_offset, const int32_t file_size,const int32_t next_meta_offset){
				fileid_ = fileid;
				location_.inner_offset_ = in_offset;
				location_.size_ = file_size;
				next_meta_offset_ = next_meta_offset;
			}
			
			MetaInfo(const MetaInfo & meta_info){
				memcpy(this , &meta_info, sizeof(meta_info));	
			}
			
			MetaInfo& operator=(const MetaInfo& meta_info){   //自己实现赋值构造，避免系统自己的浅拷贝
				if(this == &meta_info){
					return *this;
				}
				fileid_ = meta_info.fileid_;
				location_.inner_offset_ =  meta_info.location_.inner_offset_;
				location_.size_ =  meta_info.location_.size_;
				next_meta_offset_ =  meta_info.next_meta_offset_;
				return *this;
			}
			
			MetaInfo& clone(const MetaInfo & meta_info){
				assert(this == &meta_info);
				
				fileid_ = meta_info.fileid_;
				location_.inner_offset_ =  meta_info.location_.inner_offset_;
				location_.size_ =  meta_info.location_.size_;
				next_meta_offset_ =  meta_info.next_meta_offset_;
				return *this;
			}
			
			uint64_t get_key() const {  
				return fileid_;
			}
			
			void set_key(const uint64_t key){
				fileid_ = key;
			}
			
			uint64_t get_file_id() const {  
				return fileid_;
			}
			
			void set_file_id(const uint64_t file_id){
				fileid_ = file_id;
			}
			
			int32_t get_offset(){
				return location_.inner_offset_;
			}
			
			void set_offset(const int32_t offset){
				 location_.inner_offset_ = offset;
			}
			
			int32_t get_size(){
				return location_.size_;
			}
			
			void set_size(int32_t file_size){
				location_.size_ = file_size;
			}
			
			int32_t get_next_meta_offset(){
				return next_meta_offset_;
			}
			
			void set_next_meta_offset(int32_t offset){
				next_meta_offset_ = offset;
			}
			
		private:	
			uint64_t fileid_;
			
			struct {
				int32_t inner_offset_;
				int32_t size_;
			}location_;
			
			int32_t next_meta_offset_;
	
		private:
			void init(){
				fileid_ = 0;
				location_.inner_offset_ = 0;
				location_.size_ = 0;
				next_meta_offset_ = 0;
			}
			
		};
		
		
	}
}



#endif    //COMMON_H