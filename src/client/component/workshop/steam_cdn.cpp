#include <std_include.hpp>

#include "workshop.hpp"

#include <utils/cryptography.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>

#include <curl/curl.h>
#include <rapidjson/document.h>
#include <tomcrypt.h>
#include <winhttp.h>
#include <winioctl.h>
#include <zlib.h>
#include <zstd.h>

#pragma comment(lib, "winhttp.lib")

#define _7ZIP_ST
extern "C" {
#include "lzma/LzmaDec.c"
#include "lzma/LzmaDec.h"
}

namespace workshop {
namespace {
constexpr uint32_t PROTO_MASK = 0x80000000u;
constexpr uint32_t EMSG_MULTI = 1;
constexpr uint32_t EMSG_HEARTBEAT = 703;
constexpr uint32_t EMSG_LOGON_RESPONSE = 751;
constexpr uint32_t EMSG_SERVICE_METHOD_CALL = 151;
constexpr uint32_t EMSG_SERVICE_METHOD_RESPONSE = 147;
constexpr uint32_t EMSG_DEPOT_KEY = 5438;
constexpr uint32_t EMSG_DEPOT_KEY_RESPONSE = 5439;
constexpr uint32_t EMSG_LOGON = 5514;
constexpr uint32_t EMSG_HELLO = 9805;
constexpr uint32_t PROTOCOL = 65581;
constexpr uint64_t JOB_NONE = 0xFFFFFFFFFFFFFFFFULL;
constexpr uint64_t ANON_STEAMID = 0x01A0000000000000ULL;
constexpr uint32_t EOS_WIN10 = 16;
constexpr uint32_t MANIFEST_PAYLOAD_MAGIC = 0x71F617D0u;
constexpr uint32_t MANIFEST_METADATA_MAGIC = 0x1F4812BEu;
constexpr uint32_t MANIFEST_SIGNATURE_MAGIC = 0x1B81B817u;
constexpr uint32_t MANIFEST_END_MAGIC = 0x32C415ABu;
constexpr uint32_t DIR_FLAG = 64;
constexpr unsigned MAX_WORKERS = 8;
constexpr int CHUNK_ATTEMPTS = 10;

struct context {
  const download_job &job;
  cdn_progress &progress;

  bool cancelled() const { return job.cancel && job.cancel->load(); }

  void wait_while_paused() const {
    while (job.pause && job.pause->load() && !cancelled()) {
      std::this_thread::sleep_for(200ms);
    }
  }
};

uint32_t rd_u32(const uint8_t *p) {
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) |
         (static_cast<uint32_t>(p[3]) << 24);
}

uint64_t rd_u64(const uint8_t *p) {
  return static_cast<uint64_t>(rd_u32(p)) |
         (static_cast<uint64_t>(rd_u32(p + 4)) << 32);
}

bool is_zip(const uint8_t *b, size_t n) {
  return n >= 4 && b[0] == 'P' && b[1] == 'K' && b[2] == 0x03 && b[3] == 0x04;
}

void wr_u32(std::string &o, uint32_t v) {
  for (int i = 0; i < 4; ++i)
    o.push_back(static_cast<char>(v >> (i * 8)));
}

void pb_varint(std::string &o, uint64_t v) {
  while (v >= 0x80) {
    o.push_back(static_cast<char>((v & 0x7F) | 0x80));
    v >>= 7;
  }
  o.push_back(static_cast<char>(v));
}

void pb_tag(std::string &o, uint32_t field, uint32_t wt) {
  pb_varint(o, (static_cast<uint64_t>(field) << 3) | wt);
}

void pb_uint(std::string &o, uint32_t field, uint64_t v) {
  pb_tag(o, field, 0);
  pb_varint(o, v);
}

void pb_fixed64(std::string &o, uint32_t field, uint64_t v) {
  pb_tag(o, field, 1);
  wr_u32(o, static_cast<uint32_t>(v));
  wr_u32(o, static_cast<uint32_t>(v >> 32));
}

void pb_bytes(std::string &o, uint32_t field, const std::string &s) {
  pb_tag(o, field, 2);
  pb_varint(o, s.size());
  o.append(s);
}

struct pb_in {
  const uint8_t *p{};
  const uint8_t *end{};

  bool read_var(uint64_t &out) {
    out = 0;
    for (int shift = 0; p < end && shift <= 63; shift += 7) {
      const uint8_t b = *p++;
      out |= static_cast<uint64_t>(b & 0x7F) << shift;
      if ((b & 0x80) == 0)
        return true;
    }
    return false;
  }

  bool read_fixed64(uint64_t &out) {
    if (end - p < 8)
      return false;
    out = rd_u64(p);
    p += 8;
    return true;
  }

  bool get_len(pb_in &inner) {
    uint64_t n = 0;
    if (!read_var(n) || static_cast<uint64_t>(end - p) < n)
      return false;
    inner = {p, p + n};
    p = inner.end;
    return true;
  }

  std::string read_bytes() {
    pb_in inner;
    return get_len(inner) ? inner.str() : std::string{};
  }

  std::string str() const {
    return {reinterpret_cast<const char *>(p), static_cast<size_t>(end - p)};
  }

