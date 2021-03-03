#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <rocksdb/db.h>
#include <rocksdb/sst_file_writer.h>

void check(rocksdb::Status &&status) {
	if (status.ok()) {
		return;
	}

	std::cerr << status.ToString() << std::endl;
	exit(1);
}

int main() {
	char buf[] = "/tmp/test-XXXXXX";
	char *dir = mkdtemp(buf);
	if (dir == nullptr) {
		perror("mkdtemp");
		exit(1);
	}

	rocksdb::Options options;
	options.create_if_missing = true;
	
	rocksdb::DB *db;
	check(rocksdb::DB::Open(options, dir, &db));

	check(db->Put(rocksdb::WriteOptions{}, "b", "hello"));

	std::string value;

	char sst_buf[] = "/tmp/sst-XXXXXX";
	char *sst_file = mktemp(sst_buf);
	if (sst_file == nullptr) {
		perror("mktemp");
		exit(1);
	}

	rocksdb::SstFileWriter writer{rocksdb::EnvOptions{}, db->GetOptions(db->DefaultColumnFamily())};
	check(writer.Open(sst_file));
	check(writer.DeleteRange("a", "c"));
	check(writer.Put("a", "hola"));
	check(writer.Finish());

	rocksdb::IngestExternalFileOptions ingest_options;
	ingest_options.move_files = true;

	check(db->IngestExternalFile(db->DefaultColumnFamily(), {sst_file}, ingest_options));

	check(db->Get(rocksdb::ReadOptions{}, "a", &value));
	std::cout << "a: " << value << std::endl;

	rocksdb::Status status = db->Get(rocksdb::ReadOptions{}, "b", &value);
	if (status.ok()) {
		std::cout << "b: " << value << std::endl;
	} else if (status.IsNotFound()) {
		std::cout << "b is null" << std::endl;
	} else {
		std::cerr << status.ToString() << std::endl;
		exit(1);
	}
    return 0;
}