#include <iostream>
#include <string>
#include <rocksdb/db.h>
#include <rocksdb/iterator.h>
#include <rocksdb/table.h>
#include <rocksdb/options.h>
#include <rocksdb/env.h>
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

    rocksdb::DestroyDB("./iterator_db", option);

    for(int i = 0; i < 1000000; i ++) {
        rocksdb::Status s = db->Put(rocksdb::WriteOptions(), 
                                to_string(i), string(10, 'a' + (i % 26)) );

        if (!s.ok()) {
            cout << "Put failed with " << s.ToString() << endl;
            exit(1);
        }
    }   

    clock_t ts = clock();
    cout << "after put , seek all keys :" << endl;
    rocksdb::ReadOptions read_option;
    auto it=db->NewIterator(read_option);
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        //cout << it->key().ToString() << " " << it->value().ToString() << endl;
        assert(it->Valid());
    }
    cout << "oridinary seek use " << clock() - ts << endl;

    string start("1000");
    string end("5000");
    ts = clock();
    //for(it ->Seek(start); it->Valid() && it->key().ToString() < end; it->Next()) {
    //    db->Delete(rocksdb::WriteOptions(), it->key());
    //}
    s = db->DeleteRange(rocksdb::WriteOptions(),db->DefaultColumnFamily(), start, end);
    assert(s.ok());
    cout << "DeleteRange use " << clock() - ts << endl;

    cout << "after DeleteRange from 2-5 , seek all keys :" << endl;
    ts = clock();
    it = db->NewIterator(read_option);
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        //cout << it->key().ToString() << " " << it->value().ToString() << endl;
        assert(it->Valid());
    }
    cout << "after DeletRange ,seek use " << clock() - ts << endl;

    delete db;

    return 0;
}