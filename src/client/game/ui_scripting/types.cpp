#include <std_include.hpp>
#include "types.hpp"
#include "execution.hpp"

namespace ui_scripting {
using namespace game::ui::lua::hks;
/***************************************************************
 * Lightuserdata
 **************************************************************/

lightuserdata::lightuserdata(void *ptr_) : ptr(ptr_) {}

/***************************************************************
 * Userdata
 **************************************************************/

userdata::userdata(void *ptr_) : ptr(ptr_) { this->add(); }

userdata::userdata(const userdata &other) { this->operator=(other); }

userdata::userdata(userdata &&other) noexcept {
  this->ptr = other.ptr;
  this->ref = other.ref;
  other.ref = 0;
}

userdata::~userdata() { this->release(); }

userdata &userdata::operator=(const userdata &other) {
  if (&other != this) {
    this->release();
    this->ptr = other.ptr;
    this->ref = other.ref;
    this->add();
  }

  return *this;
}

userdata &userdata::operator=(userdata &&other) noexcept {
  if (&other != this) {
    this->release();
    this->ptr = other.ptr;
    this->ref = other.ref;
    other.ref = 0;
  }

  return *this;
}

void userdata::add() {
  HksObject value{};
  value.v.ptr = this->ptr;
  value.t = HksObjectType::TUSERDATA;

  lua_State *state = *primary_luaVM;
  if (state) {
    const auto top = state->m_apistack.top;

    push_value(value);

    this->ref = hksi_luaL_ref(state, -10000);
    state->m_apistack.top = top;
  }
}

void userdata::release() {
  lua_State *state = *primary_luaVM;
  if (this->ref && state) {
    hksi_luaL_unref(state, -10000, this->ref);
  }
}

void userdata::set(const script_value &key, const script_value &value) const {
  set_field(*this, key, value);
}

script_value userdata::get(const script_value &key) const {
  return get_field(*this, key);
}

userdata_value userdata::operator[](const script_value &key) const {
  return {*this, key};
}

userdata_value::userdata_value(const userdata &table, const script_value &key)
    : userdata_(table), key_(key) {
  this->value_ = this->userdata_.get(key).get_raw();
}

void userdata_value::operator=(const script_value &value) {
  this->userdata_.set(this->key_, value);
  this->value_ = value.get_raw();
}

bool userdata_value::operator==(const script_value &value) {
  return this->userdata_.get(this->key_) == value;
}

/***************************************************************
 * Table
 **************************************************************/

table::table() {
  lua_State *state = *primary_luaVM;
  this->ptr = Hashtable_Create(state, 0, 0);
  this->add();
}

table::table(HashTable *ptr_) : ptr(ptr_) { this->add(); }

table::table(const table &other) { this->operator=(other); }

table::table(table &&other) noexcept {
  this->ptr = other.ptr;
  this->ref = other.ref;
  other.ref = 0;
}

table::~table() { this->release(); }

table &table::operator=(const table &other) {
  if (&other != this) {
    this->release();
    this->ptr = other.ptr;
    this->ref = other.ref;
    this->add();
  }

  return *this;
}

table &table::operator=(table &&other) noexcept {
  if (&other != this) {
    this->release();
    this->ptr = other.ptr;
    this->ref = other.ref;
    other.ref = 0;
  }

  return *this;
}

void table::add() {
  HksObject value{};
  value.v.table = this->ptr;
  value.t = HksObjectType::TTABLE;

  lua_State *state = *primary_luaVM;
  if (state) {
    const auto top = state->m_apistack.top;

    push_value(value);

    this->ref = hksi_luaL_ref(state, -10000);
    state->m_apistack.top = top;
  }
}

void table::release() {
  lua_State *state = *primary_luaVM;
  if (this->ref && state) {
    hksi_luaL_unref(state, -10000, this->ref);
  }
}

void table::set(const script_value &key, const script_value &value) const {
  lua_State *state = *primary_luaVM;
  if (state) {
    set_field(*this, key, value);
  }
}

table_value table::operator[](const script_value &key) const {
  return {*this, key};
}

script_value table::get(const script_value &key) const {
  lua_State *state = *primary_luaVM;
  if (state) {
    return get_field(*this, key);
  }

  return {};
}

table_value::table_value(const table &table, const script_value &key)
    : table_(table), key_(key) {
  lua_State *state = *primary_luaVM;
  if (state) {
    this->value_ = this->table_.get(key).get_raw();
  }
}

void table_value::operator=(const script_value &value) {
  this->table_.set(this->key_, value);
  this->value_ = value.get_raw();
}

void table_value::operator=(const table_value &value) {
  this->table_.set(this->key_, value);
  this->value_ = value.get_raw();
}

bool table_value::operator==(const script_value &value) {
  return this->table_.get(this->key_) == value;
}

bool table_value::operator==(const table_value &value) {
  return this->table_.get(this->key_) == value;
}

/***************************************************************
 * Function
 **************************************************************/

function::function(lua_function func) {
  lua_State *state = *primary_luaVM;
  if (state) {
    this->ptr = cclosure_Create(state, func, 0, 0, 0);
    this->type = HksObjectType::TCFUNCTION;
    this->add();
  }
}

function::function(cclosure *ptr_, HksObjectType type_)
    : ptr(ptr_), type(type_) {
  this->add();
}

function::function(const function &other) { this->operator=(other); }

function::function(function &&other) noexcept {
  this->ptr = other.ptr;
  this->type = other.type;
  this->ref = other.ref;
  other.ref = 0;
}

function::~function() { this->release(); }

function &function::operator=(const function &other) {
  if (&other != this) {
    this->release();
    this->ptr = other.ptr;
    this->type = other.type;
    this->ref = other.ref;
    this->add();
  }

  return *this;
}

function &function::operator=(function &&other) noexcept {
  if (&other != this) {
    this->release();
    this->ptr = other.ptr;
    this->type = other.type;
    this->ref = other.ref;
    other.ref = 0;
  }

  return *this;
}

void function::add() {
  HksObject value{};
  value.v.cClosure = this->ptr;
  value.t = this->type;

  lua_State *state = *primary_luaVM;
  if (state) {
    const auto top = state->m_apistack.top;

    push_value(value);

    this->ref = hksi_luaL_ref(state, -10000);
    state->m_apistack.top = top;
  }
}

void function::release() {
  lua_State *state = *primary_luaVM;
  if (this->ref && state) {
    hksi_luaL_unref(state, -10000, this->ref);
  }
}

arguments function::call(const arguments &arguments) const {
  return call_script_function(*this, arguments);
}

arguments function::operator()(const arguments &arguments) const {
  return this->call(arguments);
}

arguments function::operator()() const { return this->call({}); }
} // namespace ui_scripting