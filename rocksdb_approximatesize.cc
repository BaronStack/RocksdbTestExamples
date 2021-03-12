#include <iostream>
#include <string>

#include <rocksdb/db.h>
#include <rocksdb/slice.h>


#define VALUE_SIZE 1024

using namespace std;
using namespace rocksdb;

void check_status(Status s, std::string op) {
	if (!s.ok()) {
		cout << " Excute " << op << " failed "
			 << s.ToString() << endl;
		exit(1);
	}
}

static std::string Key(int i) {
	char buf[100];
	snprintf(buf, sizeof(buf), "key%06d", i);
	return std::string(buf);
}

int main() {
    rocksdb::DB* db;
    rocksdb::Options options;
    rocksdb::Status s;
    
    options.create_if_missing = true;
	options.compression = kNoCompression;

	check_status(rocksdb::DB::Open(options, "./db", &db), "Open DB");

	for (int i = 0;i < 10; i++) {
		check_status(db->Put(WriteOptions(), 
							Key(i), 
							Slice(string(VALUE_SIZE, 'a' + (i % 26)))), 
					"Put DB");
	}

	uint64_t size;
	string start = Key(1);
	string end = Key(3);
	Range r(start, end);
	db->GetApproximateSizes(&r, 1, &size);
	cout << "Approximate size is " << size << endl; 
	
	uint8_t include_both = DB::SizeApproximationFlags::INCLUDE_FILES |
                         DB::SizeApproximationFlags::INCLUDE_MEMTABLES;

	db->GetApproximateSizes(&r, 1, &size, include_both);
	cout << "After set memtable flag, Approximate size is " << size << endl; 

	db->Flush(FlushOptions());
	db->GetApproximateSizes(&r, 1, &size);
	cout << "After flush, Approximate size is " << size << endl; 

	delete db;

  return 0;
}