//  Copyright (c) 2017-present The blackwidow Authors.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.

#ifndef SRC_LISTS_FILTER_H_
#define SRC_LISTS_FILTER_H_

#include <string>
#include <memory>
#include <vector>

#include "src/lists_meta_value_format.h"
#include "src/lists_data_key_format.h"
#include "src/debug.h"
#include "rocksdb/compaction_filter.h"

namespace blackwidow {

class ListsMetaFilter : public rocksdb::CompactionFilter {
 public:
  ListsMetaFilter() = default;
  virtual bool Filter(int level, const rocksdb::Slice& key,
                      const rocksdb::Slice& value,
                      std::string* new_value, bool* value_changed) const
      override {
    ParsedListsMetaValue parsed_meta_value(value);
    int64_t unix_time;
    rocksdb::Env::Default()->GetCurrentTime(&unix_time);
    int32_t cur_time = static_cast<int32_t>(unix_time);
    Trace("==================================================================");
    Trace("[MetaFilter], key: %s, count = %ld, timestamp: %d, cur_time: %d, version: %d",
        key.ToString().c_str(),
        static_cast<int64_t>(parsed_meta_value.count()),
        parsed_meta_value.timestamp(),
        cur_time, parsed_meta_value.version());
    if (parsed_meta_value.timestamp() != 0 &&
        parsed_meta_value.timestamp() < cur_time &&
        parsed_meta_value.version() < cur_time) {
      Trace("Drop[Stale & version < cur_time]");
      return true;
    }
    if (parsed_meta_value.count() == 0 &&
        parsed_meta_value.version() < cur_time) {
      Trace("Drop[Empty & version < cur_time]");
      return true;
    }
    Trace("Reserve");
    Trace("==================================================================");
    return false;
  }

  virtual const char* Name() const override { return "ListsMetaFilter"; }
};

class ListsMetaFilterFactory : public rocksdb::CompactionFilterFactory {
 public:
  ListsMetaFilterFactory() = default;
  virtual std::unique_ptr<rocksdb::CompactionFilter> CreateCompactionFilter(
      const rocksdb::CompactionFilter::Context& context) override {
    return std::unique_ptr<rocksdb::CompactionFilter>(
        new ListsMetaFilter());
  }
  virtual const char* Name() const override {
    return "ListsMetaFilterFactory";
  }
};

class ListsDataFilter : public rocksdb::CompactionFilter {
 public:
  ListsDataFilter(rocksdb::DB* db,
      std::vector<rocksdb::ColumnFamilyHandle*>* handles_ptr)
      : db_(db), cf_handles_ptr_(handles_ptr), meta_not_found_(false) {
  }
  virtual bool Filter(int level, const rocksdb::Slice& key,
                      const rocksdb::Slice& value,
                      std::string* new_value, bool* value_changed) const
      override {
    ParsedListsDataKey parsed_data_key(key);

    Trace("------------------------------------------------------------------");
    Trace("[DataFilter], key: %s, version: %d",
        parsed_data_key.key().ToString().c_str(),
        parsed_data_key.version());

    if (parsed_data_key.key().ToString() != cur_key_) {
      Trace("+++++++++++++++++++++++++++++++++++++++++++++++++++");
      Trace("!!!===>Prev Key: %s", cur_key_.c_str());
      cur_key_ = parsed_data_key.key().ToString();
      std::string meta_value;
      Status s = db_->Get(default_read_options_, (*cf_handles_ptr_)[0],
          cur_key_, &meta_value);
      if (s.ok()) {
        meta_not_found_ = false;
        ParsedListsMetaValue parsed_meta_value(&meta_value);
        cur_meta_version_ = parsed_meta_value.version();
        cur_meta_timestamp_ = parsed_meta_value.timestamp();
        Trace("Update cur_key to "
            "[%s, cur_meta_version: %d, cur_meta_timestamp: %d]",
            cur_key_.c_str(),
            cur_meta_version_, cur_meta_timestamp_);
        Trace("+++++++++++++++++++++++++++++++++++++++++++++++++++");
      } else if (s.IsNotFound()) {
        meta_not_found_ = true;
        Trace("Update cur_key to "
            "[%s, but meta_key not found]",
            cur_key_.c_str());
        Trace("+++++++++++++++++++++++++++++++++++++++++++++++++++");
      } else {
        Trace("Update cur_key to "
            "[%s, but Get meta_key FAILED] | Reserve",
            cur_key_.c_str());
        Trace("+++++++++++++++++++++++++++++++++++++++++++++++++++");
        return false;
      }
    }

    if (meta_not_found_) {
      Trace("Drop[Meta key not exist]");
      return true;
    }

    int64_t unix_time;
    rocksdb::Env::Default()->GetCurrentTime(&unix_time);
    if (cur_meta_timestamp_ != 0 &&
        cur_meta_timestamp_ < static_cast<int32_t>(unix_time)) {
      Trace("Drop[cur_meta_timestamp < cur_time(%d)]",
          static_cast<int32_t>(unix_time));
      return true;
    }
#ifndef NDEBUG
    if (cur_meta_version_ > parsed_data_key.version()) {
      Trace("Drop[value_version < cur_time_version]");
    } else {
      Trace("Reserve[value_version == cur_meta_version]");
    }
    Trace("------------------------------------------------------------------");
#endif
    return cur_meta_version_ > parsed_data_key.version();
  }

  virtual const char* Name() const override { return "ListsDataFilter"; }

 private:
  rocksdb::DB* db_;
  std::vector<rocksdb::ColumnFamilyHandle*>* cf_handles_ptr_;
  rocksdb::ReadOptions default_read_options_;
  mutable std::string cur_key_;
  mutable bool meta_not_found_;
  mutable int32_t cur_meta_version_;
  mutable int32_t cur_meta_timestamp_;
};

class ListsDataFilterFactory : public rocksdb::CompactionFilterFactory {
 public:
  ListsDataFilterFactory(rocksdb::DB** db_ptr,
      std::vector<rocksdb::ColumnFamilyHandle*>* handles_ptr)
    : db_ptr_(db_ptr), cf_handles_ptr_(handles_ptr) {
  }
  virtual std::unique_ptr<rocksdb::CompactionFilter> CreateCompactionFilter(
      const rocksdb::CompactionFilter::Context& context) override {
    return std::unique_ptr<rocksdb::CompactionFilter>(
        new ListsDataFilter(*db_ptr_, cf_handles_ptr_));
  }
  virtual const char* Name() const override {
    return "ListsDataFilterFactory";
  }

 private:
  rocksdb::DB** db_ptr_;
  std::vector<rocksdb::ColumnFamilyHandle*>* cf_handles_ptr_;
};

}  //  namespace blackwidow
#endif  // SRC_LISTS_FILTER_H_