  bool skip(uint32_t wt) {
    uint64_t dummy = 0;
    pb_in inner;
    switch (wt) {
    case 0:
      return read_var(dummy);
    case 1:
      return read_fixed64(dummy);
    case 2:
      return get_len(inner);
    case 5:
      if (end - p < 4)
        return false;
      p += 4;
      return true;
    default:
      return false;
    }
  }

  template <typename F> void fields(F &&handle) {
    uint64_t tag = 0;
    while (p < end && read_var(tag)) {
      const auto field = static_cast<uint32_t>(tag >> 3);
      const auto wt = static_cast<uint32_t>(tag & 7);
      if (!handle(field, wt) && !skip(wt))
        return;
    }
  }

  uint64_t var() {
    uint64_t v = 0;
    read_var(v);
    return v;
  }
};

pb_in as_in(const std::string &s) {
  const auto *p = reinterpret_cast<const uint8_t *>(s.data());
  return {p, p + s.size()};
}

std::string inflate_window(const uint8_t *data, size_t n, int window_bits,
                           size_t expect) {
  z_stream s{};
  if (inflateInit2(&s, window_bits) != Z_OK)
    return {};
  s.next_in = const_cast<Bytef *>(data);
  s.avail_in = static_cast<uInt>(n);
  std::string out(expect ? expect : n * 4 + 256, '\0');
  int rc = Z_OK;
  while (rc != Z_STREAM_END) {
    if (s.total_out >= out.size())
      out.resize(out.size() * 2 + 256);
    s.next_out = reinterpret_cast<Bytef *>(out.data() + s.total_out);
    s.avail_out = static_cast<uInt>(out.size() - s.total_out);
    rc = inflate(&s, Z_NO_FLUSH);
    if (rc != Z_OK && rc != Z_STREAM_END) {
      inflateEnd(&s);
      return {};
    }
  }
  out.resize(s.total_out);
  inflateEnd(&s);
  return out;
}

std::string unzip_first_entry(const std::string &blob) {
  const auto *b = reinterpret_cast<const uint8_t *>(blob.data());
  if (blob.size() < 30 || !is_zip(b, blob.size()))
    return {};
  const auto method = static_cast<uint16_t>(b[8] | (b[9] << 8));
  const auto comp_size = rd_u32(b + 18);
  const auto uncomp_size = rd_u32(b + 22);
  const size_t data_off = 30ull + static_cast<uint16_t>(b[26] | (b[27] << 8)) +
                          static_cast<uint16_t>(b[28] | (b[29] << 8));
  if (blob.size() < data_off)
    return {};
  size_t n = blob.size() - data_off;
  if (comp_size > 0 && comp_size <= n)
    n = comp_size;
  if (method == 0)
    return blob.substr(data_off, n);
  if (method == 8)
    return inflate_window(b + data_off, n, -MAX_WBITS, uncomp_size);
  return {};
}

bool aes256_decrypt(const std::string &key, const uint8_t *data, size_t len,
                    std::string &out) {
  if (key.size() != 32 || len < 32 || (len % 16) != 0)
    return false;
  register_cipher(&aes_desc);
  const int idx = find_cipher("aes");
  const auto *k = reinterpret_cast<const uint8_t *>(key.data());

  uint8_t iv[16];
  symmetric_ECB ecb{};
  if (idx < 0 || ecb_start(idx, k, 32, 0, &ecb) != CRYPT_OK)
    return false;
  ecb_decrypt(data, iv, 16, &ecb);
  ecb_done(&ecb);

  std::string buf(len - 16, '\0');
  symmetric_CBC cbc{};
  if (cbc_start(idx, iv, k, 32, 0, &cbc) != CRYPT_OK)
    return false;
  const bool ok =
      cbc_decrypt(data + 16, reinterpret_cast<uint8_t *>(buf.data()),
                  static_cast<unsigned long>(buf.size()), &cbc) == CRYPT_OK;
  cbc_done(&cbc);
  if (!ok)
    return false;

  const auto pad = static_cast<unsigned char>(buf.back());
  if (pad == 0 || pad > 16)
    return false;
  for (size_t i = 0; i < pad; ++i) {
    if (static_cast<unsigned char>(buf[buf.size() - 1 - i]) != pad)
      return false;
  }
  buf.resize(buf.size() - pad);
  out = std::move(buf);
  return true;
}

std::string to_hex(const std::string &bytes) {
  static constexpr char digits[] = "0123456789abcdef";
  std::string hex;
  for (const auto c : bytes) {
    hex.push_back(digits[static_cast<unsigned char>(c) >> 4]);
    hex.push_back(digits[static_cast<unsigned char>(c) & 0xF]);
  }
  return hex;
}

std::string make_machine_id() {
  const auto random_hex = [] {
    uint8_t b[20];
    utils::cryptography::random::get_data(b, sizeof(b));
    return to_hex(std::string(reinterpret_cast<char *>(b), sizeof(b)));
  };
  std::string id;
  id.push_back('\0');
  id += "MessageObject";
  id.push_back('\0');
  for (const char *key : {"BB3", "FF2", "3B3"}) {
    id.push_back('\1');
    id += key;
    id.push_back('\0');
    id += random_hex();
    id.push_back('\0');
  }
  id.push_back('\b');
  id.push_back('\b');
  return id;
}

bool lzma_decode(const uint8_t *props, const uint8_t *src, size_t src_len,
                 uint8_t *dst, size_t dst_len) {
  static ISzAlloc alloc{[](ISzAllocPtr, size_t size) { return malloc(size); },
                        [](ISzAllocPtr, void *address) { free(address); }};
  SizeT dest_len = dst_len;
  SizeT source_len = src_len;
  ELzmaStatus status = LZMA_STATUS_NOT_SPECIFIED;
  return LzmaDecode(dst, &dest_len, src, &source_len, props, LZMA_PROPS_SIZE,
                    LZMA_FINISH_END, &status, &alloc) == SZ_OK &&
         dest_len == dst_len;
}

bool decode_chunk(const std::string &depot_key, const std::string &encrypted,
                  uint32_t crc, uint32_t uncompressed, std::string &plain) {
  std::string data;
  if (!aes256_decrypt(depot_key,
                      reinterpret_cast<const uint8_t *>(encrypted.data()),
                      encrypted.size(), data) ||
      data.size() < 30) {
    return false;
  }

  const auto *b = reinterpret_cast<const uint8_t *>(data.data());
  std::string out;
  if (b[0] == 'V' && b[1] == 'S' && b[2] == 'Z' && b[3] == 'a') {
    const auto size =
        *reinterpret_cast<const int32_t *>(data.data() + data.size() - 11);
    if (size <= 0)
      return false;
    out.resize(static_cast<size_t>(size));
    const auto got =
        ZSTD_decompress(out.data(), out.size(), b + 8, data.size() - 8 - 15);
    if (ZSTD_isError(got) || got != out.size())
      return false;
  } else if (b[0] == 'V' && b[1] == 'Z' && b[2] == 'a') {
    const auto size =
        *reinterpret_cast<const int32_t *>(data.data() + data.size() - 6);
    if (size <= 0)
      return false;
    out.resize(static_cast<size_t>(size));
    if (!lzma_decode(b + 7, b + 12, data.size() - 12 - 10,
                     reinterpret_cast<uint8_t *>(out.data()), out.size()))
      return false;
  } else if (is_zip(b, data.size())) {
    out = unzip_first_entry(data);
    if (out.empty())
      return false;
  } else {
    return false;
  }

  if ((uncompressed && out.size() != uncompressed) ||
      (crc && adler32(1L, reinterpret_cast<const Bytef *>(out.data()),
                      static_cast<uInt>(out.size())) != crc)) {
    return false;
  }
  plain = std::move(out);
  return true;
}

struct chunk_info {
  std::string sha;
  uint32_t crc{};
  uint64_t offset{};
  uint32_t original{};
};

struct file_mapping {
  std::string filename;
  uint64_t size{};
  uint32_t flags{};
  std::vector<chunk_info> chunks;
};

bool parse_manifest(const std::string &blob, const std::string &depot_key,
                    std::vector<file_mapping> &files) {
  const auto *p = reinterpret_cast<const uint8_t *>(blob.data());
  const auto *end = p + blob.size();
  std::string payload;
  bool encrypted_names = false;

  while (p + 8 <= end) {
    const auto magic = rd_u32(p);
    if (magic == MANIFEST_END_MAGIC)
      break;
    const auto len = rd_u32(p + 4);
    p += 8;
    if (p + len > end)
      return false;
    pb_in in{p, p + len};
    p += len;
    if (magic == MANIFEST_PAYLOAD_MAGIC) {
      payload = in.str();
    } else if (magic == MANIFEST_METADATA_MAGIC) {
      in.fields([&](uint32_t f, uint32_t wt) {
        if (f != 4 || wt != 0)
          return false;
        encrypted_names = in.var() != 0;
        return true;
      });
    } else if (magic != MANIFEST_SIGNATURE_MAGIC) {
      return false;
    }
  }
  if (payload.empty())
    return false;

  pb_in in = as_in(payload);
  in.fields([&](uint32_t f, uint32_t wt) {
    pb_in mapping;
    if (f != 1 || wt != 2 || !in.get_len(mapping))
      return false;
    file_mapping fm{};
    mapping.fields([&](uint32_t mf, uint32_t mwt) {
      if (mf == 1 && mwt == 2)
        fm.filename = mapping.read_bytes();
      else if (mf == 2 && mwt == 0)
        fm.size = mapping.var();
      else if (mf == 3 && mwt == 0)
        fm.flags = static_cast<uint32_t>(mapping.var());
      else if (mf == 6 && mwt == 2) {
        pb_in chunk;
        if (!mapping.get_len(chunk))
          return false;
        chunk_info c{};
        chunk.fields([&](uint32_t cf, uint32_t cwt) {
          if (cf == 1 && cwt == 2)
            c.sha = chunk.read_bytes();
          else if (cf == 2 && cwt == 0)
            c.crc = static_cast<uint32_t>(chunk.var());
          else if (cf == 3 && cwt == 0)
            c.offset = chunk.var();
          else if (cf == 4 && cwt == 0)
            c.original = static_cast<uint32_t>(chunk.var());
          else
            return false;
          return true;
        });
        fm.chunks.push_back(std::move(c));
      } else
        return false;
      return true;
    });
    files.push_back(std::move(fm));
    return true;
  });

  if (encrypted_names) {
    for (auto &fm : files) {
      const auto raw = utils::cryptography::base64::decode(fm.filename);
      std::string name;
      if (!aes256_decrypt(depot_key,
                          reinterpret_cast<const uint8_t *>(raw.data()),
                          raw.size(), name))
        return false;
      while (!name.empty() && name.back() == '\0')
        name.pop_back();
      std::replace(name.begin(), name.end(), '\\', '/');
      fm.filename = std::move(name);
    }
  }
  return true;
}

struct pubfile_info {
  uint32_t file_type{};
  uint64_t hcontent{};
  std::string file_url;
  std::string filename;
  std::vector<uint64_t> children;
};

struct steam_packet {
  uint32_t emsg{};
  int32_t session_id{};
  uint64_t steam_id{};
  uint64_t job_source{JOB_NONE};
  uint64_t job_target{JOB_NONE};
  int32_t eresult{2};
  std::string body;
};

size_t append_body(char *ptr, size_t size, size_t nmemb, void *userdata) {
  static_cast<std::string *>(userdata)->append(ptr, size * nmemb);
  return size * nmemb;
}

CURL *new_binary_session() {
  CURL *curl = curl_easy_init();
  if (!curl)
    return nullptr;
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Valve/Steam HTTP Client 1.0");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
  curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 16384L);
  return curl;
}

