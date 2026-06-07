#include <std_include.hpp>
#include "core.hpp"
#include "../../structs/structs.hpp"

#include <atomic>

namespace game {
namespace cg {
std::atomic<level::cl::cgPool *> cgArray_store = nullptr;
level::cl::cgPool *get_cgArray() {
  level::cl::cgPool *array = cgArray_store.load(std::memory_order_seq_cst);

#ifdef GAME_HPP
  if (array == nullptr) {
    array = *(cgArray.get());
  }
#endif

  return array;
}

std::atomic<level::cl::cgsPool *> cgsArray_store = nullptr;
level::cl::cgsPool *get_cgsArray() {
  level::cl::cgsPool *array = cgsArray_store.load(std::memory_order_seq_cst);

#ifdef GAME_HPP
  if (array == nullptr) {
    array = *(cgsArray.get());
  }
#endif

  return array;
}

std::atomic<anim::ViewModelInfo *> cg_viewModelArray_store = nullptr;
anim::ViewModelInfo *get_cg_viewModelArray() {
  anim::ViewModelInfo *array =
      cg_viewModelArray_store.load(std::memory_order_seq_cst);

#ifdef GAME_HPP
  if (array == nullptr) {
    array = *(cg_viewModelArray.get());
  }
#endif

  return array;
}

std::atomic<ClientPlayerAttachmentInfo *> cg_attachmentsArray_store = nullptr;
ClientPlayerAttachmentInfo *get_cg_attachmentsArray() {
  ClientPlayerAttachmentInfo *array =
      cg_attachmentsArray_store.load(std::memory_order_seq_cst);

#ifdef GAME_HPP
  if (array == nullptr) {
    array = *(cg_attachmentsArray.get());
  }
#endif

  return array;
}

level::cl::AtomicLocalClientCentityPools cg_entitiesArray_store = {nullptr,
                                                                   nullptr};

level::cl::LocalClientCentityPools get_cg_entitiesArray() {
  level::cl::LocalClientCentityPools result = {nullptr, nullptr};

  for (LocalClientNum_t idx = game::LOCAL_CLIENT_0;
       idx < game::LOCAL_CLIENT_COUNT; idx++) {
    result.pools[idx] =
        cg_entitiesArray_store[idx].load(std::memory_order_seq_cst);
#ifdef GAME_HPP
    if (result[idx] == nullptr) {
      result[idx] = cg_entitiesArray->pools[idx];
    }
#endif
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