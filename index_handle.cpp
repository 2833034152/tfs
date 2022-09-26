#include "common.h"
#include "index_handle.h"
#include <sstream>
static int debug = 1;
namespace qiqi{
	namespace largefile{
		IndexHandle::IndexHandle(const std::string & base_path, const uint32_t main_block_id){
			//create file_op
			std::stringstream tmp_stream;
			tmp_stream << base_path << INDEX_DIR_PREFIX<<main_block_id;
			
			std::string index_path;
			tmp_stream >> index_path;
			
			file_op_ = new MMapFileOpton(index_path, O_CREAT| O_RDWR | O_LARGEFILE);    //运用到多态
			is_load_ = false;
		}
		
		IndexHandle::~IndexHandle(){
			if(file_op_){
				delete file_op_;
				file_op_ = NULL;
			}
			
		}
		
		int IndexHandle::create(const uint32_t logic_block_id, const int32_t bucket_size, const MMapOption map_option){
			if(debug) printf("create index, block id:%u, bucket size:%d, max_mmap_size:%d, per_mmap_size:%d, first_mmap_size:%d\n",
				logic_block_id, bucket_size, map_option.max_mmap_size_, map_option.per_mmap_size_, map_option.first_mmap_size_);
			
			int ret = TFS_SUCCESS;

			if(is_load_){
				
				return EXIT_INDEX_ALREADY_LOADED_ERROR;
			}
			
			int64_t file_size = file_op_->get_file_size();

			if(file_size < 0){

				return TFS_ERROR;
			}else if(file_size == 0){
				IndexHeader i_header;
				i_header.block_info_.block_id_ = logic_block_id;
				i_header.block_info_.seq_no_  = 1;
				i_header.bucket_size_ = bucket_size;
				
				i_header.index_file_size_ = sizeof(IndexHeader) + bucket_size*sizeof(int32_t);
				
				
				char * init_data = new char[i_header.index_file_size_];    //重大错误，由于使用了（）,导致delete[]出现释放错误！
				memcpy(init_data, &i_header, sizeof(IndexHeader));
				memset(init_data + sizeof(IndexHeader),0, i_header.index_file_size_ - sizeof(IndexHeader));
				
				ret = file_op_->pwrite_file(init_data, i_header.index_file_size_, 0);
				

				delete[] init_data;
				init_data = NULL;

				
				if(ret != (TFS_SUCCESS)){
					fprintf(stderr,"index_handle pwrite_file failed!\n");
					return ret;
				}
				
				ret = file_op_->flush_file();    //锦上添花的作用， 立刻把缓冲区的内容学到磁盘中
				
				if(ret != (TFS_SUCCESS)){
					fprintf(stderr,"index_handle flush_file failed!\n");
					return ret;
				}
				
			}else {   //index file already exist
			    fprintf(stderr,"EXIT_META_UNEXPECT_FOUND_ERROR\n");
				return EXIT_META_UNEXPECTED_FOUND_ERROR;
			}
			
			ret = file_op_->mmap_file(map_option);
			if(debug) printf("index_handle is mmap_filing...\n");
			if(ret != (TFS_SUCCESS)){
				fprintf(stderr,"index_handle mmap_file failed!\n");
					return ret;
			}
			
			is_load_ = true;
			if(debug) printf("init blockid:%d index successful. data file size:%d,index file size:%d,bucket size:%d,free_head offset:%d,seqno:%d,size:%d,file_count:%d,del_size:%d,del_file_count:%d, version:%d\n",
				   logic_block_id, index_header()->data_file_offset_, index_header()->index_file_size_,
				   index_header()->bucket_size_, index_header()->free_header_offset_,
				   
				   block_info()->seq_no_,block_info()->size_, block_info()->file_count_,
				   block_info()->del_size_,block_info()->del_file_count_,block_info()->version_);
				   
			
			return TFS_SUCCESS;
		}
		
