#include <iostream>
#include <string>
#include <rocksdb/db.h>
#include <rocksdb/iterator.h>
#include <rocksdb/table.h>
#include <rocksdb/options.h>
#include <rocksdb/env.h>
#include <rocksdb/iostats_context.h>
#include <rocksdb/perf_context.h>
#include <rocksdb/rate_limiter.h>
#include <ctime>

using namespace std;


static string rand_key(unsigned long long key_range) {
    char buff[30];
    unsigned long long n = 1;

    for (int i =1; i <= 4; ++i) {
        n *= (unsigned long long ) rand();
    }

    sprintf(buff, "%llu", n % key_range);

    string k(buff);
    return k;
}

int main() {
    rocksdb::DB *db;
    rocksdb::Options option;

    option.create_if_missing = true;
    option.compression = rocksdb::CompressionType::kNoCompression;
    

    rocksdb::Status s = rocksdb::DB::Open(option, "./iterator_db", &db);
    if (!s.ok()) {
        cout << "Open failed with " << s.ToString() << endl;
        exit(1);
    }
    

    rocksdb::RateLimiter *rate_limiter = rocksdb::NewGenericRateLimiter(1000);

    rocksdb::WriteOptions wopts;
    wopts.sync = true;

    db->Put(wopts, "key1", "value1" );
    
    string value;
    // db->Get(rocksdb::ReadOptions(), "key1", &value);
    // cout << "key1's value :" << value << endl;

    for (int i = 0;i < 10000; ++i) {
        db->Put(wopts, std::to_string(i), string(10, 'a' + (i % 26)) );
    }

    db->SingleDelete(wopts, "key1");
    db->Get(rocksdb::ReadOptions(), "key1", &value);
    cout << "key1's value after singledelete :" << value << endl;

    db->Put(wopts, "key1", "value2" );
    db->Get(rocksdb::ReadOptions(), "key1", &value);
    cout << "key1's value :" << value << endl;

    db->Put(wopts, "key1", "value3" );
    db->Get(rocksdb::ReadOptions(), "key1", &value);
    cout << "key1's value :" << value << endl;

    rate_limiter->Request(1024, rocksdb::Env::IO_HIGH);
    cout << "if rate limit " << rate_limiter->IsRateLimited(rocksdb::RateLimiter::OpType::kWrite);
    

    db->Close();
    delete db;

    return 0;
}