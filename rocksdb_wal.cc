#include <iostream>
#include <string>
#include <vector>
#include <rocksdb/db.h>
#include <rocksdb/iterator.h>
#include <rocksdb/table.h>
#include <rocksdb/options.h>
#include <rocksdb/env.h>
#include <ctime>

using std::cout;
using std::endl;
using std::string;

int main () {
    rocksdb::DB* db;
    rocksdb::Options options;
    rocksdb::Status s;
    
    // std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
    // column_families.push_back(rocksdb::ColumnFamilyDescriptor(
    //     rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions()));
    // column_families.push_back(rocksdb::ColumnFamilyDescriptor(
    //     "new_cf", rocksdb::ColumnFamilyOptions()));
    // std::vector<rocksdb::ColumnFamilyHandle*> handles;

    options.create_if_missing = true;
    //options.max_open_files = -1;

    // s = rocksdb::DB::Open(options, "./db/", &db);

    // create column family
    // rocksdb::ColumnFamilyHandle* cf;
    // s = db->CreateColumnFamily(rocksdb::ColumnFamilyOptions(), "new_cf", &cf);
    // assert(s.ok());

    // // close DB
    // s = db->DestroyColumnFamilyHandle(cf);
    // assert(s.ok());
    // delete db;

    // s = rocksdb::DB::Open(options, "./db", column_families, &handles, &db);
    s = rocksdb::DB::OpenForReadOnly(options, "./db", &db);
    if(!s.ok()) {
        cout << "open failed status " << s.ToString() << endl;
    }

    s = db->Put(rocksdb::WriteOptions(),"key1", "value1");
    if(!s.ok()) {
        cout << "Get failed status " << s.ToString() << endl;
    }

    int count = 0;
    string value;
    for(int i = 0;i < 99999; ++i) {
        s = db->Get(rocksdb::ReadOptions(),std::to_string(i), &value);
        if(s.ok()) {
            count ++;
        }
    }

    cout << "Get "
         << count 
         << "of (9999)"
         << endl;

    // cout << handles.size() << " open status is : " << s.ToString()  << endl;
    // db->Put(rocksdb::WriteOptions(), handles[1], rocksdb::Slice("key1"), rocksdb::Slice("value1"));
    // db->Put(rocksdb::WriteOptions(), handles[0], rocksdb::Slice("key2"), rocksdb::Slice("value2"));
    // db->Put(rocksdb::WriteOptions(), handles[1], rocksdb::Slice("key3"), rocksdb::Slice("value3"));
    // db->Put(rocksdb::WriteOptions(), handles[0], rocksdb::Slice("key4"), rocksdb::Slice("value4"));

    // db->Flush(rocksdb::FlushOptions(), handles[1]);
    // // key5 and key6 will appear in a new WAL
    // db->Put(rocksdb::WriteOptions(), handles[0], rocksdb::Slice("key5"), rocksdb::Slice("value5"));
    // db->Put(rocksdb::WriteOptions(), handles[0], rocksdb::Slice("key6"), rocksdb::Slice("value6"));
    
    // db->Flush(rocksdb::FlushOptions(), handles[0]);

    delete db;

    return 0;
}