std::optional<std::string> get_binary(const std::string &url) {
  CURL *curl = new_binary_session();
  if (!curl)
    return {};
  std::string body;
  long status = 0;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append_body);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip,deflate");
  const bool ok =
      curl_easy_perform(curl) == CURLE_OK &&
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status) == CURLE_OK &&
      status == 200 && !body.empty();
  curl_easy_cleanup(curl);
  return ok ? std::optional(std::move(body)) : std::nullopt;
}

class steam3 {
public:
  explicit steam3(const context &ctx) : ctx_(ctx) {}
  ~steam3() { close(); }

  bool connect() {
    std::vector<std::string> hosts;
    if (const auto list =
            get_binary("https://api.steampowered.com/ISteamDirectory/"
                       "GetCMListForConnect/v1/?cellid=0")) {
      rapidjson::Document doc;
      if (!doc.Parse(list->c_str()).HasParseError() && doc.IsObject() &&
          doc.HasMember("response") &&
          doc["response"].HasMember("serverlist") &&
          doc["response"]["serverlist"].IsArray()) {
        for (const auto &entry : doc["response"]["serverlist"].GetArray()) {
          if (entry.IsObject() && entry.HasMember("type") &&
              entry["type"].IsString() &&
              std::string_view(entry["type"].GetString()) == "websockets" &&
              entry.HasMember("endpoint") && entry["endpoint"].IsString() &&
              !std::string_view(entry["endpoint"].GetString())
                   .starts_with("ext")) {
            hosts.emplace_back(entry["endpoint"].GetString());
          }
        }
      }
    }
    for (const char *fallback :
         {"cmp1-gru1.steamserver.net:27018", "cmp1-iad1.steamserver.net:27018",
          "cmp1-iad1.steamserver.net:443"}) {
      if (std::ranges::find(hosts, fallback) == hosts.end())
        hosts.emplace_back(fallback);
    }
    if (hosts.size() > 8)
      hosts.resize(8);

    for (const auto &host : hosts) {
      if (ctx_.cancelled())
        return false;
      if (open(host) && logon())
        return true;
      close();
    }
    printf("[ Workshop ] Could not connect to Steam\n");
    return false;
  }

