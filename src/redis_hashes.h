//  Copyright (c) 2017-present The blackwidow Authors.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.

#ifndef SRC_REDIS_HASHES_H_
#define SRC_REDIS_HASHES_H_

#include <string>
#include <vector>
#include <unordered_set>

#include "src/redis.h"
#include "blackwidow/blackwidow.h"

namespace blackwidow {

class RedisHashes : public Redis {
 public:
  RedisHashes() = default;
  ~RedisHashes();

  // Hashes Commands
  Status HSet(const Slice& key, const Slice& field, const Slice& value,
              int32_t* ret);
  Status HGet(const Slice& key, const Slice& field, std::string* value);
  Status HMSet(const Slice& key,
               const std::vector<BlackWidow::FieldValue>& fvs);
  Status HMGet(const Slice& key, const std::vector<std::string>& fields,
               std::vector<std::string>* values);
  Status HGetall(const Slice& key,
                 std::vector<BlackWidow::FieldValue>* fvs);
  Status HKeys(const Slice& key,
               std::vector<std::string>* fields);
  Status HVals(const Slice& key,
               std::vector<std::string>* values);
  Status HSetnx(const Slice& key, const Slice& field, const Slice& value,
                int32_t* ret);
  Status HLen(const Slice& key, int32_t* ret);
  Status HStrlen(const Slice& key, const Slice& field, int32_t* len);
  Status HExists(const Slice& key, const Slice& field);
  Status HIncrby(const Slice& key, const Slice& field, int64_t value,
                 int64_t* ret);
  Status HIncrbyfloat(const Slice& key, const Slice& field,
                      const Slice& by, std::string* new_value);
  Status HDel(const Slice& key, const std::vector<std::string>& fields,
              int32_t* ret);

  // Common Commands
  virtual Status Open(const rocksdb::Options& options,
      const std::string& db_path) override;
  virtual Status CompactRange(const rocksdb::Slice* begin,
      const rocksdb::Slice* end) override;

  // Keys Commands
  virtual Status Expire(const Slice& key, int32_t ttl) override;
  virtual Status Del(const Slice& key) override;
  virtual bool Scan(const std::string& start_key, const std::string& pattern,
                    std::vector<std::string>* keys,
                    int64_t* count, std::string* next_key) override;
  virtual Status Expireat(const Slice& key, int32_t timestamp) override;
  virtual Status Persist(const Slice& key) override;
  virtual Status TTL(const Slice& key, int64_t* timestamp) override;

 private:
  std::vector<rocksdb::ColumnFamilyHandle*> handles_;
};

}  //  namespace blackwidow
#endif  //  SRC_REDIS_HASHES_H_
