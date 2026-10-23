#include <std_include.hpp>

#include "workshop.hpp"
#include <loader/component_loader.hpp>

#include <component/scheduler.hpp>

#include <utils/finally.hpp>
#include <utils/http.hpp>
#include <utils/image.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include <semaphore>

#include <stb_image_resize2.h>
#include <stb_image_write.h>

namespace workshop {
// The Steam API reports 0 for most items, but browse pages embed the size
std::unordered_map<std::string, uint64_t>
parse_page_sizes(const std::string_view &html) {
  std::unordered_map<std::string, uint64_t> sizes;
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
    const uint64_t bytes = std::strtoull(html.data() + digits, nullptr, 10);
    if (bytes && id_digits < id_at + 64 && id_end - id_digits >= 6)
      sizes[std::string(html.substr(id_digits, id_end - id_digits))] = bytes;
  }
  return sizes;
}

namespace {
constexpr int PAGE_SIZE = 30;
constexpr int MAX_PREVIEW_DOWNLOADS = 4;
constexpr int32_t PREVIEW_WIDTH = 512;
constexpr const char *USER_AGENT =
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";

std::mutex catalog_mutex;
std::vector<item_details> catalog_items;
catalog_status status{};
std::string catalog_query;
std::string catalog_kind;
std::atomic<uint64_t> catalog_request{0};
std::unordered_map<std::string, uint64_t> known_sizes;

std::mutex preview_mutex;
std::unordered_set<std::string> preview_busy;
std::unordered_set<std::string> preview_failed;
std::string cover_id;
std::counting_semaphore<> preview_slots{MAX_PREVIEW_DOWNLOADS};
// The selected item skips the page prefetch queue
std::counting_semaphore<> cover_slots{2};

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
  for (const auto &[id, bytes] : parse_page_sizes(html))
    known_sizes[id] = bytes;
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

std::vector<item_details> fetch_page(int page, const std::string_view &query,
                                     const std::string_view &kind) {
  utils::http::headers headers;
  headers["User-Agent"] = USER_AGENT;
  headers["Accept-Language"] = "en-US,en;q=0.9";

  const std::string base =
      std::format("https://steamcommunity.com/workshop/browse/"
                  "?appid={}&childpublishedfileid=0&numperpage=30&p={}{}",
                  game::APP_ID_STR, page,
                  kind.empty() ? std::string{}
                               : std::format("&requiredtags%5B%5D={}", kind));
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
  std::string url(html.substr(start, html.find('"', start) - start));
  // The page HTML-escapes the URL's '&'
  for (size_t pos = url.find("&amp;"); pos != std::string::npos;
       pos = url.find("&amp;", pos + 1))
    url.replace(pos, 5, "&");
  return url;
}

// Gfx_LoadTextureFromPng only reads PNG, and Steam serves JPEGs, so decode
// and re-encode. Written to a temp file first so the cover never loads half
// of one
bool save_preview(const std::string_view &id, const std::string &data) {
  try {
    const utils::image::image decoded = utils::image::load_image(data);
    const int32_t width = static_cast<int32_t>(decoded.width);
    const int32_t height = static_cast<int32_t>(decoded.height);
    const int32_t scaled_width = std::min(width, PREVIEW_WIDTH);
    const int32_t scaled_height = std::max(1, height * scaled_width / width);
    std::string scaled(static_cast<size_t>(scaled_width) * scaled_height * 4,
                       '\0');
    if (!stbir_resize_uint8_srgb(
            reinterpret_cast<const uint8_t *>(decoded.data.data()), width,
            height, 0, reinterpret_cast<uint8_t *>(scaled.data()), scaled_width,
            scaled_height, 0, STBIR_RGBA))
      return false;

    std::string png;
    const auto append = [](void *context, void *bytes, int size) {
      static_cast<std::string *>(context)->append(static_cast<char *>(bytes),
                                                  size);
    };
    if (!stbi_write_png_to_func(append, &png, scaled_width, scaled_height, 4,
                                scaled.data(), scaled_width * 4))
      return false;

    const std::filesystem::path path = preview_path(id);
    std::filesystem::path temp = path;
    temp += ".tmp";
    if (!utils::io::write_file(temp, png))
      return false;
    std::error_code ec;
    std::filesystem::rename(temp, path, ec);
    return !ec;
  } catch (const std::exception &) {
    return false;
  }
}

// Installed items ship their own preview, which is faster than Steam and
// works offline
std::optional<std::string> read_installed_preview(const std::string_view &id) {
  const std::filesystem::path folder = find_installed(id);
  if (folder.empty())
    return std::nullopt;
  std::error_code ec;
  for (const std::filesystem::path &dir : {folder, folder / "zone"}) {
    for (const std::filesystem::directory_entry &entry :
         std::filesystem::directory_iterator(dir, ec)) {
      const std::u8string stem8 = entry.path().stem().u8string();
      const std::u8string ext8 = entry.path().extension().u8string();
      const std::string stem =
          utils::string::to_lower(std::string(stem8.begin(), stem8.end()));
      const std::string ext =
          utils::string::to_lower(std::string(ext8.begin(), ext8.end()));
      if (stem.starts_with("previewimage") &&
          (ext == ".png" || ext == ".jpg" || ext == ".jpeg")) {
        std::string data;
        if (utils::io::read_file(entry.path(), &data))
          return data;
      }
    }
  }
  return std::nullopt;
}

bool download_preview(const std::string &id, const std::string &url) {
  utils::http::headers headers;
  headers["User-Agent"] = USER_AGENT;
  headers["Accept"] = "image/jpeg,image/png,image/gif,image/*;q=0.8";

  std::vector<std::string> urls;
  if (!url.empty()) {
    urls.push_back(std::format("{}{}imw=512&ima=fit&impolicy=Letterbox", url,
                               url.find('?') == std::string::npos ? "?" : "&"));
    urls.push_back(url);
  }
  urls.push_back(std::format(
      "https://steamcommunity.com/sharedfiles/filedetails/?id={}", id));

  for (size_t i = 0; i < urls.size(); ++i) {
    const std::optional<std::string> body =
        utils::http::get_data(urls[i], headers, {}, 10);
    if (!body)
      continue;
    if (is_engine_image(*body)) {
      if (save_preview(id, *body))
        return true;
      continue;
    }
    const std::string image = og_image(*body);
    if (!image.empty() && i + 1 == urls.size()) {
      urls.push_back(image + "?imw=512&ima=fit&impolicy=Letterbox");
      urls.push_back(image);
    }
  }
  return false;
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
  // Only touch the disk while waiting on a preview, not on every tick
  static std::string applied_id;
  static bool applied_final = true;
  std::string id;
  bool failed;
  {
    std::lock_guard lock(preview_mutex);
    id = cover_id;
    failed = preview_failed.contains(id);
  }
  if (id == applied_id && applied_final)
    return;

  const bool ready = !id.empty() && has_preview(id);
  const bool final = id.empty() || ready || failed;
  if (id == applied_id && !final)
    return;
  applied_id = id;
  applied_final = final;

  // No preview at all falls back to the default image
  if (ready) {
    bind_cover(
        game::gfx::Gfx_LoadTextureFromPng(preview_path(id).string().c_str()));
  } else if (final) {
    bind_cover({});
  } else {
    const std::filesystem::path loading =
        game::get_appdata_path() /
        "data/ui_scripts/workshop_menu/loadingworkshop.png";
    bind_cover(game::gfx::Gfx_LoadTextureFromPng(loading.string().c_str()));
  }
}
} // namespace

