#include <std_include.hpp>
#include "gsc_lexer.hpp"

namespace gsc_compiler
{
	namespace
	{
		struct lexer_state
		{
			const std::string& source;
			size_t pos;
			int line;
			int column;

			lexer_state(const std::string& src) : source(src), pos(0), line(1), column(1) {}

			char peek() const { return pos < source.size() ? source[pos] : '\0'; }
			char peek_next() const { return pos + 1 < source.size() ? source[pos + 1] : '\0'; }
			char peek_at(size_t offset) const { return pos + offset < source.size() ? source[pos + offset] : '\0'; }

			char advance()
			{
				char c = peek();
				pos++;
				if (c == '\n') { line++; column = 1; }
				else { column++; }
				return c;
			}

			bool at_end() const { return pos >= source.size(); }
		};

		bool is_ident_start(char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_'; }
		bool is_ident_char(char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }
		bool is_digit(char c) { return std::isdigit(static_cast<unsigned char>(c)); }
		bool is_hex_digit(char c)
		{
			auto cu = static_cast<unsigned char>(c);
			return std::isdigit(cu) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
		}

		void skip_whitespace_and_comments(lexer_state& s)
		{
			while (!s.at_end())
			{
				char c = s.peek();
				if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
				{
					s.advance();
				}
				else if (c == '/' && s.peek_next() == '/')
				{
					// line comment
					while (!s.at_end() && s.peek() != '\n') s.advance();
				}
				else if (c == '/' && s.peek_next() == '*')
				{
					// block comment
					s.advance(); s.advance();
					while (!s.at_end())
					{
						if (s.peek() == '*' && s.peek_next() == '/')
						{
							s.advance(); s.advance();
							break;
						}
						s.advance();
					}
				}
				else if (c == '/' && s.peek_next() == '@')
				{
					// doc comment: /@ ... @/
					s.advance(); s.advance();
					while (!s.at_end())
					{
						if (s.peek() == '@' && s.peek_next() == '/')
						{
							s.advance(); s.advance();
							break;
						}
						s.advance();
					}
				}
				else if (c == '#' && s.peek_next() == '/')
				{
					// devblock start (treat as comment/passthrough)
					// #/ ... /# is a dev block delimiter - skip it as a token
					break;
				}
				else break;
			}
		}

		bool try_read_string(lexer_state& s, token& tok, bool is_istring = false, bool is_hash = false)
		{
			char quote = s.peek();
			if (quote != '"') return false;

			tok.line = s.line;
			tok.column = s.column;
			s.advance();

			std::string val;
			while (!s.at_end() && s.peek() != '"')
			{
				if (s.peek() == '\\')
				{
					s.advance();
					char esc = s.advance();
					switch (esc)
					{
					case 'n': val += '\n'; break;
					case 't': val += '\t'; break;
					case 'r': val += '\r'; break;
					case '\\': val += '\\'; break;
					case '"': val += '"'; break;
					default: val += esc; break;
					}
				}
				else
				{
					val += s.advance();
				}
			}

			if (s.at_end()) return false;
			s.advance();

			tok.value = std::move(val);
			tok.type = is_istring ? token_type::t_istring : (is_hash ? token_type::t_hash_string : token_type::t_string);
			return true;
		}

		bool try_read_number(lexer_state& s, token& tok)
		{
			if (!is_digit(s.peek()) && !(s.peek() == '.' && is_digit(s.peek_next())))
				return false;

			tok.line = s.line;
			tok.column = s.column;

			std::string val;
			bool is_float = false;

			if (s.peek() == '0' && (s.peek_next() == 'x' || s.peek_next() == 'X'))
			{
				val += s.advance();
				val += s.advance();
				while (!s.at_end() && is_hex_digit(s.peek()))
					val += s.advance();
				tok.value = std::move(val);
				tok.type = token_type::t_number_int;
				return true;
			}

			while (!s.at_end() && (is_digit(s.peek()) || s.peek() == '.'))
			{
				if (s.peek() == '.')
				{
					if (is_float) break;
					is_float = true;
				}
				val += s.advance();
			}

			tok.value = std::move(val);
			tok.type = is_float ? token_type::t_number_float : token_type::t_number_int;
			return true;
		}

		token_type keyword_or_identifier(const std::string& word)
		{
			if (word == "if") return token_type::t_if;
			if (word == "else") return token_type::t_else;
			if (word == "while") return token_type::t_while;
			if (word == "for") return token_type::t_for;
			if (word == "foreach") return token_type::t_foreach;
			if (word == "in") return token_type::t_in;
			if (word == "do") return token_type::t_do;
			if (word == "switch") return token_type::t_switch;
			if (word == "case") return token_type::t_case;
			if (word == "default") return token_type::t_default;
			if (word == "break") return token_type::t_break;
			if (word == "continue") return token_type::t_continue;
			if (word == "return") return token_type::t_return;
			if (word == "thread") return token_type::t_thread;
			if (word == "wait") return token_type::t_wait;
			if (word == "waittillframeend") return token_type::t_waittillframeend;
			if (word == "true") return token_type::t_true;
			if (word == "false") return token_type::t_false;
			if (word == "undefined") return token_type::t_undefined;
			if (word == "self") return token_type::t_self;
			if (word == "level") return token_type::t_level;
			if (word == "game") return token_type::t_game;
			if (word == "anim") return token_type::t_anim;
			if (word == "autoexec") return token_type::t_autoexec;
			if (word == "function") return token_type::t_function_kw;
			if (word == "private") return token_type::t_private;
			if (word == "world") return token_type::t_world;
			if (word == "waitrealtime") return token_type::t_waitrealtime;
			if (word == "const") return token_type::t_const_kw;
			if (word == "waittill") return token_type::t_waittill;
			if (word == "waittillmatch") return token_type::t_waittillmatch;
			if (word == "notify") return token_type::t_notify_kw;
			if (word == "endon") return token_type::t_endon_kw;
			if (word == "classes") return token_type::t_classes;
			return token_type::t_identifier;
		}
	}