		int IndexHandle::load(const uint32_t logic_block_id, const int32_t _bucket_size, const MMapOption map_option){
			//printf("load start-logic_block_id:%d, block_id_:%d\n",logic_block_id,block_info()-> block_id_);//还没映射，现在调用还没初始化
			  
			//if(debug) printf("create index, block id:%u, bucket size:%d, max_mmap_size:%d, per_mmap_size:%d, first_mmap_size:%d\n",
			//	logic_block_id, _bucket_size, map_option.max_mmap_size_, map_option.per_mmap_size_, map_option.first_mmap_size_;
			int ret = TFS_SUCCESS;
			if(debug) printf("is loading...\n");
			if(is_load_){
				return EXIT_INDEX_ALREADY_LOADED_ERROR;
				
			}
			
			int64_t file_size = file_op_->get_file_size();
			
			if(file_size < 0){
				return  file_size;
				
			}else if(file_size == 0){
				printf("file size = 0\n");
				return EXIT_INDEX_CORRUPT_ERROR;
				
			}
			MMapOption tmp_map_option = map_option;
			if(file_size > tmp_map_option.first_mmap_size_ && file_size < tmp_map_option.max_mmap_size_){
				tmp_map_option.first_mmap_size_ = file_size;
				
			}
			ret = file_op_->mmap_file(map_option);                  //主要代码，其他的都是合法性检查
			if(ret != (TFS_SUCCESS)){
				return ret;
			}
			if(0 == bucket_size() || 0 == block_info()->block_id_){
				fprintf(stderr, "index corrupt error, blockid:%u,bucket:%d\n",block_info()->block_id_, bucket_size());
				return EXIT_INDEX_CORRUPT_ERROR;
			}
			
			//check file size
			int32_t index_file_size = sizeof(IndexHeader) + _bucket_size*sizeof(int32_t);
			if(file_size < index_file_size){
				fprintf(stderr, "index corrupt error, blockid:%u, bucket:%d, file size:%"__PRI64_PREFIX"d, index file size:%d\n"
				,block_info()->block_id_, bucket_size(),file_size , index_file_size);
				return EXIT_INDEX_CORRUPT_ERROR;
			}
			//check blockid
			printf("logic_block_id:%d, block_id_:%d\n",logic_block_id,block_info()-> block_id_);
			if(logic_block_id  != block_info()->block_id_){
				fprintf(stderr, "block id conflict, blockid:%u, index blockid:%u\n",logic_block_id,block_info()->block_id_);
				return EXIT_BLOCKID_CONFLICT_ERROR;
			}
			
			
			//check bucket size
			if(_bucket_size  != bucket_size()){
				fprintf(stderr, "index configure error, old bucket size:%d, new bucket size:%d\n",bucket_size(),_bucket_size);
				return EXIT_BUCKET_CONFIGURE_ERROR;
			}
			
			is_load_ = true;
			if(debug) printf("load blockid:%d index successful. data file size:%d,index file size:%d,bucket size:%d,free_head offset:%d,seqno:%d,size:%d,file_count:%d,del_size:%d,del_file_count:%d, version:%d\n",
				   logic_block_id, index_header()->data_file_offset_, index_header()->index_file_size_,
				   index_header()->bucket_size_, index_header()->free_header_offset_,
				   
				   block_info()->seq_no_,block_info()->size_, block_info()->file_count_,
				   block_info()->del_size_,block_info()->del_file_count_,block_info()->version_ );
				   
			
			return TFS_SUCCESS;
			
		}
		
		int IndexHandle::remove(const uint32_t logi_block_id){
			if(is_load_){
				if(logi_block_id != block_info()->block_id_)
				{
					fprintf(stderr,"block id conflict.block id:%d,index block id:%d\n",logi_block_id,block_info()->block_id_);
					return EXIT_BLOCKID_CONFLICT_ERROR;
				}
			}
			
			int ret = file_op_->munmap_file();
			if(TFS_SUCCESS != ret){
				return ret;
			} 
			ret = file_op_->unlink_file();
			return ret;
		}
		
		int IndexHandle::flush(){
			int ret = file_op_->flush_file();
			if(TFS_SUCCESS != ret){
				fprintf(stderr, "index flush failed:,ret:%d, err desc:%s\n",ret, strerror(errno));
			}
			
			return ret;
		}
		
