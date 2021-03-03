#include <gtest/gtest.h>

#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/snapshot.h>
#include <thread>

using namespace std;

bool stop = false;
void DeleteRange(rocksdb::DB* db)
{
  while(!stop) {
    rocksdb::WriteOptions delete_range_option;
    db->DeleteRange(delete_range_option, db->DefaultColumnFamily(), "1", "2");
  }
}

TEST(Basic, SingleThread)
{
  rocksdb::DB* db;
  rocksdb::Options options;
  options.create_if_missing = true;
  options.allow_concurrent_memtable_write = false;

  char* tmpfile = "./db";

  rocksdb::DB::Open(options, tmpfile, &db);

  std::thread t1(DeleteRange, db);
  std::thread t2(DeleteRange, db);

  sleep(3);
  stop = true;
  t1.join();
  t2.join();
  stop = false;
}

TEST(Basic, MultiThread)
{
  rocksdb::DB* db;
  rocksdb::Options options;
  options.create_if_missing = true;
  options.allow_concurrent_memtable_write = true;

  char* tmpfile = "./db";

  rocksdb::DB::Open(options, tmpfile, &db);

  std::thread t1(DeleteRange, db);
  std::thread t2(DeleteRange, db);

  sleep(3);
  stop = true;
  t1.join();
  t2.join();
  stop = false;
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}