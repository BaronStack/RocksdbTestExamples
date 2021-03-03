#include <iostream>
#include <string>

#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/transaction_log.h>
#include <rocksdb/utilities/transaction_db.h>
#include <rocksdb/snapshot.h>
#include <rocksdb/rate_limiter.h>

using namespace rocksdb;

int main() {
    rocksdb::TransactionDB *txn_db;
    rocksdb::TransactionDBOptions txn_option;
    rocksdb::DB *db;
    rocksdb::Options option;

    option.create_if_missing = true;

    rocksdb::Status s = txn_db->Open(option, txn_option, "./txn_db", &txn_db);
    assert(s.ok());
    db = txn_db->GetBaseDB();

    WriteOptions write_options;
    ReadOptions read_options;
    TransactionOptions txn_options;
    std::string value;

    Transaction* txn = txn_db->BeginTransaction(write_options);
    assert(txn);
    value="aaa";
    // Set a snapshot at start of transaction by setting set_snapshot=true
    txn_options.set_snapshot = true;
    txn = txn_db->BeginTransaction(write_options, txn_options);

    ////////////////////////////////////////////////////////
    //
    // "Read Committed" (Monotonic Atomic Views) Example
    //   --Using multiple Snapshots
    //
    ////////////////////////////////////////////////////////

    // In this example, we set the snapshot multiple times.  This is probably
    // only necessary if you have very strict isolation requirements to
    // implement.

    // Set a snapshot at start of transaction
    txn_options.set_snapshot = true;
    txn = txn_db->BeginTransaction(write_options, txn_options);

    // Do some reads and writes to key "x"
    read_options.snapshot = txn_db->GetSnapshot();
    read_options.snapshot->GetSequenceNumber();
    std::cout << "get snapshot 1 is " << read_options.snapshot->GetSequenceNumber() << std::endl;
    s = txn->Get(read_options, "x", &value);
    std::cout << "get 'x' is " << value << std::endl;
    txn->Put("x", "x");
    s = txn->Get(read_options, "x", &value);
    std::cout << "after put, and get 'x' is " << value << std::endl;

    // Do a write outside of the transaction to key "y"
    s = txn_db->Put(write_options, "y", "y");
    assert(s.ok());

    // Set a new snapshot in the transaction
    txn->SetSnapshot();
    s = txn->Get(read_options, "y", &value);
    std::cout << "befor savepoint, and get 'y' is " << value << std::endl;
    txn->SetSavePoint();
    read_options.snapshot = txn_db->GetSnapshot();
    std::cout << "get snapshot 2 is " << read_options.snapshot->GetSequenceNumber() << std::endl;
    

    // Do some reads and writes to key "y"
    // Since the snapshot was advanced, the write done outside of the
    // transaction does not conflict.
    s = txn->GetForUpdate(read_options, "y", &value);
    txn->Put("y", "xxx");
    s = txn->Get(read_options, "y", &value);
    std::cout << "after savepoint, and 'y' is " << value << std::endl;
    txn->Put("y", "ddd");

    // Decide we want to revert the last write from this transaction.
    txn->RollbackToSavePoint();
    value="i am not update";
    s = txn->Get(read_options, "y", &value);
    std::cout << "after rollback, and 'y' is " << value << std::endl;

    // Commit.
    s = txn->Commit();
    assert(s.ok());
    delete txn;
    // Clear snapshot from read options since it is no longer valid
    read_options.snapshot = nullptr;

    // Cleanup
    delete txn_db;
    DestroyDB("./txn_db", option);
    return 0;

}