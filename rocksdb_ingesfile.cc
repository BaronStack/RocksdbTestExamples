#include <iostream>
#include <vector>

#include <gflags/gflags.h>

#include <rocksdb/db.h>
#include <rocksdb/env.h>
#include <rocksdb/sst_file_writer.h>


#define DATA_SIZE 10
#define VALUE_SIZE 1024

using namespace std;

bool cmp(pair<string, string> str1,
         pair<string, string> str2) {
  if(str1.first < str2.first) {
    return true;
  } else if (str1.first == str2.first 
            && str1.second < str2.second) {
    return true;
  } else {
    return false;
  }
}

static string rand_data(long data_range) {
    char buff[30];
    unsigned long long num = 1;
    for (int i = 0;i < 4; ++i) {
        num *= (unsigned long long )rand();
    }

    sprintf(buff, "%llu", num % (unsigned long long)data_range );
    string data(buff);

    return data;
}

void construct_data(vector<pair<string,
                    string>> &input) {
  int i;
  string key;
  string value;

  for (i = 0;i < DATA_SIZE; i++) {
    if(key == "0") {
      continue;
    }
    key = rand_data(VALUE_SIZE);
    value = rand_data(VALUE_SIZE);

    input.push_back(make_pair(key, value));
  }
}

void traverse_data(vector<pair<string,string>> input) {
  int i;
  for(auto data : input) {
    cout << data.first << " " << data.second << endl;
  }
}

int create_sst(string file_path) {
  vector<pair<string,string>> input;
  vector<pair<string,string>>::iterator input_itr;
  rocksdb::Options option;

  /* open statistics and disable compression */
  option.create_if_missing = true;
  option.compression = rocksdb::CompressionType::kNoCompression;

  rocksdb::SstFileWriter sst_file_writer(rocksdb::EnvOptions(), option);

  rocksdb::Status s = sst_file_writer.Open(file_path);
  if (!s.ok()) {
    printf("Error while opening file %s, Error: %s\n", 
        file_path.c_str(),s.ToString().c_str());
    return 1;
  }

  construct_data(input);
  sort(input.begin(), input.end(), cmp);
  traverse_data(input);
  // Insert rows into the SST file, note that inserted keys must be 
  // strictly increasing (based on options.comparator)
  for (input_itr = input.begin(); input_itr != input.end();
                                  input_itr ++) {
    rocksdb::Slice key(input_itr->first);
    rocksdb::Slice value(input_itr->second);

    s = sst_file_writer.Put(key, value);
    if (!s.ok()) {
      printf("Error while adding Key: %s, Error: %s\n",
          key.ToString().c_str(),s.ToString().c_str());
      return 1;
    }
  }

  // Close the file
  s = sst_file_writer.Finish();
  if (!s.ok()) {
    printf("Error while finishing file %s, Error: %s\n", 
        file_path.c_str(),s.ToString().c_str());
    return 1;
  }

  return 0;
}

static rocksdb::DB *db;

void create_db() {
  rocksdb::Options option;

  /* open statistics and disable compression */
  option.create_if_missing = true;
  option.compression = rocksdb::CompressionType::kNoCompression;

  rocksdb::Status s = rocksdb::DB::Open( option,"./db", &db);
  if (!s.ok()) {
    printf("Open db failed : %s\n", s.ToString().c_str());
    return;
  }
}

void db_write(int num_keys) {
  rocksdb::WriteOptions write_option;
  write_option.sync = true;

  rocksdb::Slice key;
  rocksdb::Slice value;
  rocksdb::Status s;
  int i;

  printf("begin write \n");
  for (i = 0;i < num_keys; i++) {
    key = rand_data(VALUE_SIZE);
    value = rand_data(VALUE_SIZE);

    s = db->Put(write_option, key, value);
    if (!s.ok()) {
      printf("Put db failed : %s\n", s.ToString().c_str());
      return;
    }
  }

  db->Flush(rocksdb::FlushOptions());
  printf("finish write \n");
}

int main() {
  if (create_sst("./test.sst") == 0) {
    printf("creates sst success !\n");
  } else {
    printf("creates sst failed !\n");
  }

  create_db();
  db_write(100000);
  
  rocksdb::IngestExternalFileOptions ifo;
  // Ingest the 2 passed SST files into the DB
  rocksdb::Status s = db->IngestExternalFile({"test.sst"}, ifo);
  if (!s.ok()) {
    printf("Error while adding file test.sst , Error %s\n",
           s.ToString().c_str());
    return 1;
  }

  return 0;
}

