#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <iomanip>
#include <map>
#include <set>
#include <cassert>
#include <random>
#include <functional>

template <typename Key, typename Value, class Hash = std::hash<Key>>
class CuckooMap {
 private:
  std::pair<Key, Value>* memory_;
  bool* stored_;
  size_t* visited_;
  size_t capacity_;
  size_t size_;
  Hash Hash1_;

  struct Hash2 {
    size_t operator()(size_t x) {
      x = (x ^ (x >> 30)) * size_t(0xbf58476d1ce4e5b9);
      x = (x ^ (x >> 27)) * size_t(0x94d049bb133111eb);
      x = x ^ (x >> 31);
      x &= 0b1010101010101010101010101010101010101010101010101010101010101010;
      return x;
    }
  };

  bool put(size_t pos, Key insert_key, Value insert_value) {
    if (!stored_[pos]) {
      stored_[pos] = 1;
      memory_[pos].first = insert_key;
      memory_[pos].second = insert_value;
      return 1;
    }
    return 0;
  }

  std::pair<Key, Value>* extract_memory() {
    std::pair<Key, Value>* ret = memory_;
    memory_ = nullptr;
    return ret;
  }

  bool* extract_stored() {
    bool* ret = stored_;
    stored_ = nullptr;
    return ret;
  }

  size_t* extract_visited() {
    size_t* ret = visited_;
    visited_ = nullptr;
    return ret;
  }

  void rehash(Key new_key, Value new_value) {
    CuckooMap<Key, Value, Hash> new_map(capacity_ * 3, Hash1_);
    new_map.insert(new_key, new_value);

    for (auto c : (*this)) {
      new_map.insert(c);
    }

    delete[] memory_;
    memory_ = new_map.extract_memory();
    delete[] stored_;
    stored_ = new_map.extract_stored();
    delete[] visited_;
    visited_ = new_map.extract_visited();
    capacity_ = new_map.capacity_;
  }

 public:
  class const_iterator {
   private:
    size_t pointer_;
    bool* stored_;
    size_t capacity_;
    std::pair<const Key, Value>* memory_;

   public:
    std::pair<const Key, Value>* GetMemory() const { return memory_; }
    size_t GetPointer() const { return pointer_; }
    const_iterator() {}
    const_iterator(size_t pointer, bool* stored, size_t capacity,
                   std::pair<const Key, Value>* memory)
        : pointer_(pointer),
          stored_(stored),
          capacity_(capacity),
          memory_(memory) {}

    const_iterator& operator=(const_iterator rht) {
      pointer_ = rht.pointer_;
      stored_ = rht.stored_;
      capacity_ = rht.capacity_;
      memory_ = rht.memory_;
      return (*this);
    }

    const std::pair<const Key, Value>& operator*() const {
      return (memory_[pointer_]);
    }

    const std::pair<const Key, Value>* operator->() const {
      return (&memory_[pointer_]);
    }

    const_iterator& operator++() {
      ++pointer_;
      while (pointer_ < capacity_ && !stored_[pointer_]) {
        ++pointer_;
      }

      return *this;
    }

    const_iterator operator++(int) {
      const_iterator ans = *this;
      ++(*this);
      return ans;
    }

    bool operator==(const const_iterator& second_value) const {
      return std::make_pair(pointer_, memory_) ==
             std::make_pair(second_value.GetPointer(),
                            second_value.GetMemory());
    }

    bool operator!=(const const_iterator& second_value) const {
      return !((*this) == second_value);
    }
  };

  class iterator {
   private:
    size_t pointer_;
    bool* stored_;
    size_t capacity_;
    std::pair<const Key, Value>* memory_;

   public:
    std::pair<const Key, Value>* GetMemory() const { return memory_; }
    size_t GetPointer() const { return pointer_; }
    iterator() {}
    iterator(size_t pointer, bool* stored, size_t capacity,
             std::pair<const Key, Value>* memory)
        : pointer_(pointer),
          stored_(stored),
          capacity_(capacity),
          memory_(memory) {}