  bool get_details(uint64_t pubfile, pubfile_info &info) {
    std::string req;
    pb_fixed64(req, 1, pubfile);
    pb_uint(req, 4, 1);
    pb_uint(req, 14, game::APP_ID);
    steam_packet pkt{};
    if (!call("PublishedFile.GetDetails#1", req, pkt))
      return false;
    pb_in in = as_in(pkt.body);
    in.fields([&](uint32_t f, uint32_t wt) {
      pb_in details;
      if (f != 1 || wt != 2 || !in.get_len(details))
        return false;
      details.fields([&](uint32_t df, uint32_t dwt) {
        if (df == 7 && dwt == 2)
          info.filename = details.read_bytes();
        else if (df == 10 && dwt == 2)
          info.file_url = details.read_bytes();
        else if (df == 14 && dwt == 1)
          details.read_fixed64(info.hcontent);
        else if (df == 34 && dwt == 0)
          info.file_type = static_cast<uint32_t>(details.var());
        else if (df == 53 && dwt == 2) {
          pb_in child;
          if (!details.get_len(child))
            return false;
          child.fields([&](uint32_t cf, uint32_t cwt) {
            uint64_t id = 0;
            if (cf != 1 || !(cwt == 0 ? child.read_var(id)
                                      : cwt == 1 && child.read_fixed64(id)))
              return false;
            info.children.push_back(id);
            return true;
          });
        } else
          return false;
        return true;
      });
      return true;
    });
    return info.hcontent != 0 || !info.file_url.empty() ||
           !info.children.empty();
  }

