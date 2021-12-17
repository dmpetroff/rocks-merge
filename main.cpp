#include <rocksdb/db.h>
#include <rocksdb/merge_operator.h>
#include <iostream>
#include <cstdlib>
#include <assert.h>

// Note: code will system("rm -fr " DB_DIR) !!!
#define DB_DIR "./db"

class DummyMergeOperator : public rocksdb::MergeOperator {
private:
	bool FullMerge(const rocksdb::Slice &key, const rocksdb::Slice *existing_value,
	               const std::deque<std::string> &operands_list, std::string *new_value,
	               rocksdb::Logger *logger) const override {
		*new_value = "FullMerge";
		std::cout << "    FullMerge "
			<< (existing_value ? "SOME_VALUE" : "<empty>")
			<< " + " << operands_list.size() << " merge ops" << std::endl;
		return true;
	}
	bool FullMergeV2(const MergeOperationInput &merge_in,
	                 MergeOperationOutput *merge_out) const override {
		merge_out->new_value = "FullMergeV2";
		std::cout << "    FullMergeV2 "
			<< (merge_in.existing_value ? "SOME_VALUE" : "<empty>")
			<< " + " << merge_in.operand_list.size() << " merge ops" << std::endl;
		return true;
	}

	bool PartialMergeMulti(const rocksdb::Slice &key, const std::deque<rocksdb::Slice> &operand_list,
	                       std::string *new_value, rocksdb::Logger *logger) const override {
		*new_value = "PartialMergeMulti";
		std::cout << "    PartialMergeMulti " << operand_list.size() << " merge ops" << std::endl;
		return true;
	}
	const char* Name() const override { return "DummyMergeOperator"; }
	//bool AllowSingleOperand() const override { return true; }
};

std::unique_ptr<rocksdb::DB> db_open(const char *dir)
{
	rocksdb::Options opts;
	opts.create_if_missing = true;
	opts.merge_operator = std::make_shared<DummyMergeOperator>();

	rocksdb::DB *db;
	auto status = rocksdb::DB::Open(opts, dir, &db);
	assert(status.ok());

	return std::unique_ptr<rocksdb::DB>(db);
}

int main()
{
	std::system("rm -fr " DB_DIR);

	auto db = db_open(DB_DIR);

	const rocksdb::Slice key("1");
	auto put = [&]() {
		auto s = db->Put(rocksdb::WriteOptions(), key, "p");
		std::cout << "=== Put" << std::endl;
		assert(s.ok());
	};
	auto merge = [&]() {
		std::cout << "+++ Merge" << std::endl;
		auto s = db->Merge(rocksdb::WriteOptions(), key, "m");
		assert(s.ok());
	};
	auto compact = [&]() {
		std::cout << "--- COMPACT" << std::endl;
		auto s = db->CompactRange(rocksdb::CompactRangeOptions(), nullptr, nullptr);
		assert(s.ok());
	};
	auto get = [&]() {
		std::string value;
		std::cout << ">>> Get" << std::endl;
		auto s = db->Get(rocksdb::ReadOptions(), key, &value);
		assert(s.ok());
		std::cout << "\033[s\033[2A\033[8Cvalue = " << value << "\033[u" << std::flush;
	};

	//put();
	merge();
	merge();
	get();
	compact(); // If you uncomment put, this will invoke FullMergeV2, otherwise PartialMergeMulti is invoked
	merge();
	compact();

	return 0;
}