    iterator& operator=(iterator rht) {
      pointer_ = rht.pointer_;
      stored_ = rht.stored_;
      capacity_ = rht.capacity_;
      memory_ = rht.memory_;
      return (*this);
    }

    std::pair<const Key, Value>& operator*() { return (memory_[pointer_]); }

    std::pair<const Key, Value>* operator->() { return (&memory_[pointer_]); }

    iterator& operator++() {
      ++pointer_;
      while (pointer_ < capacity_ && !stored_[pointer_]) {
        ++pointer_;
      }

      return *this;
    }

    iterator operator++(int) {
      iterator ans = *this;
      ++(*this);
      return ans;
    }

    operator const_iterator() {
      const_iterator iter(pointer_, stored_, capacity_, memory_);
      return iter;
    }
    bool operator==(const iterator& second_value) const {
      return std::make_pair(pointer_, memory_) ==
             std::make_pair(second_value.GetPointer(),
                            second_value.GetMemory());
    }

    bool operator!=(const iterator& second_value) const {
      return !((*this) == second_value);
    }
  };

  CuckooMap(size_t capacity, Hash hasher = Hash{})
      : memory_(new std::pair<Key, Value>[capacity]),
        stored_(new bool[capacity]),
        visited_(new size_t[capacity]),
        capacity_(capacity),
        size_(0),
        Hash1_(hasher) {
    std::fill(visited_, visited_ + capacity_, 0);
    std::fill(stored_, stored_ + capacity_, false);
  }

  CuckooMap(Hash hasher = Hash{}) : CuckooMap(1, hasher) {}

  template <typename Iter>
  CuckooMap(Iter fst, Iter nd, Hash hasher = Hash{})
      : CuckooMap(1, hasher) {
    while (fst != nd) {
      insert(*fst);
    }
  }

  CuckooMap(std::initializer_list<std::pair<Key, Value>> input_list,
            Hash hasher = Hash{})
      : CuckooMap(1, hasher) {
    for (auto c : input_list) {
      insert(c);
    }
  }

  size_t capacity() const { return capacity_; }

  void clear() {
    size_ = 0;
    delete[] memory_;
    delete[] visited_;
    delete[] stored_;
    capacity_ = 2;
    memory_ = (new std::pair<Key, Value>[capacity_]);
    stored_ = new bool[capacity_];
    visited_ = new size_t[capacity_];

    std::fill(visited_, visited_ + capacity_, 0);
    std::fill(stored_, stored_ + capacity_, false);
  }

  size_t size() const { return size_; }

  bool empty() const { return size_ == 0; }

  Hash hash_function() const { return Hash1_; }

  const_iterator end() const {
    return {capacity_, stored_, capacity_,
            reinterpret_cast<std::pair<const Key, Value>*>(memory_)};
  }

  iterator end() {
    return {capacity_, stored_, capacity_,
            reinterpret_cast<std::pair<const Key, Value>*>(memory_)};
  }

  const_iterator begin() const {
    if (size_ == 0) {
      return end();
    }

    size_t bucket = 0;
    while (bucket < capacity_ && !stored_[bucket]) {
      ++bucket;
    }

    return {bucket, stored_, capacity_,
            reinterpret_cast<std::pair<const Key, Value>*>(memory_)};
  }

  iterator begin() {
    if (size_ == 0) {
      return end();
    }

    size_t bucket = 0;
    while (bucket < capacity_ && !stored_[bucket]) {
      ++bucket;
    }

    return {bucket, stored_, capacity_,
            reinterpret_cast<std::pair<const Key, Value>*>(memory_)};
  }

  const_iterator find(Key key) const {
    size_t first_hash = Hash1_(key) % capacity_;
    size_t second_hash = Hash2()(Hash1_(key)) % capacity_;

    if (stored_[first_hash] == true && memory_[first_hash].first == key) {
      return {first_hash, stored_, capacity_,
              reinterpret_cast<std::pair<const Key, Value>*>(memory_)};
    }
    if (stored_[second_hash] == true && memory_[second_hash].first == key) {
      return {second_hash, stored_, capacity_,
              reinterpret_cast<std::pair<const Key, Value>*>(memory_)};
    }

    return end();
  }

