#pragma once

#include "func.hpp"

#include <rapidjson/document.h>

namespace gsc {
namespace func {
namespace script_value {
using json_allocator = rapidjson::Document::AllocatorType;
using task_result = std::vector<rapidjson::Document>;
using task_work = std::function<task_result()>;

rapidjson::Value read(scriptInstance_t inst, uint32_t index,
                      json_allocator &allocator);
void push(scriptInstance_t inst, const rapidjson::Value &value);

const rapidjson::Value *find_member(const rapidjson::Value &object,
                                    const char *name);
std::string to_string(const rapidjson::Value &value);

rapidjson::Document make_value(const std::string &value);
rapidjson::Document make_value(int64_t value);
rapidjson::Document make_null();
task_result error_result(const std::string &error);

void start_task(scriptInstance_t inst, task_work work);
} // namespace script_value
} // namespace func
} // namespace gsc
