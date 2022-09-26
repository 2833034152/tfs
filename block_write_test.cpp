#include "common.h"
#include "file_op.h"
#include "index_handle.h"

#include <sstream>
using namespace qiqi;
using namespace std;
static int debug = 1;
const static largefile::MMapOption mmap_option = {1024000,4096,4096};
const static uint32_t main_blocksize =  1024 * 1024 * 64; //main block的大小

const static uint32_t bucket_size = 1000;  //哈希桶的大小

static int32_t block_id = 1;
int main(int argc, char** argv){
	
	std::string main_block_path;
	std::string index_path;
	
	int32_t ret = largefile::TFS_SUCCESS;
	cout<<"Type your blockid:"<<endl;
	cin>>block_id;
	
	if(block_id < 1){
		cerr << "invalid block_id, exit	"<<endl;
		exit(-1);
	}
	
	
		
		//1.加载索引文件
		
		largefile::IndexHandle* index_handle = new largefile::IndexHandle(".", block_id);  
		if(debug) printf("load index...\n");
		ret = index_handle->load(block_id, bucket_size, mmap_option);
		if(ret != largefile::TFS_SUCCESS){ 
			fprintf(stderr, "load index %d failed.\n",block_id);
//			delete mainblock;
			delete index_handle;
			exit(-3);
		}
		printf("42\n");
		
		//2. 写入文件到主块文件
		std::stringstream tmp_stream;
		tmp_stream << "."<< largefile::MAINBLOCK_DIR_PREFIX <<block_id;	
		
		tmp_stream >>main_block_path;
		
		largefile::FileOption * mainblock = new largefile::FileOption(main_block_path,  O_RDWR|O_LARGEFILE |O_CREAT);
		
//		ret = mainblock->ftruncate_file(main_blocksize);
		
		char buffer[4096];
		memset(buffer , '6', sizeof(buffer));
		
		int32_t data_offset = index_handle->get_block_data_offset();
		uint32_t file_no = index_handle->block_info()->seq_no_;
		
		if((ret = mainblock->pwrite_file(buffer, sizeof(buffer), data_offset)) !=  largefile::TFS_SUCCESS){
			fprintf(stderr, "write to main block failed. ret:%d, reason:%s\n", ret, strerror(errno));
			mainblock->close_file();
			
//			delete mainblock;
			delete index_handle;
			exit(-4);
		}
		
		//3.索引文件中写入MetaInfo
		largefile::MetaInfo  meta;
		meta.set_file_id(file_no);        //小文件的编号
		meta.set_size(sizeof(buffer));    //小文件的大小
		meta.set_offset(data_offset);
		
		ret = index_handle->write_segment_meta(meta.get_key(), meta);
		if(ret == largefile::TFS_SUCCESS){
			//1.更新索引头部信息
			index_handle->commit_block_data_offset(sizeof(buffer));

			//2.更新块信息
			index_handle->update_block_info(largefile::C_OPER_INSERT, 	sizeof(buffer));   //，由于前面流程挺严谨，ret出错的可能性很小，所以就不对ret进行判断了
			
			ret = index_handle->flush();
			
			if(ret != largefile::TFS_SUCCESS){
				fprintf(stderr,"flush mainblock %d failed.file no :%u\n",block_id, file_no);
			
			}

		}else{
			fprintf(stderr,"write_segment_meta -mainblock %d failed. file no.%u\n",block_id, file_no);
		  }
		mainblock->close_file();
			
		delete mainblock;
		delete index_handle;
		
		return 0;
}