  iterator find(Key key) {
    size_t first_hash = Hash1_(key) % capacity_;
    size_t second_hash = Hash2()(Hash1_(key)) % capacity_;

    if (stored_[first_hash] == true && memory_[first_hash].first == key) {
      return {first_hash, stored_, capacity_,
              reinterpret_cast<std::pair<const Key, Value>*>(memory_)};
    }
    if (stored_[second_hash] == true && memory_[second_hash].first == key) {
      return {second_hash, stored_, capacity_,
              reinterpret_cast<std::pair<const Key, Value>*>(memory_)};
    }

    return end();
  }

  bool find_bad_collision(Key key) {
    size_t first_hash = Hash1_(key) % capacity_;
    size_t second_hash = Hash2()(Hash1_(key)) % capacity_;

    if (stored_[first_hash] == true && !(memory_[first_hash].first == key) &&
        Hash1_(memory_[first_hash].first) == Hash1_(key)) {
      return true;
    }
    if (stored_[second_hash] == true && !(memory_[second_hash].first == key) &&
        Hash1_(memory_[second_hash].first) == Hash1_(key)) {
      return true;
    }

    return false;
  }

  Value& operator[](Key key) {
    if (find(key) == end()) {
      std::pair<Key, Value> k;
      k.first = key;
      insert(k.first, k.second);
    }

    auto it = find(key);
    return std::ref(memory_[it.GetPointer()].second);
  }

  const Value& at(Key key) const {
    if (find(key) == end()) {
      throw std::out_of_range("invalid index");
    }

    auto it = find(key);
    return std::ref(memory_[it.GetPointer()].second);
  }

  bool erase(Key key) {
    auto it = find(key);
    if (it != end()) {
      stored_[it.GetPointer()] = false;
      --size_;
      return true;
    }
    return false;
  }

  bool insert(Key insert_key, Value insert_value) {
    if (find(insert_key) != end()) {
      return 0;
    }

    ++size_;
    size_t first_hash = Hash1_(insert_key) % capacity_;
    size_t second_hash = Hash2()(Hash1_(insert_key)) % capacity_;

    if (put(first_hash, insert_key, insert_value)) {
      return true;
    }
    if (put(second_hash, insert_key, insert_value)) {
      return true;
    }

    size_t part = (first_hash ^ second_hash) % 2;
    size_t prev;
    if (part) {
      prev = first_hash;
    } else {
      prev = second_hash;
    }

    while (true) {
      first_hash = Hash1_(insert_key) % capacity_;
      second_hash = Hash2()(Hash1_(insert_key)) % capacity_;

      if (first_hash == prev) {
        std::swap(first_hash, second_hash);
      }
      if (visited_[first_hash]) {
        rehash(insert_key, insert_value);
        return true;
      }
      visited_[first_hash] = prev + 1;

      bool flag = stored_[first_hash];
      Key new_key = memory_[first_hash].first;
      Value new_value = memory_[first_hash].second;

      stored_[first_hash] = false;
      put(first_hash, insert_key, insert_value);
      insert_key = new_key;
      insert_value = new_value;
      prev = first_hash;
      if (!flag) break;
    }

    while (visited_[prev] != 0) {
      size_t p = visited_[prev];
      visited_[prev] = 0;
      prev = p - 1;
    }

    return 1;
  }

  bool insert(std::pair<Key, Value> pr) { return insert(pr.first, pr.second); }

  ~CuckooMap() {
    delete[] memory_;
    delete[] visited_;
    delete[] stored_;
  }
};

template <typename Key, typename Value, class Hash = std::hash<Key>>
class HashMap {
 private:
  std::vector<CuckooMap<Key, Value, Hash>*> maps_;
  size_t size_ = 0;
  Hash Hash1_;

