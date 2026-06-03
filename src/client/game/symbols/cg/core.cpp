#include "core.hpp"
#include "../../structs/structs.hpp"

#include <atomic>

namespace game {
namespace cg {
std::atomic<level::cl::cg_t *> cgArray_store = nullptr;
level::cl::cg_t *get_cgArray() {
  level::cl::cg_t *array = cgArray_store.load(std::memory_order_seq_cst);

#ifdef GAME_HPP
  if (array == nullptr) {
    array = *(cgArray.get());
  }
#endif

  return array;
}

std::atomic<level::cl::cgs_t *> cgsArray_store = nullptr;
level::cl::cgs_t *get_cgsArray() {
  level::cl::cgs_t *array = cgsArray_store.load(std::memory_order_seq_cst);

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
} // namespace cg
} // namespace game