		int IndexHandle::update_block_info(const OperType oper_type, const uint32_t modify_size){
			if(block_info()->block_id_ == 0){
				return EXIT_BLOCKID_ZERO_ERROR;
			}
			if( oper_type == C_OPER_INSERT){
				++block_info()->version_;
				++block_info()->file_count_;
				++block_info()->seq_no_;
				block_info()->size_ += modify_size;
			}else if( oper_type == C_OPER_DELETE){
				++block_info()->version_;
				--block_info()->file_count_;
				
			    block_info()->size_ -= modify_size;
				++block_info()->del_file_count_;
				block_info()->del_size_ += modify_size;
			
			}
			if(debug) printf("update blockid:%d index successful. data file size:%d,index file size:%d,bucket size:%d,free_head offset:%d,seqno:%d,size:%d,file_count:%d,del_size:%d,del_file_count:%d, version:%d,oper_type:%d\n",
				   block_info()->block_id_, index_header()->data_file_offset_, index_header()->index_file_size_,
				   index_header()->bucket_size_, index_header()->free_header_offset_,
				   
				   block_info()->seq_no_,block_info()->size_, block_info()->file_count_,
				   block_info()->del_size_,block_info()->del_file_count_,block_info()->version_ , oper_type);
				   
				   return TFS_SUCCESS;
		}
		
		int IndexHandle::write_segment_meta(const uint64_t key, MetaInfo & meta){
			int32_t current_offset = 0, previous_offset = 0;
			
			//1.从文件哈希表中查找key是否存在
			int ret  = hash_find(key, current_offset, previous_offset);
			if(TFS_SUCCESS == ret){
				return EXIT_META_UNEXPECTED_FOUND_ERROR;         //成功了就不写入了。防止重复写入，不期望的meta
			}else if(EXIT_META_NOT_FOUND_ERROR != ret){          //排除三种情况，成功找到  ，读取文件错误， 没有找到MetaInfo
				return ret ;                                    
			}
			
			//2.不存在就写入meta到文件哈希表，hash_inset(key, previous_offset , meta);
			ret = hash_insert(key, previous_offset, meta);
			return ret;
			
		}
		
		int32_t IndexHandle:: read_segment_meta(const uint64_t key, MetaInfo & meta){
				int32_t current_offset = 0, previous_offset = 0;
			
				//1.从文件哈希表中查找key是否存在
				int ret  = hash_find(key, current_offset, previous_offset);
				if(TFS_SUCCESS == ret){    //若果找到了对应的key的metainfo就去读
					ret = file_op_->pread_file(reinterpret_cast<char*> (&meta), sizeof(MetaInfo), current_offset);
					return ret;
				}else{
					return ret;
				}
				
				
		}
		
		int32_t IndexHandle::delete_segment_meta(const uint64_t key){
			int32_t current_offset = 0, previous_offset = 0;
			
				//1.从文件哈希表中查找key是否存在
				int ret  = hash_find(key, current_offset, previous_offset);
				if(ret != largefile::TFS_SUCCESS){
					return ret;
				}
				
				MetaInfo  	meta_info;
				ret = file_op_->pread_file(reinterpret_cast<char*> (&meta_info), sizeof(MetaInfo), current_offset);
				printf("delete_segment_meta:%d\n",meta_info.get_size());
				
				if(ret != largefile::TFS_SUCCESS){
					return ret;
				}
				int32_t next_pos = meta_info.get_next_meta_offset();
				
				if(previous_offset == 0){
					int32_t slot  = key % bucket_size();
					bucket_slot()[slot] = next_pos;
				}else{
					MetaInfo pre_meta_info;
					ret = file_op_->pread_file(reinterpret_cast<char*> (&pre_meta_info), sizeof(MetaInfo), previous_offset);
					if(ret != largefile::TFS_SUCCESS){
						return ret;
					}
					
					pre_meta_info.set_next_meta_offset(next_pos);
					ret = file_op_->pwrite_file(reinterpret_cast<char*> (&pre_meta_info), sizeof(MetaInfo), previous_offset);
					if(ret != largefile::TFS_SUCCESS){
						return ret;
					}
					
					
				}
				printf("first meta_info.get_size():%d\n",meta_info.get_size());	
				//把删除节点加入可重用节点链表
				meta_info.set_next_meta_offset(free_header_offset());
				ret = file_op_->pwrite_file(reinterpret_cast<char*> (&meta_info), sizeof(MetaInfo), current_offset);
				if(ret != largefile::TFS_SUCCESS){
					return ret;
				}
				index_header()->free_header_offset_ = current_offset;              //头插法
				if(debug) printf("delete_segment_meta- reuse metainfo ,current_offset:%d\n",current_offset);
				
				
				update_block_info(C_OPER_DELETE, meta_info.get_size());    //这个更新不会失败，所以不用判断
				printf("meta_info.get_size()%d\n",meta_info.get_size());	
				return largefile::TFS_SUCCESS;
				
				
		}
			
