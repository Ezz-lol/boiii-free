#include <std_include.hpp>

#include "workshop.hpp"
#include <loader/component_loader.hpp>

#include <component/scheduler.hpp>

#include <utils/http.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

namespace workshop {
namespace {
constexpr int PAGE_SIZE = 30;
constexpr int MAX_PREVIEW_DOWNLOADS = 4;
constexpr const char *USER_AGENT =
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";

std::mutex catalog_mutex;
std::vector<item_details> catalog_items;
catalog_status status{};
std::string catalog_query;
std::atomic<uint64_t> catalog_request{0};
std::unordered_map<std::string, uint64_t> known_sizes;

std::mutex preview_mutex;
std::unordered_set<std::string> preview_busy;
std::string cover_id;
std::string applied_cover;
std::atomic<int> preview_downloads{0};

std::string url_encode(const std::string_view &value) {
  std::string out;
  for (const uint8_t c : value) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      out.push_back(static_cast<char>(c));
    } else {
      out += utils::string::va("%%%02X", c);
    }
  }
  return out;
}

void collect_ids(const std::string_view &html, const std::string_view &prefix,
                 std::vector<std::string> &ids,
                 std::unordered_set<std::string> &seen) {
  for (size_t pos = html.find(prefix);
       pos != std::string::npos && ids.size() < PAGE_SIZE;
       pos = html.find(prefix, pos)) {
    pos += prefix.size();
    std::string id;
    while (pos < html.size() && std::isdigit(static_cast<uint8_t>(html[pos])))
      id.push_back(html[pos++]);
    if (id.size() >= 8 && seen.insert(id).second)
      ids.push_back(id);
  }
}

void remember_page_sizes(const std::string_view &html) {
  const std::string key = "file_size";
  for (size_t pos = html.find(key); pos != std::string::npos;
       pos = html.find(key, pos + key.size())) {
    const size_t digits = html.find_first_of("0123456789", pos + key.size());
    const size_t id_at = html.rfind("publishedfileid", pos);
    if (digits == std::string::npos || digits > pos + 40 ||
        id_at == std::string::npos || pos - id_at > 4000)
      continue;
    const size_t id_digits = html.find_first_of("0123456789", id_at + 16);
    const size_t id_end = html.find_first_not_of("0123456789", id_digits);
    const size_t bytes = std::strtoull(html.data() + digits, nullptr, 10);
    if (bytes && id_digits < id_at + 64 && id_end - id_digits >= 6)
      known_sizes[std::string(html.substr(id_digits, id_end - id_digits))] =
          bytes;
  }
}

bool matches_query(const item_details &item, const std::string_view &query) {
  const std::string hay =
      utils::string::to_lower(item.title + " " + item.description);
  int needed = 0;
  int hits = 0;
  std::string term;
  for (const char c : utils::string::to_lower(std::string(query)) + " ") {
    if (std::isalnum(static_cast<uint8_t>(c))) {
      term.push_back(c);
      continue;
    }
    if (term.size() >= 2) {
      ++needed;
      const bool gta = (term == "grand" || term == "theft" || term == "auto") &&
                       hay.find("gta") != std::string::npos;
      hits += hay.find(term) != std::string::npos || gta;
    }
    term.clear();
  }
  return needed <= 1 ? hits >= needed : hits >= std::max(2, (needed + 1) / 2);
}

std::string strip_tags(const std::string_view &text) {
  std::string plain;
  bool in_tag = false;
  for (const char c : text) {
    if (c == '<')
      in_tag = true;
    else if (c == '>')
      in_tag = false;
    else if (!in_tag)
      plain.push_back(c);
  }
  return plain;
}

std::vector<item_details> fetch_page(int page, const std::string_view &query) {
  utils::http::headers headers;
  headers["User-Agent"] = USER_AGENT;
  headers["Accept-Language"] = "en-US,en;q=0.9";

  const std::string base =
      std::format("https://steamcommunity.com/workshop/browse/"
                  "?appid={}&childpublishedfileid=0&numperpage=30&p={}",
                  game::APP_ID_STR, page);
  std::vector<std::string> urls;
  if (query.empty()) {
    urls.push_back(std::format(
        "{}&section=readytouseitems&browsesort=trend&actualsort=trend", base));
  } else {
    const std::string search =
        std::format("&browsesort=textsearch&actualsort=textsearch&"
                    "searchtext={}",
                    url_encode(query));
    urls.push_back(std::format("{}&section=readytouseitems{}", base, search));
    urls.push_back(std::format("{}{}", base, search));
  }

  for (const std::string &url : urls) {
    const std::optional<std::string> html =
        utils::http::get_data(url, headers, {}, 15);
    if (!html)
      continue;

    std::vector<std::string> ids;
    std::unordered_set<std::string> seen;
    for (const char *prefix :
         {"filedetails/?id=", "sharedfile_", "PublishedFileId\":\""}) {
      collect_ids(*html, prefix, ids, seen);
    }
    if (ids.empty())
      continue;

    std::vector<item_details> items;
    {
      std::lock_guard lock(catalog_mutex);
      remember_page_sizes(*html);
    }
    for (auto &item : get_details(ids)) {
      if (!query.empty() && !matches_query(item, query))
        continue;
      item.description = strip_tags(item.description);
      std::lock_guard lock(catalog_mutex);
      if (item.file_size)
        known_sizes[item.id] = item.file_size;
      else if (known_sizes.contains(item.id))
        item.file_size = known_sizes[item.id];
      items.push_back(std::move(item));
    }
    if (!items.empty())
      return items;
  }
  return {};
}

std::filesystem::path preview_path(const std::string_view &id) {
  return game::get_appdata_path() / "workshop_previews" /
         std::format("{}.png", id);
}