	lexer_result tokenize(const std::string& source)
	{
		lexer_result result{};
		result.success = true;

		lexer_state s(source);

		while (true)
		{
			skip_whitespace_and_comments(s);
			if (s.at_end())
			{
				result.tokens.push_back({token_type::t_eof, "", s.line, s.column});
				break;
			}

			token tok{};
			tok.line = s.line;
			tok.column = s.column;

			char c = s.peek();

			// directives: #namespace, #using, #include
			if (c == '#')
			{
				std::string directive;
				size_t save_pos = s.pos;
				int save_line = s.line;
				int save_col = s.column;
				s.advance();
				if (s.peek() == '"')
				{
					if (try_read_string(s, tok, false, true))
					{
						result.tokens.push_back(std::move(tok));
						continue;
					}
				}

				while (!s.at_end() && is_ident_char(s.peek()))
					directive += s.advance();

				if (directive == "namespace")
				{
					tok.type = token_type::t_namespace_kw;
					tok.value = "#namespace";
					result.tokens.push_back(std::move(tok));
					continue;
				}
				if (directive == "using" || directive == "include")
				{
					tok.type = token_type::t_using;
					tok.value = "#using";
					result.tokens.push_back(std::move(tok));
					continue;
				}
				if (directive == "precache")
				{
					tok.type = token_type::t_precache;
					tok.value = "#precache";
					result.tokens.push_back(std::move(tok));
					continue;
				}
				if (directive == "insert")
				{
					tok.type = token_type::t_insert;
					tok.value = "#insert";
					result.tokens.push_back(std::move(tok));
					continue;
				}

				if (directive.empty() && s.peek() == '/')
				{
					s.advance();
					while (!s.at_end())
					{
						if (s.peek() == '/' && s.peek_next() == '#')
						{
							s.advance(); s.advance();
							break;
						}
						s.advance();
					}
					continue;
				}

				s.pos = save_pos;
				s.line = save_line;
				s.column = save_col;
				s.advance();
				tok.type = token_type::t_hash;
				tok.value = "#";
				result.tokens.push_back(std::move(tok));
				continue;
			}

			// istring: &"..."
			if (c == '&' && s.peek_next() == '"')
			{
				s.advance();
				if (try_read_string(s, tok, true))
				{
					result.tokens.push_back(std::move(tok));
					continue;
				}
				result.success = false;
				result.error = "Unterminated istring";
				result.error_line = tok.line;
				result.error_column = tok.column;
				return result;
			}

			// &func_name is a function reference (same as ::func_name)
			// &namespace::func is a namespaced function reference
			if (c == '&' && is_ident_start(s.peek_next()))
			{
				s.advance();
				std::string word;
				while (!s.at_end() && is_ident_char(s.peek()))
					word += s.advance();

				// Check for namespace path: &ns\path::func or &ns::func
				if (!s.at_end() && (s.peek() == ':' || s.peek() == '\\'))
				{
					// Emit '&' as an ampersand, then the identifier. Let the parser handle namespacing
					result.tokens.push_back({token_type::t_ampersand, "&", tok.line, tok.column});
					result.tokens.push_back({token_type::t_identifier, word, tok.line, tok.column + 1});
				}
				else
				{
					// Simple &func. Emit as '::' followed by the identifier for func_ref
					result.tokens.push_back({token_type::t_double_colon, "::", tok.line, tok.column});
					result.tokens.push_back({token_type::t_identifier, word, tok.line, tok.column + 1});
				}
				continue;
			}

			if (c == '"')
			{
				if (try_read_string(s, tok))
				{
					result.tokens.push_back(std::move(tok));
					continue;
				}
				result.success = false;
				result.error = "Unterminated string literal";
				result.error_line = tok.line;
				result.error_column = tok.column;
				return result;
			}

			if (is_digit(c) || (c == '.' && is_digit(s.peek_next())))
			{
				if (try_read_number(s, tok))
				{
					result.tokens.push_back(std::move(tok));
					continue;
				}
			}

			if (is_ident_start(c))
			{
				std::string word;
				while (!s.at_end() && is_ident_char(s.peek()))
					word += s.advance();

				tok.value = word;
				tok.type = keyword_or_identifier(word);
				result.tokens.push_back(std::move(tok));
				continue;
			}

			// Multi-character operators
			char c2 = s.peek_next();

			// Three-char operators
			if (c == '=' && c2 == '=' && s.peek_at(2) == '=') { s.advance(); s.advance(); s.advance(); tok.type = token_type::t_seq; tok.value = "==="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '!' && c2 == '=' && s.peek_at(2) == '=') { s.advance(); s.advance(); s.advance(); tok.type = token_type::t_sneq; tok.value = "!=="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '<' && c2 == '<' && s.peek_at(2) == '=') { s.advance(); s.advance(); s.advance(); tok.type = token_type::t_lshift_assign; tok.value = "<<="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '>' && c2 == '>' && s.peek_at(2) == '=') { s.advance(); s.advance(); s.advance(); tok.type = token_type::t_rshift_assign; tok.value = ">>="; result.tokens.push_back(std::move(tok)); continue; }

			// Two-char operators
			if (c == '+' && c2 == '+') { s.advance(); s.advance(); tok.type = token_type::t_inc; tok.value = "++"; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '-' && c2 == '-') { s.advance(); s.advance(); tok.type = token_type::t_dec; tok.value = "--"; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '-' && c2 == '>') { s.advance(); s.advance(); tok.type = token_type::t_arrow; tok.value = "->"; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '+' && c2 == '=') { s.advance(); s.advance(); tok.type = token_type::t_plus_assign; tok.value = "+="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '-' && c2 == '=') { s.advance(); s.advance(); tok.type = token_type::t_minus_assign; tok.value = "-="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '*' && c2 == '=') { s.advance(); s.advance(); tok.type = token_type::t_star_assign; tok.value = "*="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '/' && c2 == '=') { s.advance(); s.advance(); tok.type = token_type::t_slash_assign; tok.value = "/="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '%' && c2 == '=') { s.advance(); s.advance(); tok.type = token_type::t_percent_assign; tok.value = "%="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '&' && c2 == '=') { s.advance(); s.advance(); tok.type = token_type::t_amp_assign; tok.value = "&="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '|' && c2 == '=') { s.advance(); s.advance(); tok.type = token_type::t_pipe_assign; tok.value = "|="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '^' && c2 == '=') { s.advance(); s.advance(); tok.type = token_type::t_caret_assign; tok.value = "^="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '=' && c2 == '=') { s.advance(); s.advance(); tok.type = token_type::t_eq; tok.value = "=="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '!' && c2 == '=') { s.advance(); s.advance(); tok.type = token_type::t_neq; tok.value = "!="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '<' && c2 == '=') { s.advance(); s.advance(); tok.type = token_type::t_leq; tok.value = "<="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '>' && c2 == '=') { s.advance(); s.advance(); tok.type = token_type::t_geq; tok.value = ">="; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '&' && c2 == '&') { s.advance(); s.advance(); tok.type = token_type::t_and; tok.value = "&&"; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '|' && c2 == '|') { s.advance(); s.advance(); tok.type = token_type::t_or; tok.value = "||"; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '<' && c2 == '<') { s.advance(); s.advance(); tok.type = token_type::t_lshift; tok.value = "<<"; result.tokens.push_back(std::move(tok)); continue; }
			if (c == '>' && c2 == '>') { s.advance(); s.advance(); tok.type = token_type::t_rshift; tok.value = ">>"; result.tokens.push_back(std::move(tok)); continue; }
			if (c == ':' && c2 == ':') { s.advance(); s.advance(); tok.type = token_type::t_double_colon; tok.value = "::"; result.tokens.push_back(std::move(tok)); continue; }

			// Single-char operators
			s.advance();
			switch (c)
			{
			case '+': tok.type = token_type::t_plus; tok.value = "+"; break;
			case '-': tok.type = token_type::t_minus; tok.value = "-"; break;
			case '*': tok.type = token_type::t_star; tok.value = "*"; break;
			case '/': tok.type = token_type::t_slash; tok.value = "/"; break;
			case '%': tok.type = token_type::t_percent; tok.value = "%"; break;
			case '&': tok.type = token_type::t_ampersand; tok.value = "&"; break;
			case '|': tok.type = token_type::t_pipe; tok.value = "|"; break;
			case '^': tok.type = token_type::t_caret; tok.value = "^"; break;
			case '~': tok.type = token_type::t_tilde; tok.value = "~"; break;
			case '!': tok.type = token_type::t_bang; tok.value = "!"; break;
			case '=': tok.type = token_type::t_assign; tok.value = "="; break;
			case '<': tok.type = token_type::t_lt; tok.value = "<"; break;
			case '>': tok.type = token_type::t_gt; tok.value = ">"; break;
			case '(': tok.type = token_type::t_lparen; tok.value = "("; break;
			case ')': tok.type = token_type::t_rparen; tok.value = ")"; break;
			case '{': tok.type = token_type::t_lbrace; tok.value = "{"; break;
			case '}': tok.type = token_type::t_rbrace; tok.value = "}"; break;
			case '[': tok.type = token_type::t_lbracket; tok.value = "["; break;
			case ']': tok.type = token_type::t_rbracket; tok.value = "]"; break;
			case ';': tok.type = token_type::t_semicolon; tok.value = ";"; break;
			case ',': tok.type = token_type::t_comma; tok.value = ","; break;
			case '.': tok.type = token_type::t_dot; tok.value = "."; break;
			case ':': tok.type = token_type::t_colon; tok.value = ":"; break;
			case '\\': tok.type = token_type::t_backslash; tok.value = "\\"; break;
			case '?': tok.type = token_type::t_question; tok.value = "?"; break;
			default:
				result.success = false;
				result.error = std::string("Unexpected character '") + c + "'";
				result.error_line = tok.line;
				result.error_column = tok.column;
				return result;
			}

			result.tokens.push_back(std::move(tok));
		}

		return result;
	}
}
