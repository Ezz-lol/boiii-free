#include <std_include.hpp>
#include "gsc_parser.hpp"
#include <stdexcept>
#include <algorithm>

namespace gsc_compiler {
namespace {
struct parser_state {
  const std::vector<token> &tokens;
  size_t pos;

  parser_state(const std::vector<token> &toks) : tokens(toks), pos(0) {}

  const token &current() const { return tokens[pos]; }
  const token &peek(size_t offset = 0) const {
    size_t idx = pos + offset;
    return idx < tokens.size() ? tokens[idx] : tokens.back();
  }

  const token &advance() {
    const token &t = current();
    if (pos + 1 < tokens.size())
      pos++;
    return t;
  }

  bool at_end() const { return current().type == token_type::t_eof; }

  bool check(token_type t) const { return current().type == t; }

  bool match(token_type t) {
    if (check(t)) {
      advance();
      return true;
    }
    return false;
  }

  const token &expect(token_type t, const std::string &msg) {
    if (check(t))
      return advance();
    throw std::runtime_error(
        msg + " at line " + std::to_string(current().line) + ", column " +
        std::to_string(current().column) + " (got '" + current().value + "')");
  }
};

// Forward declarations
ast_ptr parse_expression(parser_state &s);
ast_ptr parse_statement(parser_state &s);
ast_ptr parse_block(parser_state &s);
ast_ptr parse_assignment_expr(parser_state &s);
ast_ptr parse_unary(parser_state &s);

// ---- Expression parsing (precedence climbing) ----

ast_ptr parse_primary(parser_state &s) {
  const auto &t = s.current();

  if (s.check(token_type::t_lparen)) {
    s.advance();
    auto expr = parse_expression(s);

    if (s.check(token_type::t_comma)) {
      auto vec = make_node(node_type::n_vector, "", t.line, t.column);
      vec->children.push_back(std::move(expr));
      s.expect(token_type::t_comma, "Expected ','");
      vec->children.push_back(parse_expression(s));
      s.expect(token_type::t_comma, "Expected ','");
      vec->children.push_back(parse_expression(s));
      s.expect(token_type::t_rparen, "Expected ')'");
      return vec;
    }

    s.expect(token_type::t_rparen, "Expected ')'");
    return expr;
  }

  if (s.check(token_type::t_lbracket)) {
    if (s.peek(1).type == token_type::t_lbracket) {
      s.advance();
      s.advance();
      auto ptr_expr = parse_expression(s);
      s.expect(token_type::t_rbracket, "Expected ']'");
      s.expect(token_type::t_rbracket, "Expected ']'");
      s.expect(token_type::t_lparen, "Expected '('");

      auto call = make_node(node_type::n_call_ptr, "", t.line, t.column);
      call->children.push_back(
          make_node(node_type::n_undefined, "undefined", t.line, t.column));
      call->children.push_back(std::move(ptr_expr));

      auto args = make_node(node_type::n_block, "args", t.line, t.column);
      if (!s.check(token_type::t_rparen)) {
        args->children.push_back(parse_expression(s));
        while (s.match(token_type::t_comma))
          args->children.push_back(parse_expression(s));
      }
      s.expect(token_type::t_rparen, "Expected ')'");
      call->children.push_back(std::move(args));

      return call;
    }

    s.advance();
    s.expect(token_type::t_rbracket, "Expected ']'");
    return make_node(node_type::n_empty_array, "[]", t.line, t.column);
  }

  if (s.check(token_type::t_number_int)) {
    auto &tok = s.advance();
    return make_node(node_type::n_number, tok.value, tok.line, tok.column);
  }
  if (s.check(token_type::t_number_float)) {
    auto &tok = s.advance();
    return make_node(node_type::n_float_number, tok.value, tok.line,
                     tok.column);
  }
  if (s.check(token_type::t_string)) {
    auto &tok = s.advance();
    return make_node(node_type::n_string, tok.value, tok.line, tok.column);
  }
  if (s.check(token_type::t_istring)) {
    auto &tok = s.advance();
    return make_node(node_type::n_istring, tok.value, tok.line, tok.column);
  }
  if (s.check(token_type::t_hash_string)) {
    auto &tok = s.advance();
    return make_node(node_type::n_hash_string, tok.value, tok.line, tok.column);
  }
  if (s.check(token_type::t_true)) {
    auto &tok = s.advance();
    return make_node(node_type::n_true_val, "true", tok.line, tok.column);
  }
  if (s.check(token_type::t_false)) {
    auto &tok = s.advance();
    return make_node(node_type::n_false_val, "false", tok.line, tok.column);
  }
  if (s.check(token_type::t_undefined)) {
    auto &tok = s.advance();
    return make_node(node_type::n_undefined, "undefined", tok.line, tok.column);
  }
  if (s.check(token_type::t_self)) {
    auto &tok = s.advance();
    return make_node(node_type::n_self, "self", tok.line, tok.column);
  }
  if (s.check(token_type::t_level)) {
    auto &tok = s.advance();
    return make_node(node_type::n_level, "level", tok.line, tok.column);
  }
  if (s.check(token_type::t_game)) {
    auto &tok = s.advance();
    return make_node(node_type::n_game, "game", tok.line, tok.column);
  }
  if (s.check(token_type::t_anim)) {
    auto &tok = s.advance();
    return make_node(node_type::n_anim, "anim", tok.line, tok.column);
  }

  if (s.check(token_type::t_world)) {
    auto &tok = s.advance();
    return make_node(node_type::n_world, "world", tok.line, tok.column);
  }

  if (s.check(token_type::t_classes)) {
    auto &tok = s.advance();
    return make_node(node_type::n_classes, "classes", tok.line, tok.column);
  }

  if (s.check(token_type::t_double_colon)) {
    s.advance();
    auto &name =
        s.expect(token_type::t_identifier, "Expected function name after '::'");
    return make_node(node_type::n_func_ref, name.value, t.line, t.column);
  }

  if (s.check(token_type::t_ampersand) &&
      s.peek(1).type == token_type::t_identifier) {
    auto &amp_tok = s.advance();
    auto &name_tok = s.advance();
    std::string ns_name = name_tok.value;

    // Support &path\to\namespace::function syntax
    while (s.check(token_type::t_backslash)) {
      ns_name += "\\";
      s.advance();
      if (s.check(token_type::t_identifier))
        ns_name += s.advance().value;
    }

    if (s.check(token_type::t_double_colon)) {
      s.advance();
      if (s.check(token_type::t_identifier)) {
        std::string func_name = s.advance().value;
        auto ref = make_node(node_type::n_func_ref, func_name, amp_tok.line,
                             amp_tok.column);
        ref->children.push_back(make_node(node_type::n_identifier, ns_name,
                                          amp_tok.line, amp_tok.column));
        return ref;
      }
    }

    return make_node(node_type::n_func_ref, ns_name, amp_tok.line,
                     amp_tok.column);
  }

  if (s.check(token_type::t_identifier)) {
    auto &tok = s.advance();
    std::string name = tok.value;

    while (s.check(token_type::t_backslash)) {
      name += "\\";
      s.advance();
      if (s.check(token_type::t_identifier)) {
        name += s.advance().value;
      }
    }

    if (s.check(token_type::t_double_colon)) {
      s.advance();
      if (s.check(token_type::t_identifier)) {
        std::string func_name = s.advance().value;
        if (s.check(token_type::t_lparen)) {
          s.advance();
          auto call =
              make_node(node_type::n_call, func_name, tok.line, tok.column);
          call->children.push_back(
              make_node(node_type::n_identifier, name, tok.line, tok.column));
          auto args =
              make_node(node_type::n_block, "args", tok.line, tok.column);
          if (!s.check(token_type::t_rparen)) {
            args->children.push_back(parse_expression(s));
            while (s.match(token_type::t_comma))
              args->children.push_back(parse_expression(s));
          }
          s.expect(token_type::t_rparen, "Expected ')'");
          call->children.push_back(std::move(args));
          return call;
        }
        // Function reference: ns::func
        auto ref =
            make_node(node_type::n_func_ref, func_name, tok.line, tok.column);
        ref->children.push_back(
            make_node(node_type::n_identifier, name, tok.line, tok.column));
        return ref;
      }
    }

    if (s.check(token_type::t_lparen)) {
      s.advance();
      auto call = make_node(node_type::n_call, name, tok.line, tok.column);
      call->children.push_back(
          make_node(node_type::n_identifier, "", tok.line, tok.column));
      auto args = make_node(node_type::n_block, "args", tok.line, tok.column);
      if (!s.check(token_type::t_rparen)) {
        args->children.push_back(parse_expression(s));
        while (s.match(token_type::t_comma))
          args->children.push_back(parse_expression(s));
      }
      s.expect(token_type::t_rparen, "Expected ')'");
      call->children.push_back(std::move(args));
      return call;
    }

    return make_node(node_type::n_identifier, name, tok.line, tok.column);
  }

  if (s.check(token_type::t_thread)) {
    s.advance();
    auto call_expr = parse_primary(s);
    auto thread_node =
        make_node(node_type::n_thread_call, "thread", t.line, t.column);
    thread_node->children.push_back(std::move(call_expr));
    return thread_node;
  }

  throw std::runtime_error("Unexpected token '" + t.value + "' at line " +
                           std::to_string(t.line) + ", column " +
                           std::to_string(t.column));
}

// Post-fix operations: field access, array access, method calls, ++, --
ast_ptr parse_postfix(parser_state &s) {
  auto expr = parse_primary(s);

  while (true) {
    if (s.check(token_type::t_dot)) {
      s.advance();
      if (s.check(token_type::t_identifier)) {
        auto &field_tok = s.advance();
        if (field_tok.value == "size") {
          auto size_node = make_node(node_type::n_size, "size", field_tok.line,
                                     field_tok.column);
          size_node->children.push_back(std::move(expr));
          expr = std::move(size_node);
        } else {
          auto field = make_node(node_type::n_field_access, field_tok.value,
                                 field_tok.line, field_tok.column);
          field->children.push_back(std::move(expr));
          expr = std::move(field);
        }
      }
      continue;
    }

    if (s.check(token_type::t_lbracket) &&
        s.peek(1).type != token_type::t_lbracket) {
      s.advance();
      auto key = parse_expression(s);
      s.expect(token_type::t_rbracket, "Expected ']'");
      auto arr =
          make_node(node_type::n_array_access, "", expr->line, expr->column);
      arr->children.push_back(std::move(expr));
      arr->children.push_back(std::move(key));
      expr = std::move(arr);
      continue;
    }

    auto is_method_start = [&]() -> bool {
      return s.check(token_type::t_identifier) ||
             s.check(token_type::t_thread) || s.check(token_type::t_waittill) ||
             s.check(token_type::t_waittillmatch) ||
             s.check(token_type::t_notify_kw) ||
             s.check(token_type::t_endon_kw);
    };

    if (is_method_start()) {
      bool is_thread = false;
      if (s.check(token_type::t_thread)) {
        is_thread = true;
        s.advance();
      }

      auto is_callable_token = [&]() -> bool {
        return s.check(token_type::t_identifier) ||
               s.check(token_type::t_waittill) ||
               s.check(token_type::t_waittillmatch) ||
               s.check(token_type::t_notify_kw) ||
               s.check(token_type::t_endon_kw);
      };

      if (is_callable_token()) {
        const auto &next = s.peek(1);
        if (next.type == token_type::t_lparen) {
          auto &method_tok = s.advance();
          std::string method_name = method_tok.value;

          node_type nt = node_type::n_method_call;
          if (method_name == "waittill")
            nt = node_type::n_waittill;
          else if (method_name == "notify")
            nt = node_type::n_notify;
          else if (method_name == "endon")
            nt = node_type::n_endon;

          s.advance();
          auto call =
              make_node(nt, method_name, method_tok.line, method_tok.column);
          call->children.push_back(std::move(expr));
          auto args = make_node(node_type::n_block, "args", method_tok.line,
                                method_tok.column);
          if (!s.check(token_type::t_rparen)) {
            args->children.push_back(parse_expression(s));
            while (s.match(token_type::t_comma))
              args->children.push_back(parse_expression(s));
          }
          s.expect(token_type::t_rparen, "Expected ')'");
          call->children.push_back(std::move(args));

          if (is_thread) {
            auto thread_node = make_node(node_type::n_thread_call, "thread",
                                         call->line, call->column);
            thread_node->children.push_back(std::move(call));
            expr = std::move(thread_node);
          } else {
            expr = std::move(call);
          }
          continue;
        }
        // Handle path-style method call: expr scripts\path::func(args)
        if (next.type == token_type::t_backslash ||
            next.type == token_type::t_double_colon) {
          auto &first_tok = s.advance();
          std::string ns = first_tok.value;

          while (s.check(token_type::t_backslash)) {
            ns += "\\";
            s.advance();
            if (s.check(token_type::t_identifier))
              ns += s.advance().value;
          }

          if (s.check(token_type::t_double_colon)) {
            s.advance();
            if (s.check(token_type::t_identifier) &&
                s.peek(1).type == token_type::t_lparen) {
              auto &func_tok = s.advance();
              s.advance();

              auto call = make_node(node_type::n_method_call, func_tok.value,
                                    func_tok.line, func_tok.column);
              call->children.push_back(std::move(expr));
              auto ns_node = make_node(node_type::n_identifier, ns,
                                       first_tok.line, first_tok.column);

              auto args = make_node(node_type::n_block, "args", func_tok.line,
                                    func_tok.column);
              if (!s.check(token_type::t_rparen)) {
                args->children.push_back(parse_expression(s));
                while (s.match(token_type::t_comma))
                  args->children.push_back(parse_expression(s));
              }
              s.expect(token_type::t_rparen, "Expected ')'");
              call->children.push_back(std::move(args));
              call->children.push_back(std::move(ns_node));

              if (is_thread) {
                auto thread_node = make_node(node_type::n_thread_call, "thread",
                                             call->line, call->column);
                thread_node->children.push_back(std::move(call));
                expr = std::move(thread_node);
              } else {
                expr = std::move(call);
              }
              continue;
            }
          }
        }
      }

      // Also handle: expr [[funcptr]](args)
      if (s.check(token_type::t_lbracket) &&
          s.peek(1).type == token_type::t_lbracket) {
        s.advance();
        s.advance(); // [[
        auto ptr_expr = parse_expression(s);
        s.expect(token_type::t_rbracket, "Expected ']'");
        s.expect(token_type::t_rbracket, "Expected ']'");
        s.expect(token_type::t_lparen, "Expected '('");

        auto call =
            make_node(node_type::n_call_ptr, "", expr->line, expr->column);
        call->children.push_back(std::move(expr));
        call->children.push_back(std::move(ptr_expr));

        auto args =
            make_node(node_type::n_block, "args", call->line, call->column);
        if (!s.check(token_type::t_rparen)) {
          args->children.push_back(parse_expression(s));
          while (s.match(token_type::t_comma))
            args->children.push_back(parse_expression(s));
        }
        s.expect(token_type::t_rparen, "Expected ')'");
        call->children.push_back(std::move(args));

        if (is_thread) {
          auto thread_node = make_node(node_type::n_thread_call, "thread",
                                       call->line, call->column);
          thread_node->children.push_back(std::move(call));
          expr = std::move(thread_node);
        } else {
          expr = std::move(call);
        }
        continue;
      }

      // Not a method call, restore thread token conceptually
      if (is_thread) {
        break;
      }
      break;
    }

    if (s.check(token_type::t_lbracket) &&
        s.peek(1).type == token_type::t_lbracket) {
      s.advance();
      s.advance();
      auto ptr_expr = parse_expression(s);
      s.expect(token_type::t_rbracket, "Expected ']'");
      s.expect(token_type::t_rbracket, "Expected ']'");
      s.expect(token_type::t_lparen, "Expected '('");

      auto call =
          make_node(node_type::n_call_ptr, "", expr->line, expr->column);
      call->children.push_back(std::move(expr));
      call->children.push_back(std::move(ptr_expr));

      auto args =
          make_node(node_type::n_block, "args", call->line, call->column);
      if (!s.check(token_type::t_rparen)) {
        args->children.push_back(parse_expression(s));
        while (s.match(token_type::t_comma))
          args->children.push_back(parse_expression(s));
      }
      s.expect(token_type::t_rparen, "Expected ')'");
      call->children.push_back(std::move(args));

      expr = std::move(call);
      continue;
    }

    if (s.check(token_type::t_inc)) {
      s.advance();
      auto node =
          make_node(node_type::n_inc_dec, "post++", expr->line, expr->column);
      node->children.push_back(std::move(expr));
      expr = std::move(node);
      continue;
    }
    if (s.check(token_type::t_dec)) {
      s.advance();
      auto node =
          make_node(node_type::n_inc_dec, "post--", expr->line, expr->column);
      node->children.push_back(std::move(expr));
      expr = std::move(node);
      continue;
    }

    break;
  }

  return expr;
}

int get_precedence(token_type t) {
  switch (t) {
  case token_type::t_or:
    return 1;
  case token_type::t_and:
    return 2;
  case token_type::t_pipe:
    return 3;
  case token_type::t_caret:
    return 4;
  case token_type::t_ampersand:
    return 5;
  case token_type::t_eq:
  case token_type::t_neq:
  case token_type::t_seq:
  case token_type::t_sneq:
    return 6;
  case token_type::t_lt:
  case token_type::t_gt:
  case token_type::t_leq:
  case token_type::t_geq:
    return 7;
  case token_type::t_lshift:
  case token_type::t_rshift:
    return 8;
  case token_type::t_plus:
  case token_type::t_minus:
    return 9;
  case token_type::t_star:
  case token_type::t_slash:
  case token_type::t_percent:
    return 10;
  default:
    return -1;
  }
}

ast_ptr parse_unary(parser_state &s) {
  const auto &t = s.current();

  if (s.check(token_type::t_bang)) {
    s.advance();
    auto operand = parse_unary(s);
    auto node = make_node(node_type::n_unary_op, "!", t.line, t.column);
    node->children.push_back(std::move(operand));
    return node;
  }
  if (s.check(token_type::t_minus)) {
    s.advance();
    auto operand = parse_unary(s);
    auto node = make_node(node_type::n_unary_op, "-", t.line, t.column);
    node->children.push_back(std::move(operand));
    return node;
  }
  if (s.check(token_type::t_tilde)) {
    s.advance();
    auto operand = parse_unary(s);
    auto node = make_node(node_type::n_unary_op, "~", t.line, t.column);
    node->children.push_back(std::move(operand));
    return node;
  }
  if (s.check(token_type::t_inc)) {
    s.advance();
    auto operand = parse_unary(s);
    auto node = make_node(node_type::n_inc_dec, "pre++", t.line, t.column);
    node->children.push_back(std::move(operand));
    return node;
  }
  if (s.check(token_type::t_dec)) {
    s.advance();
    auto operand = parse_unary(s);
    auto node = make_node(node_type::n_inc_dec, "pre--", t.line, t.column);
    node->children.push_back(std::move(operand));
    return node;
  }

  return parse_postfix(s);
}

ast_ptr parse_binary(parser_state &s, int min_prec) {
  auto left = parse_unary(s);

  while (true) {
    int prec = get_precedence(s.current().type);
    if (prec < min_prec)
      break;

    auto &op = s.advance();
    auto right = parse_binary(s, prec + 1);

    auto node = make_node(node_type::n_binary_op, op.value, op.line, op.column);
    node->children.push_back(std::move(left));
    node->children.push_back(std::move(right));
    left = std::move(node);
  }

  return left;
}

ast_ptr parse_expression(parser_state &s) {
  auto expr = parse_binary(s, 1);

  if (s.check(token_type::t_question)) {
    s.advance();
    auto true_expr = parse_expression(s);
    s.expect(token_type::t_colon, "Expected ':' in ternary expression");
    auto false_expr = parse_expression(s);

    auto ternary =
        make_node(node_type::n_ternary, "?:", expr->line, expr->column);
    ternary->children.push_back(std::move(expr));
    ternary->children.push_back(std::move(true_expr));
    ternary->children.push_back(std::move(false_expr));
    return ternary;
  }

  return expr;
}

ast_ptr parse_assignment_expr(parser_state &s) {
  auto left = parse_expression(s);

  auto is_assign_op = [](token_type t) -> bool {
    switch (t) {
    case token_type::t_assign:
    case token_type::t_plus_assign:
    case token_type::t_minus_assign:
    case token_type::t_star_assign:
    case token_type::t_slash_assign:
    case token_type::t_percent_assign:
    case token_type::t_amp_assign:
    case token_type::t_pipe_assign:
    case token_type::t_caret_assign:
    case token_type::t_lshift_assign:
    case token_type::t_rshift_assign:
      return true;
    default:
      return false;
    }
  };

  if (is_assign_op(s.current().type)) {
    auto &op = s.advance();
    auto right = parse_expression(s);
    auto node = make_node(node_type::n_assign, op.value, op.line, op.column);
    node->children.push_back(std::move(left));
    node->children.push_back(std::move(right));
    return node;
  }

  return left;
}

// ---- Statement parsing ----

ast_ptr parse_block(parser_state &s) {
  auto block =
      make_node(node_type::n_block, "", s.current().line, s.current().column);
  s.expect(token_type::t_lbrace, "Expected '{'");

  while (!s.check(token_type::t_rbrace) && !s.at_end()) {
    block->children.push_back(parse_statement(s));
  }

  s.expect(token_type::t_rbrace, "Expected '}'");
  return block;
}

ast_ptr parse_if(parser_state &s) {
  auto &tok = s.advance(); // 'if'
  s.expect(token_type::t_lparen, "Expected '(' after 'if'");
  auto cond = parse_expression(s);
  s.expect(token_type::t_rparen, "Expected ')'");

  auto body =
      s.check(token_type::t_lbrace) ? parse_block(s) : parse_statement(s);

  auto node = make_node(node_type::n_if, "", tok.line, tok.column);
  node->children.push_back(std::move(cond));
  node->children.push_back(std::move(body));

  if (s.match(token_type::t_else)) {
    if (s.check(token_type::t_if)) {
      node->children.push_back(parse_if(s));
    } else {
      auto else_body =
          s.check(token_type::t_lbrace) ? parse_block(s) : parse_statement(s);
      node->children.push_back(std::move(else_body));
    }
  }

  return node;
}

ast_ptr parse_while(parser_state &s) {
  auto &tok = s.advance(); // 'while'
  s.expect(token_type::t_lparen, "Expected '('");
  auto cond = parse_expression(s);
  s.expect(token_type::t_rparen, "Expected ')'");
  auto body =
      s.check(token_type::t_lbrace) ? parse_block(s) : parse_statement(s);

  auto node = make_node(node_type::n_while, "", tok.line, tok.column);
  node->children.push_back(std::move(cond));
  node->children.push_back(std::move(body));
  return node;
}

ast_ptr parse_do_while(parser_state &s) {
  auto &tok = s.advance(); // 'do'
  auto body = parse_block(s);
  s.expect(token_type::t_while, "Expected 'while'");
  s.expect(token_type::t_lparen, "Expected '('");
  auto cond = parse_expression(s);
  s.expect(token_type::t_rparen, "Expected ')'");
  s.expect(token_type::t_semicolon, "Expected ';'");

  auto node = make_node(node_type::n_do_while, "", tok.line, tok.column);
  node->children.push_back(std::move(cond));
  node->children.push_back(std::move(body));
  return node;
}

ast_ptr parse_for(parser_state &s) {
  auto &tok = s.advance(); // 'for'
  s.expect(token_type::t_lparen, "Expected '('");

  ast_ptr init = nullptr;
  if (!s.check(token_type::t_semicolon))
    init = parse_assignment_expr(s);
  s.expect(token_type::t_semicolon, "Expected ';'");

  ast_ptr cond = nullptr;
  if (!s.check(token_type::t_semicolon))
    cond = parse_expression(s);
  s.expect(token_type::t_semicolon, "Expected ';'");

  ast_ptr iter = nullptr;
  if (!s.check(token_type::t_rparen))
    iter = parse_assignment_expr(s);
  s.expect(token_type::t_rparen, "Expected ')'");

  auto body =
      s.check(token_type::t_lbrace) ? parse_block(s) : parse_statement(s);

  auto node = make_node(node_type::n_for, "", tok.line, tok.column);
  node->children.push_back(
      init ? std::move(init)
           : make_node(node_type::n_undefined, "", tok.line, tok.column));
  node->children.push_back(
      cond ? std::move(cond)
           : make_node(node_type::n_true_val, "true", tok.line, tok.column));
  node->children.push_back(
      iter ? std::move(iter)
           : make_node(node_type::n_undefined, "", tok.line, tok.column));
  node->children.push_back(std::move(body));
  return node;
}

ast_ptr parse_foreach(parser_state &s) {
  auto &tok = s.advance(); // 'foreach'
  s.expect(token_type::t_lparen, "Expected '('");

  auto first = s.expect(token_type::t_identifier, "Expected variable name");
  std::string key_name;
  std::string val_name;

  if (s.match(token_type::t_comma)) {
    key_name = first.value;
    val_name =
        s.expect(token_type::t_identifier, "Expected value variable").value;
  } else {
    val_name = first.value;
  }

  s.expect(token_type::t_in, "Expected 'in'");
  auto array_expr = parse_expression(s);
  s.expect(token_type::t_rparen, "Expected ')'");
  auto body =
      s.check(token_type::t_lbrace) ? parse_block(s) : parse_statement(s);

  auto node = make_node(node_type::n_foreach, val_name, tok.line, tok.column);
  // children[0] = key name (empty if single), children[1] = array expr,
  // children[2] = body
  node->children.push_back(
      make_node(node_type::n_identifier, key_name, tok.line, tok.column));
  node->children.push_back(std::move(array_expr));
  node->children.push_back(std::move(body));
  return node;
}

ast_ptr parse_switch(parser_state &s) {
  auto &tok = s.advance(); // 'switch'
  s.expect(token_type::t_lparen, "Expected '('");
  auto expr = parse_expression(s);
  s.expect(token_type::t_rparen, "Expected ')'");
  s.expect(token_type::t_lbrace, "Expected '{'");

  auto node = make_node(node_type::n_switch, "", tok.line, tok.column);
  node->children.push_back(std::move(expr));

  while (!s.check(token_type::t_rbrace) && !s.at_end()) {
    if (s.check(token_type::t_case)) {
      auto &case_tok = s.advance();
      auto case_val = parse_expression(s);
      s.expect(token_type::t_colon, "Expected ':'");

      auto case_node =
          make_node(node_type::n_case, "", case_tok.line, case_tok.column);
      case_node->children.push_back(std::move(case_val));

      auto body =
          make_node(node_type::n_block, "", case_tok.line, case_tok.column);
      while (!s.check(token_type::t_case) && !s.check(token_type::t_default) &&
             !s.check(token_type::t_rbrace) && !s.at_end()) {
        body->children.push_back(parse_statement(s));
      }
      case_node->children.push_back(std::move(body));
      node->children.push_back(std::move(case_node));
    } else if (s.check(token_type::t_default)) {
      auto &def_tok = s.advance();
      s.expect(token_type::t_colon, "Expected ':'");

      auto def_node = make_node(node_type::n_default_case, "", def_tok.line,
                                def_tok.column);
      auto body =
          make_node(node_type::n_block, "", def_tok.line, def_tok.column);
      while (!s.check(token_type::t_case) && !s.check(token_type::t_default) &&
             !s.check(token_type::t_rbrace) && !s.at_end()) {
        body->children.push_back(parse_statement(s));
      }
      def_node->children.push_back(std::move(body));
      node->children.push_back(std::move(def_node));
    } else {
      throw std::runtime_error(
          "Expected 'case' or 'default' in switch at line " +
          std::to_string(s.current().line));
    }
  }

  s.expect(token_type::t_rbrace, "Expected '}'");
  return node;
}

ast_ptr parse_statement(parser_state &s) {
  if (s.match(token_type::t_semicolon))
    return make_node(node_type::n_block, "", s.current().line,
                     s.current().column);

  if (s.check(token_type::t_lbrace))
    return parse_block(s);

  if (s.check(token_type::t_if))
    return parse_if(s);
  if (s.check(token_type::t_while))
    return parse_while(s);
  if (s.check(token_type::t_do))
    return parse_do_while(s);
  if (s.check(token_type::t_for))
    return parse_for(s);
  if (s.check(token_type::t_foreach))
    return parse_foreach(s);
  if (s.check(token_type::t_switch))
    return parse_switch(s);

  if (s.check(token_type::t_break)) {
    auto &tok = s.advance();
    s.expect(token_type::t_semicolon, "Expected ';'");
    return make_node(node_type::n_break, "", tok.line, tok.column);
  }
  if (s.check(token_type::t_continue)) {
    auto &tok = s.advance();
    s.expect(token_type::t_semicolon, "Expected ';'");
    return make_node(node_type::n_continue, "", tok.line, tok.column);
  }

  if (s.check(token_type::t_return)) {
    auto &tok = s.advance();
    auto node = make_node(node_type::n_return, "", tok.line, tok.column);
    if (!s.check(token_type::t_semicolon))
      node->children.push_back(parse_expression(s));
    s.expect(token_type::t_semicolon, "Expected ';'");
    return node;
  }

  if (s.check(token_type::t_wait)) {
    auto &tok = s.advance();
    auto node = make_node(node_type::n_wait, "", tok.line, tok.column);
    node->children.push_back(parse_expression(s));
    s.expect(token_type::t_semicolon, "Expected ';'");
    return node;
  }

  if (s.check(token_type::t_waittillframeend)) {
    auto &tok = s.advance();
    s.expect(token_type::t_semicolon, "Expected ';'");
    return make_node(node_type::n_waittillframeend, "", tok.line, tok.column);
  }

  if (s.check(token_type::t_waitrealtime)) {
    auto &tok = s.advance();
    auto node = make_node(node_type::n_waitrealtime, "", tok.line, tok.column);
    node->children.push_back(parse_expression(s));
    s.expect(token_type::t_semicolon, "Expected ';'");
    return node;
  }

  auto expr = parse_assignment_expr(s);
  s.expect(token_type::t_semicolon, "Expected ';'");

  auto stmt =
      make_node(node_type::n_expression_stmt, "", expr->line, expr->column);
  stmt->children.push_back(std::move(expr));
  return stmt;
}

ast_ptr parse_parameters(parser_state &s) {
  auto params = make_node(node_type::n_block, "params", s.current().line,
                          s.current().column);
  s.expect(token_type::t_lparen, "Expected '('");

  auto parse_one_param = [&]() {
    // Skip & or :: prefix (pass-by-reference / function-ref marker, handled by VM)
    s.match(token_type::t_ampersand);
    s.match(token_type::t_double_colon);
    auto &p = s.expect(token_type::t_identifier, "Expected parameter name");
    auto param_node =
        make_node(node_type::n_identifier, p.value, p.line, p.column);
    // Optional default value: param = expr
    if (s.match(token_type::t_assign)) {
      param_node->children.push_back(parse_expression(s));
    }
    params->children.push_back(std::move(param_node));
  };

  if (!s.check(token_type::t_rparen)) {
    parse_one_param();
    while (s.match(token_type::t_comma)) {
      parse_one_param();
    }
  }

  s.expect(token_type::t_rparen, "Expected ')'");
  return params;
}

ast_ptr parse_function_def(parser_state &s, bool autoexec, bool is_private) {
  auto &name_tok = s.expect(token_type::t_identifier, "Expected function name");
  auto func = make_node(node_type::n_function_def, name_tok.value,
                        name_tok.line, name_tok.column);
  std::string flags_val;
  if (autoexec)
    flags_val = "autoexec";
  if (is_private) {
    if (!flags_val.empty())
      flags_val += ",";
    flags_val += "private";
  }
  func->children.push_back(make_node(node_type::n_identifier, flags_val,
                                     name_tok.line, name_tok.column));
  func->children.push_back(parse_parameters(s));
  func->children.push_back(parse_block(s));
  return func;
}

ast_ptr parse_script(parser_state &s) {
  auto root = make_node(node_type::n_script, "", 1, 1);

  while (!s.at_end()) {
    if (s.check(token_type::t_namespace_kw)) {
      auto &tok = s.advance();
      auto &name =
          s.expect(token_type::t_identifier, "Expected namespace name");
      s.expect(token_type::t_semicolon, "Expected ';'");
      root->children.push_back(
          make_node(node_type::n_namespace, name.value, tok.line, tok.column));
      continue;
    }

    if (s.check(token_type::t_using)) {
      auto &tok = s.advance();
      std::string path;
      while (s.check(token_type::t_identifier) ||
             s.check(token_type::t_backslash)) {
        path += s.advance().value;
      }
      s.expect(token_type::t_semicolon, "Expected ';'");
      root->children.push_back(
          make_node(node_type::n_include, path, tok.line, tok.column));
      continue;
    }

    if (s.check(token_type::t_precache)) {
      s.advance();
      s.expect(token_type::t_lparen, "Expected '('");
      int depth = 1;
      while (!s.at_end() && depth > 0) {
        if (s.check(token_type::t_lparen))
          depth++;
        else if (s.check(token_type::t_rparen))
          depth--;
        if (depth > 0)
          s.advance();
      }
      s.expect(token_type::t_rparen, "Expected ')'");
      s.expect(token_type::t_semicolon, "Expected ';'");
      continue;
    }

    if (s.check(token_type::t_insert)) {
      s.advance();
      while (!s.at_end() && !s.check(token_type::t_semicolon))
        s.advance();
      if (s.check(token_type::t_semicolon))
        s.advance();
      continue;
    }

    bool is_autoexec = false;
    bool is_private = false;
    bool saw_function_kw = false;

    while (s.check(token_type::t_function_kw) ||
           s.check(token_type::t_autoexec) || s.check(token_type::t_private)) {
      if (s.check(token_type::t_function_kw)) {
        s.advance();
        saw_function_kw = true;
      } else if (s.check(token_type::t_autoexec)) {
        s.advance();
        is_autoexec = true;
      } else if (s.check(token_type::t_private)) {
        s.advance();
        is_private = true;
      }
    }

    if (saw_function_kw || is_autoexec || is_private) {
      root->children.push_back(parse_function_def(s, is_autoexec, is_private));
      continue;
    }

    if (s.check(token_type::t_identifier)) {
      if (s.peek(1).type == token_type::t_lparen) {
        root->children.push_back(parse_function_def(s, false, false));
        continue;
      }
    }

    throw std::runtime_error("Unexpected token '" + s.current().value +
                             "' at top-level, line " +
                             std::to_string(s.current().line));
  }

  return root;
}
} // namespace

parse_result parse(const std::vector<token> &tokens) {
  parse_result result{};
  parser_state s(tokens);

  try {
    result.root = parse_script(s);
    result.success = true;
  } catch (const std::runtime_error &e) {
    result.success = false;
    result.error = e.what();
    result.error_line = s.current().line;
    result.error_column = s.current().column;
  }

  return result;
}
} // namespace gsc_compiler
