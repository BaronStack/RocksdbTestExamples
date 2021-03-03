#include <iostream>
#include <vector>

#include <rocksdb/db.h>
#include <rocksdb/table.h>
#include <rocksdb/options.h>
#include <rocksdb/merge_operator.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/perf_context.h>
#include <rocksdb/iostats_context.h>
#include <rocksdb/trace_reader_writer.h>

using namespace rocksdb;

// A 'model' merge operator with uint64 addition semantics
class UInt64AddOperator : public AssociativeMergeOperator {
    public:
    virtual bool Merge(
    const Slice& key,
    const Slice* existing_value,
    const Slice& value,
    std::string* new_value,
    Logger* logger) const override {

    // assuming 0 if no existing value
    uint64_t existing = 0;
    if (existing_value) {
        if (!Deserialize(*existing_value, &existing)) {
        // if existing_value is corrupted, treat it as 0
        Log(logger, "existing value corruption");
        existing = 0;
        }
    }

    uint64_t oper;
    if (!Deserialize(value, &oper)) {
        // if operand is corrupted, treat it as 0
        Log(logger, "operand value corruption");
        oper = 0;
    }

    auto new = existing + oper;
    *new_value = Serialize(new);
    return true;        // always return true for this, since we treat all errors as "zero".
    }

    virtual const char* Name() const override {
    return "UInt64AddOperator";
    }
};

int main() {

    return 0;
}