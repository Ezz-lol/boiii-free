#pragma once

#include "gsc_lexer.hpp"
#include <memory>
#include <variant>

namespace gsc_compiler {
// AST node types
enum class node_type {
  n_script,       // root: includes, namespace, functions
  n_include,      // #using path;
  n_namespace,    // #namespace name;
  n_function_def, // function definition
  n_block,        // { statements }
  n_return,       // return [expr];
  n_if,           // if (cond) block [else block]
  n_while,        // while (cond) block
  n_do_while,     // do block while (cond);
  n_for,          // for (init; cond; iter) block
  n_foreach,      // foreach (val in arr) / foreach (key, val in arr)
  n_switch,       // switch (expr) { cases }
  n_case,         // case expr:
  n_default_case, // default:
  n_break,
  n_continue,
  n_wait, // wait(expr);
  n_waittillframeend,
  n_expression_stmt, // expr;
  n_assign,          // var = expr / var += expr / etc.
  n_binary_op,       // a + b, a == b, etc.
  n_unary_op,        // !a, -a, ~a
  n_inc_dec,         // a++, a--, ++a, --a
  n_call,            // function call
  n_method_call,     // obj method_call(...)
  n_call_ptr,        // [[expr]](args)
  n_thread_call,     // thread call(...)
  n_identifier,
  n_number,
  n_float_number,
  n_string,
  n_istring,
  n_hash_string,
  n_undefined,
  n_true_val,
  n_false_val,
  n_self,
  n_level,
  n_game,
  n_anim,
  n_field_access, // expr.field
  n_array_access, // expr[key]
  n_size,         // expr.size
  n_vector,       // (x, y, z)
  n_empty_array,  // []
  n_func_ref,     // ::func or path::func
  n_waittill,     // obj waittill("event", ...)
  n_notify,       // obj notify("event", ...)
  n_endon,        // obj endon("event")
  n_world,        // world
  n_classes,      // classes
  n_precache,     // #precache(type, value)
  n_waitrealtime, // waitrealtime(expr)
  n_ternary,      // cond ? true_expr : false_expr
};

struct ast_node {
  node_type type;
  std::string value;
  int line;
  int column;
  std::vector<std::shared_ptr<ast_node>> children;
  // for function defs: children[0] = params, children[1] = body
  // for binary ops: children[0] = left, children[1] = right
};

using ast_ptr = std::shared_ptr<ast_node>;

inline ast_ptr make_node(node_type t, const std::string &val, int line,
                         int col) {
  auto n = std::make_shared<ast_node>();
  n->type = t;
  n->value = val;
  n->line = line;
  n->column = col;
  return n;
}

struct parse_result {
  bool success;
  ast_ptr root;
  std::string error;
  int error_line;
  int error_column;
};

parse_result parse(const std::vector<token> &tokens);
} // namespace gsc_compiler