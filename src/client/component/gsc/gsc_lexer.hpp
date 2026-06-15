#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace gsc_compiler {
enum class token_type {
  // literals
  t_string,
  t_istring,     // &"..."
  t_hash_string, // #"..."
  t_number_int,
  t_number_float,
  t_identifier,

  // keywords
  t_if,
  t_else,
  t_while,
  t_for,
  t_foreach,
  t_in,
  t_do,
  t_switch,
  t_case,
  t_default,
  t_break,
  t_continue,
  t_return,
  t_thread,
  t_wait,
  t_waittillframeend,
  t_true,
  t_false,
  t_undefined,
  t_self,
  t_level,
  t_game,
  t_anim,
  t_autoexec,
  t_namespace_kw,  // #namespace
  t_using,         // #using
  t_include,       // #include
  t_function_kw,   // function
  t_private,       // private
  t_world,         // world
  t_waitrealtime,  // waitrealtime
  t_const_kw,      // const
  t_precache,      // #precache
  t_insert,        // #insert
  t_waittill,      // waittill
  t_waittillmatch, // waittillmatch
  t_notify_kw,     // notify
  t_endon_kw,      // endon
  t_classes,       // classes

  // operators
  t_plus,      // +
  t_minus,     // -
  t_star,      // *
  t_slash,     // /
  t_percent,   // %
  t_ampersand, // &
  t_pipe,      // |
  t_caret,     // ^
  t_tilde,     // ~
  t_bang,      // !
  t_assign,    // =
  t_lt,        // <
  t_gt,        // >

  t_plus_assign,    // +=
  t_minus_assign,   // -=
  t_star_assign,    // *=
  t_slash_assign,   // /=
  t_percent_assign, // %=
  t_amp_assign,     // &=
  t_pipe_assign,    // |=
  t_caret_assign,   // ^=
  t_lshift_assign,  // <<=
  t_rshift_assign,  // >>=

  t_eq,     // ==
  t_neq,    // !=
  t_leq,    // <=
  t_geq,    // >=
  t_seq,    // ===
  t_sneq,   // !==
  t_and,    // &&
  t_or,     // ||
  t_lshift, // <<
  t_rshift, // >>
  t_inc,    // ++
  t_dec,    // --
  t_arrow,  // ->

  // ternary
  t_question, // ?

  // delimiters
  t_lparen,       // (
  t_rparen,       // )
  t_lbrace,       // {
  t_rbrace,       // }
  t_lbracket,     // [
  t_rbracket,     // ]
  t_semicolon,    // ;
  t_comma,        // ,
  t_dot,          // .
  t_colon,        // :
  t_double_colon, // ::
  t_hash,         // #
  t_backslash,    // (path separator)

  t_eof,
};

struct token {
  token_type type;
  std::string value;
  int line;
  int column;
};

struct lexer_result {
  bool success;
  std::vector<token> tokens;
  std::string error;
  int error_line;
  int error_column;
};

lexer_result tokenize(const std::string &source);
} // namespace gsc_compiler