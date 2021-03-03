#include <iostream>
#include <unistd.h>
#include <ctime>
#include <string>
#include <rocksdb/db.h>
#include <rocksdb/table.h>
#include <rocksdb/options.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/perf_context.h>
#include <rocksdb/iostats_context.h>
#include <rocksdb/trace_reader_writer.h>

using namespace  std;

static char *getTime()
{
  time_t nowtime;
  nowtime = time(NULL);
  return ctime(&nowtime);
}

static string rand_key(long key_range) {
    char buff[30];
    unsigned long long num = 1;
    for (int i = 0;i < 4; ++i) {
        num *= (unsigned long long )rand();
    }

    sprintf(buff, "%llu", num % (unsigned long long)key_range );
    string key(buff);

    return key;
}

int main() {
    rocksdb::DB *db = nullptr;
    rocksdb::DB *test_db = nullptr;

    rocksdb::Options option;
    rocksdb::Env *env;
    rocksdb::EnvOptions env_options;
    std::unique_ptr<rocksdb::TraceWriter> trace_writer;
    std::string trace_path = "./trace_file";
    rocksdb::TraceOptions trace_opt;

    rocksdb::BlockBasedTableOptions bbto;
    bbto.cache_index_and_filter_blocks = true;
    bbto.whole_key_filtering = true;
    bbto.filter_policy .reset(rocksdb::NewBloomFilterPolicy(16,false)); // blockbased bloomfilter and fullfilter performance
    option.table_factory.reset(rocksdb::NewBlockBasedTableFactory(bbto));

    /* open statistics and disable compression */
    option.create_if_missing = true;
    option.statistics = rocksdb::CreateDBStatistics();
    option.compression = rocksdb::CompressionType::kNoCompression;

    // NewFileTraceWriter(env, env_options, trace_path, &trace_writer);
    rocksdb::Status s = rocksdb::DB::Open(option,"./db" , &db);
    if (!s.ok()) {
       cout << "Open db failed! Result is " << s.ToString() << endl;  
       exit(1);
    }

    test_db = db;

    cout << " compaction_trigger : " << option.level0_file_num_compaction_trigger << endl;


    // vector <rocksdb::ColumnFamilyHandle *> cfd;
    // cfd.push_back(db->DefaultColumnFamily());

    // db->EnableAutoCompaction(cfd);

    /* perf stat */
    rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeAndCPUTimeExceptForMutex);
    rocksdb::get_perf_context()->Reset();
    rocksdb::get_iostats_context()->Reset();
    
    cout << "Put begin " 
         << getTime() 
         << endl;


    // db->StartTrace(trace_opt, std::move(trace_writer));
    clock_t ts = clock();
    for (int i = 0;i < 99999; ++i) {
        /* random key */
        
        //rocksdb::Status s = db->Put(rocksdb::WriteOptions(),rand_key(999999), string(500, 'a' + (i % 26)));
        rocksdb::Status s = db->Put(rocksdb::WriteOptions(),rand_key(999999), string(""));
        if(!s.ok()) {
            cout << "Put db failed! Result is " << s.ToString() << endl;  
            exit(1);
        }
        if(i == 9999) {
            db->Delete(rocksdb::WriteOptions(), rand_key(i));
            s = db->SetOptions(db->DefaultColumnFamily(), {{"disable_auto_compactions","1"}});
            if(s.ok()) {
                cout << "SetOptions result: " << s.ToString() << endl;
            }
        }

        if(i == 111111) {
            s = db->SetOptions(db->DefaultColumnFamily(), {{"disable_auto_compactions","0"}});
            if(s.ok()) {
                cout << "SetOptions result: " << s.ToString() << endl;
            }
        }
    }
    cout << "End put , time is " << clock() - ts << endl; 

    // s = db->SetOptions({{"level0_file_num_compaction_trigger","3"}});


    cout << " compaction_trigger : " << option.level0_file_num_compaction_trigger << endl;

    cout << "Get begin " << endl;

    string value;
    int count = 0;
     ts = clock();
    for(int i = 0;i < 99999; ++i) {
        rocksdb::Status s = test_db->Get(rocksdb::ReadOptions(),rand_key(999999), &value);
        if(s.ok()) {
            count ++;
           cout <<  "get " << i << "'s value is " << value << endl;
        } else {
           cout <<  "get " << i << "'s value is " << value << endl;
        }
    }
    cout << "End get , time is " << clock() - ts << endl; 
    cout << "Get "
         << count 
         << " of (99999) "
         << endl;

    db->EndTrace();

    rocksdb::SetPerfLevel(rocksdb::PerfLevel::kDisable);

    /* print write time on cpu,and bloom filter's miss count */
    cout << "rocksdb iostats cpu_write_nanos:" 
         << rocksdb::get_iostats_context()->ToString()
         << " \nrocksdb perf stats : bloom_sst_miss_count "
         << rocksdb::get_perf_context()->bloom_sst_miss_count << endl;


    sleep(100);
    delete db;
    return 0;
}
