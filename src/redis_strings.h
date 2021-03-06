//  Copyright (c) 2017-present The blackwidow Authors.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.

#ifndef SRC_REDIS_STRINGS_H_
#define SRC_REDIS_STRINGS_H_

#include <string>
#include <vector>
#include <algorithm>

#include "src/redis.h"
#include "blackwidow/blackwidow.h"

namespace blackwidow {

class RedisStrings : public Redis {
 public:
  RedisStrings() = default;
  ~RedisStrings() = default;

  // Strings Commands
  Status Set(const Slice& key, const Slice& value);
  Status Get(const Slice& key, std::string* value);
  Status GetSet(const Slice& key, const Slice& value, std::string* old_value);
  Status SetBit(const Slice& key, int64_t offset, int32_t value, int32_t* ret);
  Status GetBit(const Slice& key, int64_t offset, int32_t* ret);
  Status MSet(const std::vector<BlackWidow::KeyValue>& kvs);
  Status MGet(const std::vector<std::string>& keys,
              std::vector<std::string>* values);
  Status Setnx(const Slice& key, const Slice& value, int32_t* ret);
  Status MSetnx(const std::vector<BlackWidow::KeyValue>& kvs, int32_t* ret);
  Status Setrange(const Slice& key, int64_t start_offset,
                  const Slice& value, int32_t* ret);
  Status Getrange(const Slice& key, int64_t start_offset, int64_t end_offset,
                  std::string* ret);
  Status Append(const Slice& key, const Slice& value, int32_t* ret);
  Status BitCount(const Slice& key, int64_t start_offset, int64_t end_offset,
                  int32_t* ret, bool have_offset);
  Status BitOp(BlackWidow::BitOpType op, const std::string& dest_key,
               const std::vector<std::string>& src_keys, int64_t* ret);
  Status BitPos(const Slice& key, int32_t bit, int64_t* ret);
  Status BitPos(const Slice& key, int32_t bit,
                int64_t start_offset, int64_t* ret);
  Status BitPos(const Slice& key, int32_t bit,
                int64_t start_offset, int64_t end_offset,
                int64_t* ret);
  Status Decrby(const Slice& key, int64_t value, int64_t* ret);
  Status Incrby(const Slice& key, int64_t value, int64_t* ret);
  Status Incrbyfloat(const Slice& key, const Slice& value, std::string* ret);
  Status Setex(const Slice& key, const Slice& value, int32_t ttl);
  Status Strlen(const Slice& key, int32_t *len);

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
};

}  //  namespace blackwidow
#endif  //  SRC_REDIS_STRINGS_H_
