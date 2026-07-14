#include <std_include.hpp>
#include "../../game.hpp"
#include "core.hpp"

#include <atomic>

namespace game {
namespace cg {
std::atomic<level::cl::cgPool *> cgArray_store = nullptr;
level::cl::cgPool *get_cgArray() {
  if (is_server()) {
    return *cgArray;
  }

  level::cl::cgPool *array = cgArray_store.load(std::memory_order_seq_cst);

  if (array == nullptr) {
    array = *(cgArray.get());
  }

  return array;
}

std::atomic<level::cl::cgsPool *> cgsArray_store = nullptr;
level::cl::cgsPool *get_cgsArray() {
  if (is_server()) {
    return *cgsArray;
  }

  level::cl::cgsPool *array = cgsArray_store.load(std::memory_order_seq_cst);

  if (array == nullptr) {
    array = *(cgsArray.get());
  }

  return array;
}

std::atomic<anim::ViewModelInfo *> cg_viewModelArray_store = nullptr;
anim::ViewModelInfo *get_cg_viewModelArray() {
  anim::ViewModelInfo *array =
      cg_viewModelArray_store.load(std::memory_order_seq_cst);

  if (array == nullptr) {
    array = *(cg_viewModelArray.get());
  }

  return array;
}

std::atomic<ClientPlayerAttachmentInfo *> cg_attachmentsArray_store = nullptr;
ClientPlayerAttachmentInfo *get_cg_attachmentsArray() {
  if (is_server()) {
    return *cg_attachmentsArray;
  }

  ClientPlayerAttachmentInfo *array =
      cg_attachmentsArray_store.load(std::memory_order_seq_cst);

  if (array == nullptr) {
    array = *(cg_attachmentsArray.get());
  }

  return array;
}

level::cl::AtomicLocalClientCentityPools cg_entitiesArray_store = {nullptr,
                                                                   nullptr};
level::cl::LocalClientCentityPools get_cg_entitiesArray() {
  if (is_server()) {
    return *cg_entitiesArray;
  }

  level::cl::LocalClientCentityPools result = {nullptr, nullptr};

  for (LocalClientNum_t idx = game::LOCAL_CLIENT_0;
       idx < game::LOCAL_CLIENT_COUNT; idx++) {
    result.pools[idx] =
        cg_entitiesArray_store[idx].load(std::memory_order_seq_cst);
    if (result.pools[idx] == nullptr) {
      result.pools[idx] = cg_entitiesArray->pools[idx];
    }
  }

  return result;
}

void clear_cgEntitiesArray_store() {
  for (LocalClientNum_t idx = game::LOCAL_CLIENT_0;
       idx < game::LOCAL_CLIENT_COUNT; idx++) {
    cg_entitiesArray_store[idx].store(nullptr, std::memory_order_seq_cst);
  }
}
} // namespace cg
} // namespace game