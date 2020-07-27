#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>



class Reference {
 public:
  explicit Reference(size_t object_id) :id_(object_id) { }
  size_t id() const { return id_; }
 private:
  size_t id_;
};

struct RefInfo {
  std::unordered_set<size_t> adjs;
  size_t ref_count = 1;
  size_t in_degree = 0;
  bool orphan() const {
    return ref_count == 0 && in_degree == 0;
  }
};

class GC {
 public:
  GC() :max_object_id_(0) { }
  Reference allocate(size_t size) {
    auto obj_id = max_object_id_++;
    objects.emplace(obj_id, RefInfo());
    return Reference(obj_id);
  }
  void increase_rc(Reference ref) {
    auto it = objects.find(ref.id());
    if (it == objects.end()) {
      throw std::runtime_error("increase_rc(): Object id not found " + std::to_string(ref.id()));
    } else {
      it->second.ref_count++;
    }
  }

  void decrease_rc(Reference ref) {
    auto it = objects.find(ref.id());
    if (it == objects.end()) {
      throw std::runtime_error("decrease_rc(): Object id not found " + std::to_string(ref.id()));
    } else {
      it->second.ref_count--;
    }
  }

  void refer(Reference referer, Reference referee) {
    auto it = objects.find(referer.id());
    auto it_referee = objects.find(referee.id());
    if (it == objects.end()) {
      throw std::runtime_error("refer(): referer object id not found " + std::to_string(referer.id()));
    } else if (it_referee == objects.end()) {
      throw std::runtime_error("refer(): referee object id not found " + std::to_string(referee.id()));
    } else {
      it->second.adjs.insert(referee.id());
      it_referee->second.in_degree++;
    }
  }

  void gc() {
    std::vector<size_t> orphan_nodes;
    for (auto &[object_id, ref_info] : objects) {
      if (ref_info.orphan()) {
        orphan_nodes.push_back(object_id);
      }
    }

    while (!orphan_nodes.empty()) {
      auto orphan_id = orphan_nodes.back();
      orphan_nodes.pop_back();

      erase(orphan_id, orphan_nodes);
      std::cout << "erasing " << orphan_id << std::endl;
    }
  }
 private:
  void erase(size_t id, std::vector<size_t> &orphan_nodes) {
    for (auto adj_id : objects.at(id).adjs) {
      auto &adj = objects.at(adj_id);
      adj.in_degree--;
      if (adj.orphan()) {
        orphan_nodes.push_back(adj_id);
      }
    }
    objects.erase(id);
  }
 private:
  size_t max_object_id_;
  std::unordered_map<size_t, RefInfo> objects;
};

int main() {
  GC gc;

  auto ref0 = gc.allocate(16);
  auto ref1 = gc.allocate(32);
  gc.gc();
  gc.refer(ref0, ref1);
  gc.decrease_rc(ref1);
  std::cout << "GC1" << std::endl;
  gc.gc();
  gc.decrease_rc(ref0);
  std::cout << "GC2" << std::endl;
  gc.gc();
}