bool has_preview(const std::string_view &id) {
  return utils::io::file_size(preview_path(id).string()) > 16;
}

bool is_engine_image(const std::string_view &body) {
  const uint8_t *u = reinterpret_cast<const uint8_t *>(body.data());
  return body.size() > 16 &&
         ((u[0] == 0x89 && u[1] == 'P' && u[2] == 'N' && u[3] == 'G') ||
          (u[0] == 0xFF && u[1] == 0xD8 && u[2] == 0xFF) ||
          body.starts_with("GIF8") || body.starts_with("BM"));
}

std::string og_image(const std::string_view &html) {
  const size_t key = html.find("og:image");
  const size_t content = key == std::string::npos
                             ? std::string::npos
                             : html.find("content=\"", key > 80 ? key - 80 : 0);
  if (content == std::string::npos)
    return {};
  const size_t start = content + 9;
  return std::string(html.substr(start, html.find('"', start) - start));
}

void bind_cover(const game::gfx::GfxTexture &loaded) {
  game::gfx::GfxImage *image =
      game::db::xasset::DB_FindXAssetHeader(game::db::xasset::XAssetType::IMAGE,
                                            "img_t7_mod_preview", 1, -1)
          .image;
  if (!image)
    return;
  if (image->texture.basemap) {
    game::gfx::Gfx_TexturePool_ReleaseRef(image->texture, 0);
    image->texture.basemap->lpVtbl->Release(image->texture.basemap);
  }
  game::gfx::GfxTexture texture = loaded;
  if (!texture.basemap) {
    texture = (*game::gfx::loadedGfxImage.get())->texture;
    game::gfx::Gfx_TexturePool_AddRef(texture, 0);
    texture.basemap->lpVtbl->AddRef(texture.basemap);
  }
  image->texture = texture;
}

void apply_cover() {
  std::string id;
  {
    std::lock_guard lock(preview_mutex);
    id = cover_id;
  }
  const std::filesystem::path loading =
      game::get_appdata_path() /
      "data/ui_scripts/workshop_menu/loadingworkshop.png";
  const std::string wanted = id.empty()        ? std::string{}
                             : has_preview(id) ? preview_path(id).string()
                                               : loading.string();
  if (wanted == applied_cover)
    return;
  applied_cover = wanted;
  bind_cover(wanted.empty()
                 ? game::gfx::GfxTexture{}
                 : game::gfx::Gfx_LoadTextureFromPng(wanted.c_str()));
}
} // namespace

void request_catalog_page(const std::string_view &query, int page) {
  page = std::max(page, 1);
  {
    std::lock_guard lock(catalog_mutex);
    if (catalog_query == query && status.page == page &&
        (status.loading || !catalog_items.empty() || !status.error.empty()))
      return;
    catalog_query = query;
    catalog_items.clear();
    status = {true, false, page, status.generation + 1, {}};
  }

  const uint64_t request = ++catalog_request;
  std::thread([query, page, request] {
    std::vector<item_details> items = fetch_page(page, query);
    for (const item_details &item : items)
      request_preview(item.id, item.preview_url);

    std::lock_guard lock(catalog_mutex);
    if (request != catalog_request)
      return;
    status.has_more = items.size() >= PAGE_SIZE;
    status.loading = false;
    status.error = items.empty() ? (query.empty() ? "No items on this page."
                                                  : "No search results.")
                                 : "";
    ++status.generation;
    catalog_items = std::move(items);
  }).detach();
}

std::vector<item_details> get_catalog_page() {
  std::lock_guard lock(catalog_mutex);
  return catalog_items;
}

catalog_status get_catalog_status() {
  std::lock_guard lock(catalog_mutex);
  return status;
}

void request_preview(const std::string_view &id, const std::string_view &url) {
  if (id.empty() || has_preview(id))
    return;
  {
    std::lock_guard lock(preview_mutex);
    if (!preview_busy.insert(std::string(id)).second)
      return;
  }

  std::thread([id, url] {
    while (preview_downloads >= MAX_PREVIEW_DOWNLOADS)
      std::this_thread::sleep_for(80ms);
    ++preview_downloads;

    utils::http::headers headers;
    headers["User-Agent"] = USER_AGENT;
    headers["Accept"] = "image/jpeg,image/png,image/gif,image/*;q=0.8";

    std::vector<std::string> urls;
    if (!url.empty()) {
      urls.push_back(std::string(url));
      urls.push_back(
          std::format("{}{}imw=512&ima=fit&impolicy=Letterbox", url,
                      url.find('?') == std::string::npos ? "?" : "&"));
    }
    urls.push_back(std::format(
        "https://steamcommunity.com/sharedfiles/filedetails/?id={}", id));

    for (size_t i = 0; i < urls.size(); ++i) {
      const std::optional<std::string> body =
          utils::http::get_data(urls[i], headers, {}, 10);
      if (!body)
        continue;
      if (is_engine_image(*body)) {
        utils::io::write_file(preview_path(id).string(), *body);
        break;
      }
      const std::string image = og_image(*body);
      if (!image.empty() && i + 1 == urls.size()) {
        urls.push_back(image);
        urls.push_back(image + "?imw=512&ima=fit&impolicy=Letterbox");
      }
    }

    --preview_downloads;
    std::lock_guard lock(preview_mutex);
    preview_busy.erase(std::string(id));
  }).detach();
}

void set_preview_cover(const std::string_view &id) {
  std::lock_guard lock(preview_mutex);
  cover_id = id;
}

class catalog final : public client_component {
  DEFINE_COMPONENT_NAME("workshop_catalog");

public:
  void post_unpack() override {
    scheduler::loop(apply_cover, scheduler::main, 50ms);
  }
};
} // namespace workshop

REGISTER_COMPONENT(workshop::catalog)
