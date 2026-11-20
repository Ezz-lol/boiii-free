#include <std_include.hpp>

#include "http.hpp"
#include "script_value.hpp"

#include <loader/component_loader.hpp>

#include <utils/flags.hpp>
#include <utils/http.hpp>
#include <utils/string.hpp>

namespace gsc {
namespace func {
namespace http {
namespace {
utils::http::headers read_headers(const rapidjson::Value *value) {
  utils::http::headers headers;
  if (value && value->IsObject()) {
    for (const auto &member : value->GetObject()) {
      headers[member.name.GetString()] = script_value::to_string(member.value);
    }
  }
  return headers;
}

constexpr size_t MAX_RESPONSE_SIZE = 1024 * 1024;

void start_request(scriptInstance_t inst, std::string url,
                   utils::http::request_options options) {
  options.max_size = MAX_RESPONSE_SIZE;
  if (!network_allowed()) {
    script_value::start_task(inst, [] {
      return script_value::error_result(
          "network access from GSC is disabled, launch with "
          "-unsafe-gsc-network to allow it");
    });
    return;
  }

  script_value::start_task(inst, [url = std::move(url),
                                  options = std::move(options)] {
    const utils::http::response response = utils::http::request(url, options);
    script_value::task_result result;
    result.emplace_back(script_value::make_value(response.body));
    result.emplace_back(
        script_value::make_value(static_cast<int64_t>(response.status)));
    result.emplace_back(response.error.empty()
                            ? script_value::make_null()
                            : script_value::make_value(response.error));
    return result;
  });
}

std::string url_arg(scriptInstance_t inst) {
  const char *url = Scr_GetString(inst, 0);
  return url ? url : "";
}

void gscr_httpget(scriptInstance_t inst) {
  rapidjson::Document document;
  const rapidjson::Value headers =
      script_value::read(inst, 1, document.GetAllocator());
  utils::http::request_options options;
  options.headers = read_headers(&headers);
  start_request(inst, url_arg(inst), std::move(options));
}

void gscr_httppost(scriptInstance_t inst) {
  rapidjson::Document document;
  const rapidjson::Value body =
      script_value::read(inst, 1, document.GetAllocator());
  const rapidjson::Value headers =
      script_value::read(inst, 2, document.GetAllocator());
  utils::http::request_options options;
  options.method = "POST";
  options.body = script_value::to_string(body);
  options.headers = read_headers(&headers);
  start_request(inst, url_arg(inst), std::move(options));
}

void gscr_httprequest(scriptInstance_t inst) {
  rapidjson::Document document;
  const rapidjson::Value settings =
      script_value::read(inst, 1, document.GetAllocator());
  utils::http::request_options options;
  if (const rapidjson::Value *method =
          script_value::find_member(settings, "method")) {
    options.method = utils::string::to_upper(script_value::to_string(*method));
  }
  if (const rapidjson::Value *body =
          script_value::find_member(settings, "body")) {
    options.body = script_value::to_string(*body);
  }
  if (const rapidjson::Value *timeout =
          script_value::find_member(settings, "timeout");
      timeout && timeout->IsInt() && timeout->GetInt() > 0) {
    options.timeout_sec =
        static_cast<uint32_t>(std::min(timeout->GetInt(), 120));
  }
  options.headers =
      read_headers(script_value::find_member(settings, "headers"));
  start_request(inst, url_arg(inst), std::move(options));
}
} // namespace

bool network_allowed() {
  return game::is_server() || utils::flags::has_flag("unsafe-gsc-network");
}

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/http");

  void post_unpack() override {
    register_builtin(SCRIPTINSTANCE_SERVER, "httpget", gscr_httpget, 1, 2);
    register_builtin(SCRIPTINSTANCE_SERVER, "httppost", gscr_httppost, 2, 3);
    register_builtin(SCRIPTINSTANCE_SERVER, "httprequest", gscr_httprequest, 1,
                     2);
  }
};
} // namespace http
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::http::component)
