#include <std_include.hpp>

#include "utils.hpp"
#include <utils/concurrency.hpp>

using namespace game::com;
using namespace game::sv;

namespace game {

namespace {
utils::concurrency::container<std::vector<std::string>>
    registered_dvar_names_{};
std::atomic_size_t registered_dvar_name_count_{0};
} // namespace

void record_registered_dvar_name(const char *dvar_name) {
  if (!dvar_name || !dvar_name[0]) {
    return;
  }
  registered_dvar_names_.access([dvar_name](std::vector<std::string> &names) {
    names.emplace_back(dvar_name);
  });
  registered_dvar_name_count_.fetch_add(1, std::memory_order_seq_cst);
} // namespace

template <typename T>
static void foreach_client(
    T *client_states,
    const std::function<void(sv::client_s &cl, ClientNum_t index)> &callback) {
  if (client_states && callback) {
    for (ClientNum_t i = CLIENT_INDEX_FIRST; i < get_max_client_count(); ++i) {
      callback(client_states[i], i);
    }
  }
}

template <typename T>
static void first_client(
    T *client_states,
    const std::function<bool(sv::client_s &cl, ClientNum_t index)> &callback) {
  if (client_states && callback) {
    for (ClientNum_t i = CLIENT_INDEX_FIRST;
         i < get_max_client_count() && !callback(client_states[i], i); ++i) {
    }
  }
}

template <typename T>
static bool
access_client(T *client_states, const ClientNum_t index,
              const std::function<void(sv::client_s &cl)> &callback) {
  if (client_states && callback && index < get_max_client_count()) {
    T &client = client_states[index];
    if (client.state != net::clientState_t::FREE) {
      callback(client);
      return true;
    }
  }
  return false;
}

void foreach_client(
    const std::function<void(sv::client_s &cl, ClientNum_t index)> &callback) {
  if (is_server()) {
    foreach_client(*svs_clients, callback);
  } else {
    foreach_client(*svs_clients_cl, callback);
  }
}

void first_client(
    const std::function<bool(sv::client_s &cl, ClientNum_t index)> &callback) {
  if (is_server()) {
    first_client(*svs_clients, callback);
  } else {
    first_client(*svs_clients_cl, callback);
  }
}

void foreach_client(const std::function<void(sv::client_s &cl)> &callback) {
  foreach_client(
      [&callback](sv::client_s &client, ClientNum_t) { callback(client); });
}
void first_client(const std::function<bool(sv::client_s &cl)> &callback) {
  first_client([&callback](sv::client_s &client, ClientNum_t) {
    return callback(client);
  });
}

void foreach_connected_client(
    const std::function<void(sv::client_s &cl, ClientNum_t index)> &callback) {
  foreach_client([&](sv::client_s &client, const ClientNum_t index) {
    if (client.state != net::clientState_t::FREE) {
      callback(client, index);
    }
  });
}

void foreach_connected_client(
    const std::function<void(sv::client_s &cl)> &callback) {
  foreach_connected_client(
      [&callback](sv::client_s &client, ClientNum_t) { callback(client); });
}

void first_connected_client(
    const std::function<bool(sv::client_s &cl, ClientNum_t index)> &callback) {
  first_client([&callback](sv::client_s &client, const ClientNum_t index) {
    return client.state != net::clientState_t::FREE && callback(client, index);
  });
}

void first_connected_client(
    const std::function<bool(sv::client_s &cl)> &callback) {
  first_connected_client([&callback](sv::client_s &client, ClientNum_t) {
    return callback(client);
  });
}

bool access_connected_client(
    const ClientNum_t index,
    const std::function<void(sv::client_s &cl)> &callback) {
  return is_server() ? access_client(*svs_clients, index, callback)
                     : access_client(*svs_clients_cl, index, callback);
}

void foreach_test_client(
    const std::function<void(sv::client_s &cl, ClientNum_t index)> &callback) {
  foreach_client([&](sv::client_s &client, const ClientNum_t index) {
    if (client.bIsTestClient) {
      callback(client, index);
    }
  });
}

void foreach_test_client(
    const std::function<void(sv::client_s &cl)> &callback) {
  foreach_test_client(
      [&callback](sv::client_s &client, ClientNum_t) { callback(client); });
}

void first_test_client(
    const std::function<bool(sv::client_s &cl, ClientNum_t index)> &callback) {
  first_client([&callback](sv::client_s &client, const ClientNum_t index) {
    if (client.bIsTestClient) {
      return callback(client, index);
    }
    return false;
  });
}

void first_test_client(const std::function<bool(sv::client_s &cl)> &callback) {
  first_test_client([&callback](sv::client_s &client, ClientNum_t) {
    return callback(client);
  });
}

void foreach_active_client(
    const std::function<void(sv::client_s &cl, ClientNum_t index)> &callback) {
  foreach_client([&callback](sv::client_s &client, const ClientNum_t index) {
    if (client.state > net::clientState_t::CONNECTED) {
      callback(client, index);
    }
  });
}

void foreach_active_client(
    const std::function<void(sv::client_s &cl)> &callback) {
  foreach_active_client(
      [&callback](sv::client_s &client, ClientNum_t) { callback(client); });
}

void first_active_client(
    const std::function<bool(sv::client_s &cl, ClientNum_t index)> &callback) {
  first_client([&callback](sv::client_s &client, const ClientNum_t index) {
    return client.state > net::clientState_t::CONNECTED &&
           callback(client, index);
  });
}

void first_active_client(
    const std::function<bool(sv::client_s &cl)> &callback) {
  first_active_client([&callback](sv::client_s &client, ClientNum_t) {
    return callback(client);
  });
}

bool access_active_client(
    const ClientNum_t index,
    const std::function<void(sv::client_s &cl)> &callback) {
  return is_server() ? access_client(*svs_clients, index, callback)
                     : access_client(*svs_clients_cl, index, callback);
}

const std::vector<std::string> &get_registered_dvar_names() {
  static const std::vector<std::string> result = []() {
    std::vector<std::string> result;
    registered_dvar_names_.access(
        [&result](const std::vector<std::string> &names) { result = names; });
    return result;
  }();
  return result;
}

size_t get_registered_dvar_name_count() {
  return registered_dvar_name_count_.load();
}

} // namespace game
