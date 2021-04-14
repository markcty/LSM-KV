//
// Created by markcty on 2021/4/14.
//

#ifndef LSM_KV__KVSTORE_H_
#define LSM_KV__KVSTORE_H_

#include <string>
#include "kvstore_api.h"

class KVStore : public KVStoreAPI {
 private:

 public:
  explicit KVStore(const std::string &dir);

  ~KVStore();

  void put(uint64_t key, const std::string &s) override;

  std::string get(uint64_t key) override;

  bool del(uint64_t key) override;

  void reset() override;

};

#endif //LSM_KV__KVSTORE_H_
