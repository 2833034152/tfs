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
		
		//2. 读取文件
		uint64_t file_id;
		cout<< "Type your file id "<<endl;
		cin >> file_id;
		
		if(file_id <  1){
			cerr<<"Invalid file id"<<endl;
			exit(-4);
		}
		
		largefile::MetaInfo meta;            //读取得到meta中了
		ret  = index_handle->read_segment_meta(file_id, meta);
		if( ret != largefile::TFS_SUCCESS){
			fprintf(stderr, "read_segment_meta failed.error:file_id :%lu,ret:%s\n",file_id, strerror(errno));
			exit(-5);
		}
		
		//把主块也给读过来
		std::stringstream tmp_stream;
		tmp_stream << "."<< largefile::MAINBLOCK_DIR_PREFIX <<block_id;	
		
		tmp_stream >>main_block_path;
		
		largefile::FileOption * mainblock = new largefile::FileOption(main_block_path,  O_RDWR|O_LARGEFILE |O_CREAT);
		char buffer[4096];
		ret = mainblock->pread_file(buffer , meta.get_size(), meta.get_offset());            //meta中记录的主块的位置和大小
		
		if( largefile::TFS_SUCCESS != ret){
			fprintf(stderr, "read from main block failed.ret:%d，reason:%s\n", ret, strerror(errno));
			mainblock->close_file();
			delete mainblock;
			delete index_handle;
			exit(-5);
		}
		buffer[meta.get_size()] = '\0';
		printf("read size:%d , content:%s\n", meta.get_size(),buffer);
		
		mainblock->close_file();
		delete mainblock;
		delete index_handle;
		
		return 0;
		
				
}		