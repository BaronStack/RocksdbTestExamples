CXX = g++
CFLAGS = -std=c++11
LIB = -lrocksdb

all: rocksdb_delete_range rocksdb_filter rocksdb_singledelete \ 
	 rocksdb_ingest rocksdb_transaction sst_file_checker \
	 rocksdb_approximatesize

rocksdb_delete_range: rocksdb_delete_range.cc
	$(CXX) $(CFLAGS) rocksdb_delete_range.cc -o$@ $(LIB)

rocksdb_filter: rocksdb_filter.cc
	$(CXX) $(CFLAGS) rocksdb_filter.cc -o$@ $(LIB)

rocksdb_singledelete: rocksdb_singledelete.cc
	$(CXX) $(CFLAGS) rocksdb_singledelete.cc -o$@ $(LIB)

rocksdb_ingest: rocksdb_ingesfile.cc
	$(CXX) $(CFLAGS) rocksdb_ingesfile.cc -o$@ $(LIB)

rocksdb_transaction: rocksdb_transaction.cc
	$(CXX) $(CFLAGS) rocksdb_transaction.cc -o$@ $(LIB)

sst_file_checker: sst_file_checker.cc
	$(CXX) $(CFLAGS) sst_file_checker.cc -o$@ $(LIB)

rocksdb_approximatesize: rocksdb_approximatesize.cc
	$(CXX) $(CFLAGS) rocksdb_approximatesize.cc -o$@ $(LIB)
clean:
	rm -rf rocksdb_delete_range rocksdb_filter rocksdb_singledelete rocksdb_ingest rocksdb_transaction sst_file_checker rocksdb_approximatesize
