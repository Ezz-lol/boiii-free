#pragma once

#include <std_include.hpp>

class html_frame;

namespace launcher::workshop {
CComVariant utf8_variant(const std::string &utf8_str);
void register_callbacks(html_frame *frame);
void try_refresh_workshop_content();

std::map<std::string, uint64_t>
batch_get_time_updated(const std::vector<std::string> &ids);

struct workshop_item_meta {
  uint64_t time_updated = 0;
  uint64_t file_size = 0;
  std::string description;
  std::string preview_url;
  int32_t star_rating = 0;
  int64_t subs = 0;
  int64_t favorites = 0;
};

std::map<std::string, workshop_item_meta>
batch_get_workshop_meta(const std::vector<std::string> &ids);
} // namespace launcher::workshop