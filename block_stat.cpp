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
	
	printf("block  stat test\n");
		
	//1.加载索引文件
	
	largefile::IndexHandle* index_handle = new largefile::IndexHandle(".", block_id);  
	if(debug) printf("load index...\n");
	
	printf("(block_id:%d, bucket_size%d\n",block_id, bucket_size);
	ret = index_handle->load(block_id, bucket_size, mmap_option);
	if(ret != largefile::TFS_SUCCESS){ 
		fprintf(stderr, "load index %d failed.\n",block_id);
//			delete mainblock;
		delete index_handle;
		exit(-3);
	}
	
	delete index_handle;
	return 0;
}