  bool get_depot_key(uint32_t depot, std::string &key) {
    std::string body;
    pb_uint(body, 1, depot);
    pb_uint(body, 2, game::APP_ID);
    const auto job = job_++;
    steam_packet pkt{};
    if (!send(EMSG_DEPOT_KEY, job, {}, body) ||
        !wait_job(job, EMSG_DEPOT_KEY_RESPONSE, pkt, 20s))
      return false;
    int32_t eresult = 2;
    pb_in in = as_in(pkt.body);
    in.fields([&](uint32_t f, uint32_t wt) {
      if (f == 1 && wt == 0)
        eresult = static_cast<int32_t>(in.var());
      else if (f == 3 && wt == 2)
        key = in.read_bytes();
      else
        return false;
      return true;
    });
    return eresult == 1 && key.size() == 32;
  }

  uint64_t get_manifest_code(uint32_t depot, uint64_t manifest) {
    std::string req;
    pb_uint(req, 1, game::APP_ID);
    pb_uint(req, 2, depot);
    pb_uint(req, 3, manifest);
    pb_bytes(req, 4, "public");
    steam_packet pkt{};
    uint64_t code = 0;
    if (call("ContentServerDirectory.GetManifestRequestCode#1", req, pkt)) {
      pb_in in = as_in(pkt.body);
      in.fields([&](uint32_t f, uint32_t wt) {
        if (f != 1 || wt != 0)
          return false;
        code = in.var();
        return true;
      });
    }
    return code;
  }

  std::string get_cdn_token(uint32_t depot, const std::string &host) {
    std::string req;
    pb_uint(req, 1, depot);
    pb_bytes(req, 2, host);
    pb_uint(req, 3, game::APP_ID);
    steam_packet pkt{};
    std::string token;
    if (call("ContentServerDirectory.GetCDNAuthToken#1", req, pkt)) {
      pb_in in = as_in(pkt.body);
      in.fields([&](uint32_t f, uint32_t wt) {
        if (f != 1 || wt != 2)
          return false;
        token = in.read_bytes();
        return true;
      });
    }
    return token;
  }

private:
  const context &ctx_;
  HINTERNET session_{};
  HINTERNET connection_{};
  HINTERNET socket_{};
  uint64_t steam_id_{ANON_STEAMID};
  int32_t session_id_{};
  uint64_t job_{1};
  int heartbeat_sec_{15};
  std::chrono::steady_clock::time_point last_beat_{};

  void close() {
    if (socket_) {
      WinHttpWebSocketClose(socket_, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
                            nullptr, 0);
      WinHttpCloseHandle(socket_);
    }
    if (connection_)
      WinHttpCloseHandle(connection_);
    if (session_)
      WinHttpCloseHandle(session_);
    socket_ = connection_ = session_ = nullptr;
  }

  bool open(const std::string &endpoint) {
    const auto colon = endpoint.rfind(':');
    const std::string host = endpoint.substr(0, colon);
    const auto port = static_cast<INTERNET_PORT>(
        colon == std::string::npos ? 443
                                   : std::stoul(endpoint.substr(colon + 1)));
    const std::wstring whost(host.begin(), host.end());

    session_ = WinHttpOpen(L"Valve/Steam HTTP Client 1.0",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session_)
      return false;
    WinHttpSetTimeouts(session_, 4000, 8000, 8000, 8000);
    connection_ = WinHttpConnect(session_, whost.c_str(), port, 0);
    if (!connection_)
      return false;

    HINTERNET request = WinHttpOpenRequest(
        connection_, L"GET", L"/cmsocket/", nullptr, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!request)
      return false;
    if (WinHttpSetOption(request, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, nullptr,
                         0) &&
        WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr,
                           0, 0, 0) &&
        WinHttpReceiveResponse(request, nullptr)) {
      socket_ = WinHttpWebSocketCompleteUpgrade(request, 0);
    }
    WinHttpCloseHandle(request);
    if (!socket_)
      return false;