 public:
  HashMap(Hash hasher = Hash{}) : Hash1_(hasher) {
    maps_.emplace_back(new CuckooMap<Key, Value, Hash>(2, Hash1_));
  }

  template <typename Iter>
  HashMap(Iter fst, Iter nd, Hash hasher = Hash{})
      : HashMap(hasher) {
    while (fst != nd) {
      insert(fst->first, fst->second);
      ++fst;
    }
  }

  HashMap(std::initializer_list<std::pair<Key, Value>> inPut_list,
          Hash hasher = Hash{})
      : HashMap(hasher) {
    for (auto c : inPut_list) {
      insert(c);
    }
  }

  HashMap(const HashMap& cpy) : HashMap(cpy.Hash1_) {
    for (const auto& c : cpy) insert(c);
  }

  HashMap& operator=(const HashMap& cpy) {
    HashMap nw(cpy);
    clear();
    for (const auto& c : nw) insert(c);
    return (*this);
  }

  void clear() {
    size_ = 0;
    for (auto& c : maps_) {
      c->clear();
    }
  }
  size_t size() const {
    size_t size_sum = 0;
    for (auto c : maps_) size_sum += c->size();
    assert(size_sum == size_);
    return size_;
  }

  bool empty() const { return size_ == 0; }

  Hash hash_function() const { return Hash1_; }

  bool insert(Key insert_key, Value insert_value) {
    if (find(insert_key) != end()) return 0;
    ++size_;

    size_t last_map = 0, inserted = 0;

    while (!inserted) {
      if (maps_.size() == last_map) {
        maps_.emplace_back(new CuckooMap<Key, Value, Hash>(2, Hash1_));
      }
      if (maps_[last_map]->find_bad_collision(insert_key)) {
        ++last_map;
      } else {
        maps_[last_map]->insert(insert_key, insert_value);
        inserted = 1;
      }
    }

    return 1;
  }

  bool insert(std::pair<Key, Value> pr) { return insert(pr.first, pr.second); }

  void erase(Key key) {
    for (auto& c : maps_) {
      if (c->erase(key)) --size_;
    }
  }

  class const_iterator {
   private:
    size_t map_pointer_;
    size_t maps_size_;
    typename CuckooMap<Key, Value, Hash>::const_iterator map_iterator_;
    const std::vector<CuckooMap<Key, Value, Hash>*>* maps_;

   public:
    const_iterator() {}
    const_iterator(
        size_t map_pointer, size_t maps_size,
        typename CuckooMap<Key, Value, Hash>::const_iterator map_iterator,
        const std::vector<CuckooMap<Key, Value, Hash>*>* maps)
        : map_pointer_(map_pointer),
          maps_size_(maps_size),
          map_iterator_(map_iterator),
          maps_(maps) {}
    size_t get_pointer() const { return map_pointer_; }
    const_iterator& operator=(const_iterator rht) {
      map_pointer_ = rht.map_pointer_;
      maps_size_ = rht.maps_size_;
      map_iterator_ = rht.map_iterator_;
      maps_ = rht.maps_;
      return (*this);
    }

    const std::pair<const Key, Value>& operator*() const {
      return (*map_iterator_);
    }

    const typename CuckooMap<Key, Value, Hash>::const_iterator& operator->()
        const {
      return (map_iterator_);
    }

    const_iterator& operator++() {
      ++map_iterator_;

      while (map_pointer_ + 1 < maps_size_ &&
             map_iterator_ == (*maps_)[map_pointer_]->end()) {
        ++map_pointer_;
        map_iterator_ = (*maps_)[map_pointer_]->begin();
      }

      return *this;
    }

    const_iterator operator++(int) {
      const_iterator ans = *this;
      ++(*this);
      return ans;
    }

    bool operator==(const const_iterator& second_value) const {
      return std::make_pair(map_pointer_, map_iterator_) ==
             std::make_pair(second_value.map_pointer_,
                            second_value.map_iterator_);
    }

    bool operator!=(const const_iterator& second_value) const {
      return !((*this) == second_value);
    }
  };

