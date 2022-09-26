#ifndef INDEX_HANDLE_H_
#define INDEX_HANDLE_H_

#include "common.h"

#include "mmap_file_op.h"

namespace  qiqi{
	namespace largefile{
		
		
		
		struct IndexHeader{
		  public:
			IndexHeader(){
				memset(this , 0, sizeof(IndexHeader));
			}
			
			
			BlockInfo block_info_;
			int32_t   bucket_size_;
			int32_t   data_file_offset_;   //offset to write next data in block_info
			int32_t index_file_size_;      //索引文件当前偏移 index header + all buckets
			int32_t free_header_offset_;      //可重用的 free meta node list for reuse
		};
		
		class IndexHandle{
		   public:
			IndexHandle(const std::string & base_path, const uint32_t main_block_id);
			~IndexHandle();
			int create(const uint32_t logic_block_id, const int32_t bucket_size, const MMapOption map_option);
			
			int load(const uint32_t logic_block_id, const int32_t bucket_size, const MMapOption map_option);
		    IndexHeader * index_header(){
				return  reinterpret_cast<IndexHeader*>(file_op_->get_map_data());
				
			}
			
			int remove(const uint32_t logi_block_id);
			int flush();
			int update_block_info(const OperType oper_type, const uint32_t modify_size);
			
			BlockInfo* block_info() {
				return reinterpret_cast<BlockInfo*>(file_op_->get_map_data());
			}
			
			int32_t* bucket_slot(){
				return reinterpret_cast<int32_t*>(reinterpret_cast<char*>(file_op_->get_map_data())  + sizeof(IndexHeader) );
			}
			
			int32_t bucket_size() const {
				return reinterpret_cast<IndexHeader*>(file_op_->get_map_data())->bucket_size_;
			}
			
			int32_t get_block_data_offset(){
				return reinterpret_cast<IndexHeader*>(file_op_->get_map_data())->data_file_offset_;
			}
			
			void commit_block_data_offset(const int file_size){
            //忘记->的-导致get_map_data() was not declared 报错
				reinterpret_cast<IndexHeader*>(file_op_->get_map_data())->data_file_offset_ += file_size;
			}
			
			int32_t free_header_offset() const{
				return reinterpret_cast<IndexHeader*>(file_op_->get_map_data())->free_header_offset_;
			}
			
			int32_t write_segment_meta(const uint64_t key, MetaInfo & meta);
			
			int32_t read_segment_meta(const uint64_t key, MetaInfo & meta);
			
			int32_t delete_segment_meta(const uint64_t key);
			
			int32_t hash_find(const uint64_t key,int32_t &current_offset, int32_t & previous_offset);
			
			int32_t hash_insert(const uint64_t key, int32_t previous_offset, MetaInfo &meta);
			
		   private:
			bool  hash_compare(const uint64_t left_key, const uint64_t right_key){
				return (left_key == right_key);
			}
		   
		    MMapFileOpton * file_op_;
			bool is_load_;        
		};
	}
}







#endif //INDEX_HANDLE_H_