    std::string hello;
    pb_uint(hello, 1, PROTOCOL);
    return send(EMSG_HELLO, JOB_NONE, {}, hello);
  }

  bool send(uint32_t emsg, uint64_t job_source, const std::string &job_name,
            const std::string &body) {
    std::string header;
    pb_fixed64(header, 1, steam_id_);
    pb_uint(header, 2, static_cast<uint32_t>(session_id_));
    if (job_source != JOB_NONE)
      pb_fixed64(header, 10, job_source);
    if (!job_name.empty())
      pb_bytes(header, 12, job_name);

    std::string packet;
    wr_u32(packet, emsg | PROTO_MASK);
    wr_u32(packet, static_cast<uint32_t>(header.size()));
    packet += header;
    packet += body;
    return socket_ &&
           WinHttpWebSocketSend(
               socket_, WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
               packet.data(), static_cast<DWORD>(packet.size())) == NO_ERROR;
  }

  bool receive_frame(std::string &frame) {
    DWORD timeout = 1000;
    WinHttpSetOption(socket_, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout,
                     sizeof(timeout));
    char buf[16384];
    for (;;) {
      DWORD got = 0;
      WINHTTP_WEB_SOCKET_BUFFER_TYPE type{};
      if (WinHttpWebSocketReceive(socket_, buf, sizeof(buf), &got, &type) !=
              NO_ERROR ||
          type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE)
        return !frame.empty();
      frame.append(buf, got);
      if (type == WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE ||
          type == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE)
        return true;
    }
  }

  void dispatch(const uint8_t *data, size_t n, std::vector<steam_packet> &out) {
    if (n < 8 || (rd_u32(data) & PROTO_MASK) == 0)
      return;
    const auto header_len = rd_u32(data + 4);
    if (8ull + header_len > n)
      return;

    steam_packet pkt{};
    pkt.emsg = rd_u32(data) & ~PROTO_MASK;
    pb_in header{data + 8, data + 8 + header_len};
    header.fields([&](uint32_t f, uint32_t wt) {
      if (f == 1 && wt == 1)
        return header.read_fixed64(pkt.steam_id);
      if (f == 2 && wt == 0)
        pkt.session_id = static_cast<int32_t>(header.var());
      else if (f == 10 && wt == 1)
        return header.read_fixed64(pkt.job_source);
      else if (f == 11 && wt == 1)
        return header.read_fixed64(pkt.job_target);
      else if (f == 13 && wt == 0)
        pkt.eresult = static_cast<int32_t>(header.var());
      else
        return false;
      return true;
    });
    pkt.body.assign(reinterpret_cast<const char *>(data + 8 + header_len),
                    n - 8 - header_len);

    if (pkt.emsg != EMSG_MULTI) {
      out.push_back(std::move(pkt));
      return;
    }

    uint64_t unzipped = 0;
    std::string inner;
    pb_in in = as_in(pkt.body);
    in.fields([&](uint32_t f, uint32_t wt) {
      if (f == 1 && wt == 0)
        unzipped = in.var();
      else if (f == 2 && wt == 2)
        inner = in.read_bytes();
      else
        return false;
      return true;
    });
    if (unzipped) {
      inner = inflate_window(reinterpret_cast<const uint8_t *>(inner.data()),
                             inner.size(), 16 + MAX_WBITS, 0);
    }
    const auto *ip = reinterpret_cast<const uint8_t *>(inner.data());
    for (size_t off = 0; off + 4 <= inner.size();) {
      const auto size = rd_u32(ip + off);
      off += 4;
      if (size == 0 || off + size > inner.size())
        break;
      dispatch(ip + off, size, out);
      off += size;
    }
  }

  bool wait_job(uint64_t job, uint32_t emsg, steam_packet &out,
                std::chrono::milliseconds timeout) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline && !ctx_.cancelled()) {
      if (std::chrono::steady_clock::now() - last_beat_ >
          std::chrono::seconds(heartbeat_sec_)) {
        last_beat_ = std::chrono::steady_clock::now();
        send(EMSG_HEARTBEAT, JOB_NONE, {}, {});
      }
      std::string frame;
      if (!receive_frame(frame))
        continue;
      std::vector<steam_packet> packets;
      dispatch(reinterpret_cast<const uint8_t *>(frame.data()), frame.size(),
               packets);
      for (auto &packet : packets) {
        if (packet.emsg == emsg &&
            (job == JOB_NONE || packet.job_target == job ||
             packet.job_source == job || emsg == EMSG_LOGON_RESPONSE)) {
          out = std::move(packet);
          return true;
        }
      }
    }
    return false;
  }

  bool logon() {
    std::string body;
    pb_uint(body, 1, PROTOCOL);
    pb_uint(body, 3, 0);
    pb_bytes(body, 6, "english");
    pb_uint(body, 7, EOS_WIN10);
    pb_bytes(body, 30, make_machine_id());
    pb_bytes(body, 80, "anonymous");
    const auto job = job_++;
    steam_packet pkt{};
    if (!send(EMSG_LOGON, job, {}, body) ||
        !wait_job(job, EMSG_LOGON_RESPONSE, pkt, 15s))
      return false;

    int32_t eresult = 2;
    pb_in in = as_in(pkt.body);
    in.fields([&](uint32_t f, uint32_t wt) {
      if (f == 1 && wt == 0)
        eresult = static_cast<int32_t>(in.var());
      else if (f == 2 && wt == 0)
        heartbeat_sec_ = std::max(1, static_cast<int>(in.var()));
      else
        return false;
      return true;
    });
    if (eresult != 1) {
      printf("[ Workshop ] Anonymous Steam logon failed (%d)\n", eresult);
      return false;
    }
    session_id_ = pkt.session_id ? pkt.session_id : session_id_;
    steam_id_ = pkt.steam_id ? pkt.steam_id : steam_id_;
    last_beat_ = std::chrono::steady_clock::now();
    return true;
  }

  bool call(const std::string &name, const std::string &req,
            steam_packet &out) {
    const auto job = job_++;
    return send(EMSG_SERVICE_METHOD_CALL, job, name, req) &&
           wait_job(job, EMSG_SERVICE_METHOD_RESPONSE, out, 20s) &&
           (out.eresult == 1 || !out.body.empty());
  }
};