		// zi:索引文件已经映射到内存， pos对应文件的偏移的pos位置
		int IndexHandle::hash_find(const uint64_t key,int32_t &current_offset, int32_t & previous_offset){
			int ret = TFS_SUCCESS;
			MetaInfo meta_info;      //
			current_offset = 0;
			previous_offset = 0;
			
			//1,确定key存放的桶的位置
			int32_t slot = (key % bucket_size());    //reinterpret_cast<int32_t>   强制转换会报错
			int32_t pos = bucket_slot()[slot];      //自：指向metaInfo存的首地址
			
			//2. 读取桶首节点存储的第一个节点的偏移量，若偏移量为0，直接返回EXIT_META_NOT_FOUND
			//3.根据偏移量读取存储的metainfo
			//4.与key相比较，相等则设置current_offset和previous_offset,	并返回TFS_SUCCESS，否则继续执行5
			//5.从metainfo中取得下一个节点在文件中的偏移量，若为0，直接返回EXIT_META_NOT_FOUND，否则继续跳转3执行
			for(;pos != 0;){
				ret = file_op_->pread_file(reinterpret_cast<char*> (&meta_info) , sizeof(meta_info) , pos );   //已经映射到内存中了， 操作的是内存中的数据
																											  //遍历，相当于next,
				
				if(TFS_SUCCESS != ret){
					return ret;
				}
				
				if(hash_compare(key , meta_info.get_key())){
					current_offset = pos;
					return TFS_SUCCESS;
				}
				
				previous_offset = pos;
				pos = meta_info.get_next_meta_offset();
			}
			
			return EXIT_META_NOT_FOUND_ERROR;
			
		}
		
            int32_t IndexHandle::hash_insert(const uint64_t key, int32_t previous_offset, MetaInfo &meta){
				int ret = TFS_SUCCESS;
				MetaInfo  tmp_meta_info;
				int32_t current_offset = 0;
				//1,确定key存放的桶的位置
				int32_t slot = (key % bucket_size());  //reinterpret_cast<int32_t>
				
				//2.确定meta节点存储在文件中的偏移量
				if(free_header_offset() != 0){
					 ret = file_op_->pread_file(reinterpret_cast< char*>(&tmp_meta_info) , sizeof(MetaInfo), free_header_offset());
					 if(TFS_SUCCESS != ret){
						return ret;
					}
					current_offset = index_header()->free_header_offset_;
					if(debug) printf("reuse metainfo, current_offset:%d\n",current_offset);
					index_header()->free_header_offset_ = tmp_meta_info.get_next_meta_offset();
				}else{
					  current_offset = index_header()->index_file_size_;
				     index_header()->index_file_size_ += sizeof(MetaInfo);
				}
				
				
			
				
				//3.将meta节点写入索引文件中
				meta.set_next_meta_offset(0);
				
				 ret = file_op_->pwrite_file(reinterpret_cast<const char*>(&meta) , sizeof(MetaInfo), current_offset);
				
				if(TFS_SUCCESS != ret){
					index_header()->index_file_size_ -= sizeof(MetaInfo);
					return ret;
				}
			    
					//4.将meta插入到哈希链表中
				if(previous_offset != 0){   //前一个节点已经存在
					ret = file_op_->pread_file(reinterpret_cast<char*>(&tmp_meta_info), sizeof(MetaInfo), previous_offset);
					if(TFS_SUCCESS != ret){
						index_header()->index_file_size_ -= sizeof(MetaInfo);
						return ret;
					}
					tmp_meta_info.set_next_meta_offset(current_offset);
					ret = file_op_->pwrite_file(reinterpret_cast<const char*>(&tmp_meta_info) ,sizeof(MetaInfo), previous_offset);
					if(TFS_SUCCESS != ret){
						index_header()->index_file_size_ -= sizeof(MetaInfo);
						return ret;
					}
					
				}else{//前一个节点不不存在
					bucket_slot()[slot] = current_offset;
				}
				
				return TFS_SUCCESS;
				
			}
			
			

	
	}
}