  class iterator {
   private:
    size_t map_pointer_;
    size_t maps_size_;
    typename CuckooMap<Key, Value, Hash>::iterator map_iterator_;
    std::vector<CuckooMap<Key, Value, Hash>*>* maps_;

   public:
    iterator() {}
    iterator(size_t map_pointer, size_t maps_size,
             typename CuckooMap<Key, Value, Hash>::iterator map_iterator,
             std::vector<CuckooMap<Key, Value, Hash>*>* maps)
        : map_pointer_(map_pointer),
          maps_size_(maps_size),
          map_iterator_(map_iterator),
          maps_(maps) {}

    size_t get_pointer() const { return map_pointer_; }
    iterator& operator=(iterator rht) {
      map_pointer_ = rht.map_pointer_;
      maps_size_ = rht.maps_size_;
      map_iterator_ = rht.map_iterator_;
      maps_ = rht.maps_;
      return (*this);
    }

    typename CuckooMap<Key, Value, Hash>::iterator& operator->() {
      return (map_iterator_);
    }

    std::pair<const Key, Value>& operator*() { return (*map_iterator_); }

    iterator& operator++() {
      ++map_iterator_;

      while (map_pointer_ + 1 < maps_size_ &&
             map_iterator_ == (*maps_)[map_pointer_]->end()) {
        ++map_pointer_;
        map_iterator_ = (*maps_)[map_pointer_]->begin();
      }

      return *this;
    }

    iterator operator++(int) {
      iterator ans = *this;
      ++(*this);
      return ans;
    }

    bool operator==(const iterator& second_value) const {
      return std::make_pair(map_pointer_, map_iterator_) ==
             std::make_pair(second_value.map_pointer_,
                            second_value.map_iterator_);
    }

    bool operator!=(const iterator& second_value) const {
      return !((*this) == second_value);
    }

    operator const_iterator() {
      const_iterator iter(map_pointer_, maps_size_, map_iterator_, maps_);
      return iter;
    }
  };

  const_iterator end() const {
    return {maps_.size() - 1, maps_.size(), maps_.back()->end(), &maps_};
  }

  iterator end() {
    return {maps_.size() - 1, maps_.size(), maps_.back()->end(), &maps_};
  }

  const_iterator begin() const {
    size_t bucket = 0;

    while (bucket + 1 < maps_.size()) {
      if (maps_[bucket]->begin() == maps_[bucket]->end()) {
        ++bucket;
      } else {
        break;
      }
    }

    return {bucket, maps_.size(), maps_[bucket]->begin(), &maps_};
  }

  iterator begin() {
    size_t bucket = 0;

    while (bucket + 1 < maps_.size()) {
      if (maps_[bucket]->begin() == maps_[bucket]->end()) {
        ++bucket;
      } else {
        break;
      }
    }

    return {bucket, maps_.size(), maps_[bucket]->begin(), &maps_};
  }

  iterator find(Key key) {
    for (size_t iter = 0; iter < maps_.size(); ++iter) {
      if (maps_[iter]->find(key) != maps_[iter]->end()) {
        return {iter, maps_.size(), maps_[iter]->find(key), &maps_};
      }
    }

    return end();
  }

  const_iterator find(Key key) const {
    for (size_t iter = 0; iter < maps_.size(); ++iter) {
      if (maps_[iter]->find(key) != maps_[iter]->end()) {
        return {iter, maps_.size(), maps_[iter]->find(key), &maps_};
      }
    }

    return end();
  }

  Value& operator[](Key key) {
    if (find(key) == end()) {
      std::pair<Key, Value> k;
      k.first = key;
      insert(k.first, k.second);
    }
    auto it = find(key);
    return std::ref((*maps_[it.get_pointer()])[key]);
  }

  const Value& at(Key key) const {
    if (find(key) == end()) {
      throw std::out_of_range("invalid index");
    }

    auto it = find(key);
    return std::ref(maps_[it.get_pointer()]->at(key));
  }

  ~HashMap() {
    for (auto& c : maps_) {
      delete c;
    }
  }
};