std::vector<std::string> cdn_hosts() {
  std::vector<std::string> hosts;
  const auto resp = utils::http::get_data(
      "https://api.steampowered.com/IContentServerDirectoryService/"
      "GetServersForSteamPipe/v1/?cell_id=0&max_servers=20",
      {}, {}, 2);
  rapidjson::Document doc;
  if (!resp || doc.Parse(resp->c_str()).HasParseError() || !doc.IsObject() ||
      !doc.HasMember("response") || !doc["response"].HasMember("servers") ||
      !doc["response"]["servers"].IsArray())
    return hosts;
  for (const auto &server : doc["response"]["servers"].GetArray()) {
    for (const char *key : {"vhost", "host"}) {
      if (server.IsObject() && server.HasMember(key) &&
          server[key].IsString()) {
        hosts.emplace_back(server[key].GetString());
        break;
      }
    }
  }
  return hosts;
}

std::string with_token(const std::string &url, const std::string &token) {
  if (token.empty())
    return url;
  const char *separator = url.find('?') == std::string::npos ? "?" : "&";
  return url + separator +
         (token.find('=') == std::string::npos ? "token=" + token : token);
}

bool is_safe_relative(const std::string &name) {
  if (name.empty() || name.size() > 240 || name[0] == '/' || name[0] == '\\' ||
      name.find(':') != std::string::npos)
    return false;
  for (const auto &segment : std::filesystem::path(name)) {
    if (segment == "." || segment == ".." || segment.empty())
      return false;
  }
  return true;
}

struct chunk_transfer {
  std::string body;
  cdn_progress *progress{};
};

size_t receive_chunk(char *ptr, size_t size, size_t nmemb, void *userdata) {
  auto *transfer = static_cast<chunk_transfer *>(userdata);
  transfer->body.append(ptr, size * nmemb);
  transfer->progress->received += size * nmemb;
  return size * nmemb;
}

bool fetch_chunk(CURL *curl, const context &ctx, const chunk_info &chunk,
                 const std::string &key, const std::vector<std::string> &hosts,
                 uint32_t depot, const std::string &token, std::string &plain) {
  const auto hex = to_hex(chunk.sha);
  for (int attempt = 0; attempt < CHUNK_ATTEMPTS && !ctx.cancelled();
       ++attempt) {
    if (attempt > 0) {
      ctx.progress.retrying = true;
      std::this_thread::sleep_for(2s);
    }
    ctx.wait_while_paused();
    for (const auto &host : hosts) {
      if (ctx.cancelled())
        return false;
      chunk_transfer transfer{{}, &ctx.progress};
      const auto url = with_token("https://" + host + "/depot/" +
                                      std::to_string(depot) + "/chunk/" + hex,
                                  token);
      long status = 0;
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive_chunk);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &transfer);
      if (curl_easy_perform(curl) == CURLE_OK &&
          curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status) ==
              CURLE_OK &&
          status == 200 &&
          decode_chunk(key, transfer.body, chunk.crc, chunk.original, plain)) {
        return true;
      }
    }
  }
  return false;
}