void request_catalog_page(const std::string_view &query, int page,
                          const std::string_view &kind) {
  page = std::max(page, 1);
  {
    std::lock_guard lock(catalog_mutex);
    if (catalog_query == query && catalog_kind == kind && status.page == page &&
        (status.loading || !catalog_items.empty() || !status.error.empty()))
      return;
    catalog_query = query;
    catalog_kind = kind;
    catalog_items.clear();
    status = {true, false, page, status.generation + 1, {}};
  }

  const uint64_t request = ++catalog_request;
  std::thread([query = std::string(query), kind = std::string(kind), page,
               request] {
    std::vector<item_details> items = fetch_page(page, query, kind);
    // Items tagged both Map and Mod would otherwise be hidden on one tab
    if (!kind.empty())
      for (item_details &item : items)
        item.kind = kind;
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
    if (preview_failed.contains(std::string(id)) ||
        !preview_busy.insert(std::string(id)).second)
      return;
  }

  const bool selected = [&] {
    std::lock_guard lock(preview_mutex);
    return cover_id == id;
  }();

  // Own copies: the caller's strings are gone once this returns
  std::thread([id = std::string(id), url = std::string(url), selected] {
    // Local files don't need a download slot
    const std::optional<std::string> local = read_installed_preview(id);
    bool saved = local && save_preview(id, *local);
    if (!saved) {
      std::counting_semaphore<> &slots = selected ? cover_slots : preview_slots;
      slots.acquire();
      const auto _ = utils::finally([&slots] { slots.release(); });
      saved = download_preview(id, url);
    }

    std::lock_guard lock(preview_mutex);
    preview_busy.erase(id);
    if (!saved)
      preview_failed.insert(id);
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
