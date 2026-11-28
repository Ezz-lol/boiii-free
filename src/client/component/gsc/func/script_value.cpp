#include <std_include.hpp>

#include "script_value.hpp"

#include <component/game_event.hpp>
#include <component/scheduler.hpp>
#include <component/script.hpp>
#include <game/impl/game/game.hpp>
#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include <utils/string.hpp>

#include <condition_variable>
#include <deque>
#include <thread>

namespace gsc {
namespace func {
namespace script_value {
namespace {
constexpr uint32_t MAX_DEPTH = 16;
constexpr size_t WORKER_COUNT = 4;
constexpr size_t MAX_PENDING_TASKS = 64;

volatile ScrVar_t *variable(const scriptInstance_t inst,
                            const ScrVarIndex_t id) {
  if (!valid_scrvar_index(inst, id)) {
    return nullptr;
  }
  return &vm::gScrVarGlob->instance[+inst].scriptVariables[id];
}

std::string hash_key(const uint32_t hash) {
  std::string name = script::resolve_hash(hash);
  if (!name.empty()) {
    return name;
  }
  return utils::string::va("#%08x", hash);
}

rapidjson::Value read_value(scriptInstance_t inst,
                            volatile ScrVarValue_t *value,
                            json_allocator &allocator, uint32_t depth);

rapidjson::Value read_object(const scriptInstance_t inst,
                             const ScrVarIndex_t id, json_allocator &allocator,
                             const uint32_t depth) {
  volatile ScrVar_t *object = variable(inst, id);
  if (!object || depth > MAX_DEPTH) {
    return rapidjson::Value{};
  }

  const ScrVarType type = object->value.type;
  if (type == ScrVarType::ENTITY) {
    return rapidjson::Value("[entity]", allocator);
  }
  if (type != ScrVarType::ARRAY && type != ScrVarType::STRUCT) {
    return rapidjson::Value(var_typename->get(type), allocator);
  }

  std::vector<std::pair<int64_t, ScrVarIndex_t>> indexed;
  std::vector<std::pair<std::string, ScrVarIndex_t>> named;
  for (ScrVarIndex_t child_id = object->value.u.firstChild; child_id != 0;) {
    volatile ScrVar_t *child = variable(inst, child_id);
    if (!child) {
      break;
    }
    const ScrVarNameType_t name_type = child->nameType;
    const uint32_t name = static_cast<uint32_t>(child->nameIndex);
    if (type == ScrVarType::ARRAY && name_type == ScrVarNameType_t::INTEGER) {
      indexed.emplace_back(static_cast<int32_t>(name), child_id);
    } else if (name_type == ScrVarNameType_t::CONST_STRING) {
      const char *key = sl::SL_ConvertToString(name);
      named.emplace_back(key ? key : "", child_id);
    } else if (name_type != ScrVarNameType_t::OBJECT &&
               name_type != ScrVarNameType_t::WEAPON) {
      named.emplace_back(hash_key(name), child_id);
    }
    child_id = child->nextSibling;
  }

  if (type == ScrVarType::ARRAY && named.empty()) {
    std::ranges::sort(indexed);
    rapidjson::Value result(rapidjson::kArrayType);
    for (const auto &[index, child_id] : indexed) {
      result.PushBack(read_value(inst, &variable(inst, child_id)->value,
                                 allocator, depth + 1),
                      allocator);
    }
    return result;
  }

  rapidjson::Value result(rapidjson::kObjectType);
  for (const auto &[index, child_id] : indexed) {
    named.emplace_back(std::to_string(index), child_id);
  }
  for (const auto &[key, child_id] : named) {
    result.AddMember(rapidjson::Value(key.c_str(), allocator),
                     read_value(inst, &variable(inst, child_id)->value,
                                allocator, depth + 1),
                     allocator);
  }
  return result;
}

rapidjson::Value read_value(const scriptInstance_t inst,
                            volatile ScrVarValue_t *value,
                            json_allocator &allocator, const uint32_t depth) {
  switch (value->type) {
  case ScrVarType::UNDEFINED:
    return rapidjson::Value{};
  case ScrVarType::INT:
    return rapidjson::Value(value->u.intValue);
  case ScrVarType::FLOAT:
    return rapidjson::Value(static_cast<double>(value->u.floatValue));
  case ScrVarType::STRING:
  case ScrVarType::LOCALIZED_STRING: {
    const char *str = sl::SL_ConvertToString(value->u.stringValue);
    return rapidjson::Value(str ? str : "", allocator);
  }
  case ScrVarType::VECTOR: {
    rapidjson::Value result(rapidjson::kArrayType);
    if (value->u.vectorValue) {
      for (int i = 0; i < 3; ++i) {
        result.PushBack(static_cast<double>(value->u.vectorValue[i]),
                        allocator);
      }
    }
    return result;
  }
  case ScrVarType::HASH:
    return rapidjson::Value(hash_key(value->u.uintValue).c_str(), allocator);
  case ScrVarType::UINT64:
    return rapidjson::Value(std::to_string(value->u.uint64Value).c_str(),
                            allocator);
  case ScrVarType::FUNCTION:
  case ScrVarType::API_FUNCTION:
    return rapidjson::Value("[function]", allocator);
  case ScrVarType::POINTER:
    return read_object(inst, value->u.pointerValue, allocator, depth);
  default:
    return rapidjson::Value(var_typename->get(value->type), allocator);
  }
}

struct task {
  scriptInstance_t inst;
  ScrVarIndex_t object;
  uint32_t generation;
  task_work work;
};

std::mutex queue_mutex;
std::condition_variable queue_cv;
std::deque<task> queue;
std::atomic_uint32_t generation{};
std::atomic_size_t pending{};
std::once_flag workers_started;

void deliver(const task &entry, const task_result &result) {
  pending--;
  if (entry.generation != generation || !game::server_running()) {
    return;
  }
  for (auto it = result.rbegin(); it != result.rend(); ++it) {
    push(entry.inst, *it);
  }
  Scr_NotifyId(entry.inst, INVALID_LOCAL_CLIENT, entry.object,
               game::CanonHash("done"), static_cast<uint32_t>(result.size()));
  ScrVar_ReleaseVariable(entry.inst, entry.object);
}

void schedule_delivery(task entry, task_result result) {
  scheduler::once([entry = std::move(entry),
                   result = std::make_shared<task_result>(
                       std::move(result))] { deliver(entry, *result); },
                  scheduler::pipeline::server);
}

void worker() {
  while (true) {
    task entry;
    {
      std::unique_lock lock(queue_mutex);
      queue_cv.wait(lock, [] { return !queue.empty(); });
      entry = std::move(queue.front());
      queue.pop_front();
    }
    task_result result;
    try {
      result = entry.work();
    } catch (const std::exception &e) {
      result = error_result(e.what());
    }
    schedule_delivery(std::move(entry), std::move(result));
  }
}
} // namespace

rapidjson::Value read(const scriptInstance_t inst, const uint32_t index,
                      json_allocator &allocator) {
  if (index >= Scr_GetNumParam(inst)) {
    return rapidjson::Value{};
  }
  if (Scr_GetType(inst, index) == ScrVarType::POINTER) {
    return read_object(inst, Scr_GetObject(inst, index), allocator, 0);
  }
  return read_value(inst, Scr_GetValue(inst, index), allocator, 0);
}

namespace {
void push_value(const scriptInstance_t inst, const rapidjson::Value &value,
                const uint32_t depth) {
  if (depth > MAX_DEPTH) {
    Scr_AddUndefined(inst);
    return;
  }
  switch (value.GetType()) {
  case rapidjson::kNullType:
    Scr_AddUndefined(inst);
    break;
  case rapidjson::kFalseType:
  case rapidjson::kTrueType:
    Scr_AddInt(inst, value.GetBool() ? 1 : 0);
    break;
  case rapidjson::kNumberType:
    if (value.IsInt()) {
      Scr_AddInt(inst, value.GetInt());
    } else if (value.IsInt64() || value.IsUint64()) {
      Scr_AddString(inst, to_string(value).c_str());
    } else {
      Scr_AddFloat(inst, static_cast<float>(value.GetDouble()));
    }
    break;
  case rapidjson::kStringType:
    Scr_AddString(inst, value.GetString());
    break;
  case rapidjson::kArrayType:
    Scr_MakeArray(inst);
    for (const rapidjson::Value &element : value.GetArray()) {
      push_value(inst, element, depth + 1);
      Scr_AddArray(inst);
    }
    break;
  case rapidjson::kObjectType: {
    Scr_MakeArray(inst);
    std::unordered_set<std::string_view> seen;
    for (const auto &member : value.GetObject()) {
      const std::string_view key(member.name.GetString(),
                                 member.name.GetStringLength());
      if (member.value.IsNull() || !seen.insert(key).second) {
        continue;
      }
      push_value(inst, member.value, depth + 1);
      Scr_AddArrayStringIndexed(
          inst, sl::SL_GetString(member.name.GetString(), 0, 0x17));
    }
    break;
  }
  }
}

} // namespace

void push(const scriptInstance_t inst, const rapidjson::Value &value) {
  push_value(inst, value, 0);
}

const rapidjson::Value *find_member(const rapidjson::Value &object,
                                    const char *name) {
  if (!object.IsObject()) {
    return nullptr;
  }
  const auto found = object.FindMember(name);
  if (found != object.MemberEnd()) {
    return &found->value;
  }
  const std::string hashed = utils::string::va(
      "#%08x", static_cast<uint32_t>(game::scr::builtin::fnv1a(name)));
  const auto found_hash = object.FindMember(hashed.c_str());
  return found_hash != object.MemberEnd() ? &found_hash->value : nullptr;
}

std::string to_string(const rapidjson::Value &value) {
  if (value.IsString()) {
    return std::string(value.GetString(), value.GetStringLength());
  }
  if (value.IsInt64()) {
    return std::to_string(value.GetInt64());
  }
  if (value.IsUint64()) {
    return std::to_string(value.GetUint64());
  }
  if (value.IsNumber()) {
    return utils::string::va("%g", value.GetDouble());
  }
  if (value.IsBool()) {
    return value.GetBool() ? "1" : "0";
  }
  return {};
}

rapidjson::Document make_value(const std::string &value) {
  rapidjson::Document document;
  document.SetString(value.c_str(), static_cast<uint32_t>(value.size()),
                     document.GetAllocator());
  return document;
}

rapidjson::Document make_value(const int64_t value) {
  rapidjson::Document document;
  document.SetInt64(value);
  return document;
}

rapidjson::Document make_null() { return rapidjson::Document{}; }

task_result error_result(const std::string &error) {
  task_result result;
  result.emplace_back(make_null());
  result.emplace_back(make_value(int64_t{0}));
  result.emplace_back(make_value(error));
  return result;
}

void start_task(const scriptInstance_t inst, task_work work) {
  const ScrVarIndex_t object = Scr_AddStruct(inst);
  volatile ScrVar_t *handle = variable(inst, object);
  if (!handle) {
    return;
  }
  handle->refCount = handle->refCount + 1;
  pending++;

  if (pending > MAX_PENDING_TASKS) {
    task entry{
        .inst = inst, .object = object, .generation = generation, .work = {}};
    task_result result = error_result("too many pending requests");
    schedule_delivery(std::move(entry), std::move(result));
    return;
  }

  std::call_once(workers_started, [] {
    for (size_t i = 0; i < WORKER_COUNT; ++i) {
      std::thread(worker).detach();
    }
  });

  {
    std::lock_guard lock(queue_mutex);
    queue.push_back(task{.inst = inst,
                         .object = object,
                         .generation = generation,
                         .work = std::move(work)});
  }
  queue_cv.notify_one();
}

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/script_value");

  void post_unpack() override {
    game_event::on_g_shutdown_game([] { generation++; });
  }
};
} // namespace script_value
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::script_value::component)