bool write_file(const context &ctx, const file_mapping &file,
                const std::filesystem::path &root, const std::string &key,
                const std::vector<std::string> &hosts, uint32_t depot,
                const std::string &token) {
  if (!is_safe_relative(file.filename)) {
    printf("[ Workshop ] Rejected unsafe path %s\n", file.filename.c_str());
    return false;
  }
  const auto path = root / file.filename;
  std::error_code ec;
  if (file.flags & DIR_FLAG) {
    std::filesystem::create_directories(path, ec);
    return !ec;
  }
  std::filesystem::create_directories(path.parent_path(), ec);

  HANDLE created = CreateFileW(path.c_str(), GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (created == INVALID_HANDLE_VALUE)
    return false;
  DWORD ignored = 0;
  DeviceIoControl(created, FSCTL_SET_SPARSE, nullptr, 0, nullptr, 0, &ignored,
                  nullptr);
  LARGE_INTEGER end{};
  end.QuadPart = static_cast<LONGLONG>(file.size);
  const bool reserved = SetFilePointerEx(created, end, nullptr, FILE_BEGIN) &&
                        SetEndOfFile(created);
  CloseHandle(created);
  if (!reserved)
    return false;

  std::atomic<size_t> next{0};
  std::atomic<bool> failed{false};
  std::vector<std::thread> workers;
  const auto count = std::clamp<size_t>(file.chunks.size(), 1, MAX_WORKERS);
  for (size_t i = 0; i < count; ++i) {
    workers.emplace_back([&] {
      HANDLE handle = CreateFileW(
          path.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
          nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
      CURL *curl = new_binary_session();
      while (!failed && !ctx.cancelled()) {
        const size_t index = next++;
        if (index >= file.chunks.size())
          break;
        const auto &chunk = file.chunks[index];
        std::string plain;
        LARGE_INTEGER pos{};
        pos.QuadPart = static_cast<LONGLONG>(chunk.offset);
        DWORD wrote = 0;
        if (handle == INVALID_HANDLE_VALUE || !curl ||
            !fetch_chunk(curl, ctx, chunk, key, hosts, depot, token, plain) ||
            chunk.offset + plain.size() > file.size ||
            !SetFilePointerEx(handle, pos, nullptr, FILE_BEGIN) ||
            !WriteFile(handle, plain.data(), static_cast<DWORD>(plain.size()),
                       &wrote, nullptr) ||
            wrote != plain.size()) {
          failed = true;
          break;
        }
        ctx.progress.written += plain.size();
      }
      if (curl)
        curl_easy_cleanup(curl);
      if (handle != INVALID_HANDLE_VALUE)
        CloseHandle(handle);
    });
  }
  for (auto &worker : workers)
    worker.join();
  return !failed && !ctx.cancelled();
}

std::string unwrap_manifest(std::string blob, const std::string &key) {
  const auto unzip = [](std::string &data) {
    if (is_zip(reinterpret_cast<const uint8_t *>(data.data()), data.size())) {
      auto unzipped = unzip_first_entry(data);
      if (!unzipped.empty())
        data = std::move(unzipped);
    }
  };
  unzip(blob);
  std::string decrypted;
  if (blob.size() >= 4 &&
      rd_u32(reinterpret_cast<const uint8_t *>(blob.data())) !=
          MANIFEST_PAYLOAD_MAGIC &&
      aes256_decrypt(key, reinterpret_cast<const uint8_t *>(blob.data()),
                     blob.size(), decrypted)) {
    blob = std::move(decrypted);
    unzip(blob);
  }
  return blob;
}

bool download_content(steam3 &steam, const context &ctx, uint64_t hcontent,
                      const std::filesystem::path &destination) {
  std::string key;
  uint32_t depot = game::APP_ID;
  if (!steam.get_depot_key(depot, key) && !steam.get_depot_key(++depot, key)) {
    printf("[ Workshop ] Could not get the depot key\n");
    return false;
  }

  const auto code = steam.get_manifest_code(depot, hcontent);
  const auto hosts = cdn_hosts();
  if (hosts.empty()) {
    printf("[ Workshop ] No Steam content servers available\n");
    return false;
  }
  const auto token = steam.get_cdn_token(depot, hosts.front());

  std::optional<std::string> manifest;
  for (const auto &host : hosts) {
    if (ctx.cancelled())
      return false;
    std::string url = "https://" + host + "/depot/" + std::to_string(depot) +
                      "/manifest/" + std::to_string(hcontent) + "/5";
    if (code)
      url += "/" + std::to_string(code);
    manifest = get_binary(with_token(url, token));
    if (manifest && manifest->size() > 16)
      break;
    manifest.reset();
  }

  std::vector<file_mapping> files;
  if (!manifest ||
      !parse_manifest(unwrap_manifest(*manifest, key), key, files)) {
    printf("[ Workshop ] Could not read the workshop manifest\n");
    return false;
  }

  uint64_t total = 0;
  for (const auto &file : files) {
    if (!(file.flags & DIR_FLAG))
      total += file.size;
  }
  ctx.progress.total += total;

  std::ranges::sort(files, [](const file_mapping &a, const file_mapping &b) {
    return a.chunks.size() > b.chunks.size();
  });
  for (const auto &file : files) {
    if (!write_file(ctx, file, destination, key, hosts, depot, token)) {
      if (!ctx.cancelled())
        printf("[ Workshop ] Failed to download %s\n", file.filename.c_str());
      return false;
    }
  }
  return true;
}

bool download_pubfile(steam3 &steam, const context &ctx, uint64_t id,
                      const std::filesystem::path &destination) {
  pubfile_info info{};
  if (!steam.get_details(id, info)) {
    printf("[ Workshop ] Item %llu has no downloadable content\n",
           static_cast<unsigned long long>(id));
    return false;
  }

  if (info.file_type == 2 && !info.children.empty()) {
    return std::ranges::all_of(info.children, [&](uint64_t child) {
      return download_pubfile(steam, ctx, child, destination);
    });
  }

  if (info.hcontent) {
    return download_content(steam, ctx, info.hcontent, destination);
  }

  std::string name =
      info.filename.empty() ? std::to_string(id) + ".bin" : info.filename;
  const auto data = get_binary(info.file_url);
  return data && is_safe_relative(name) &&
         utils::io::write_file((destination / name).string(), *data);
}
} // namespace

bool download_from_steam(const std::string &workshop_id,
                         const std::filesystem::path &destination,
                         const download_job &job, cdn_progress &progress) {
  const auto id = std::strtoull(workshop_id.c_str(), nullptr, 10);
  if (!id)
    return false;

  try {
    const context ctx{job, progress};
    steam3 steam(ctx);
    return steam.connect() && download_pubfile(steam, ctx, id, destination);
  } catch (const std::exception &ex) {
    printf("[ Workshop ] Steam download failed: %s\n", ex.what());
    return false;
  }
}
} // namespace workshop
