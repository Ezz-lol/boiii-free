#include <std_include.hpp>
#include "gsc_emitter.hpp"
#include <algorithm>
#include <numeric>
#include <cstring>

namespace gsc_compiler
{
	namespace
	{
		constexpr uint64_t T7_MAGIC = 0x1C000A0D43534780;
		constexpr uint32_t HASH_IV = 0x4B9ACE2F;
		constexpr uint32_t HASH_KEY = 0x1000193;
		constexpr uint16_t OP_SIZE = 2; // 2 bytes per opcode on LE PC

		uint32_t gsc_hash(const std::string& input)
		{
			uint32_t hash = HASH_IV;
			for (char c : input)
				hash = (static_cast<uint32_t>(std::tolower(static_cast<unsigned char>(c))) ^ hash) * HASH_KEY;
			hash *= HASH_KEY;
			return hash;
		}

		uint32_t align_value(uint32_t val, uint32_t alignment)
		{
			return (val + alignment - 1) & ~(alignment - 1);
		}

		void write_u8(std::vector<uint8_t>& buf, uint8_t v) { buf.push_back(v); }
		void write_u16(std::vector<uint8_t>& buf, uint16_t v) { buf.push_back(v & 0xFF); buf.push_back((v >> 8) & 0xFF); }
		void write_u32(std::vector<uint8_t>& buf, uint32_t v) { auto s = buf.size(); buf.resize(s + 4); std::memcpy(&buf[s], &v, 4); }
		void write_i16(std::vector<uint8_t>& buf, int16_t v) { write_u16(buf, static_cast<uint16_t>(v)); }
		void write_float(std::vector<uint8_t>& buf, float v) { auto s = buf.size(); buf.resize(s + 4); std::memcpy(&buf[s], &v, 4); }

		void pad_to(std::vector<uint8_t>& buf, uint32_t alignment)
		{
			while (buf.size() % alignment != 0) buf.push_back(0);
		}

		void write_at_u16(std::vector<uint8_t>& buf, size_t offset, uint16_t v)
		{
			buf[offset] = v & 0xFF;
			buf[offset + 1] = (v >> 8) & 0xFF;
		}

		void write_at_u32(std::vector<uint8_t>& buf, size_t offset, uint32_t v)
		{
			std::memcpy(&buf[offset], &v, 4);
		}

		void write_at_i16(std::vector<uint8_t>& buf, size_t offset, int16_t v)
		{
			write_at_u16(buf, offset, static_cast<uint16_t>(v));
		}

		uint64_t align_value64(uint64_t val, uint64_t alignment)
		{
			return (val + alignment - 1) & ~(alignment - 1);
		}

		// T7 PC opcode table from the proven reference compiler (Black-Ops-3-GSC-Compiler-1.0)
		uint16_t map_opcode(script_opcode op)
		{
			switch (op)
			{
			case script_opcode::OP_End:                          return 0x0010;
			case script_opcode::OP_Return:                       return 0x00A3;
			case script_opcode::OP_GetUndefined:                 return 0x0179;
			case script_opcode::OP_GetZero:                      return 0x002A;
			case script_opcode::OP_GetByte:                      return 0x00B4;
			case script_opcode::OP_GetNegByte:                   return 0x0013;
			case script_opcode::OP_GetUnsignedShort:             return 0x0198;
			case script_opcode::OP_GetNegUnsignedShort:          return 0x01FA;
			case script_opcode::OP_GetInteger:                   return 0x0100;
			case script_opcode::OP_GetFloat:                     return 0x0150;
			case script_opcode::OP_GetString:                    return 0x001C;
			case script_opcode::OP_GetIString:                   return 0x00C9;
			case script_opcode::OP_GetLevelObject:               return 0x0020;
			case script_opcode::OP_GetAnimObject:                return 0x0023;
			case script_opcode::OP_GetSelf:                      return 0x0146;
			case script_opcode::OP_GetLevel:                     return 0x008F;
			case script_opcode::OP_GetGame:                      return 0x01D4;
			case script_opcode::OP_GetAnim:                      return 0x021C;
			case script_opcode::OP_GetAnimation:                 return 0x0420;
			case script_opcode::OP_GetGameRef:                   return 0x0510;
			case script_opcode::OP_GetFunction:                  return 0x0514;
			case script_opcode::OP_SafeCreateLocalVariables:     return 0x01D2;
			case script_opcode::OP_EvalLocalVariableCached:      return 0x0057;
			case script_opcode::OP_EvalArray:                    return 0x00A7;
			case script_opcode::OP_EvalArrayRef:                 return 0x0079;
			case script_opcode::OP_ClearArray:                   return 0x0019;
			case script_opcode::OP_GetEmptyArray:                return 0x00FE;
			case script_opcode::OP_GetSelfObject:                return 0x0031;
			case script_opcode::OP_EvalFieldVariable:            return 0x0084;
			case script_opcode::OP_EvalFieldVariableRef:         return 0x0011;
			case script_opcode::OP_ClearFieldVariable:           return 0x008C;
			case script_opcode::OP_SetWaittillVariableFieldCached: return 0x0126;
			case script_opcode::OP_ClearParams:                  return 0x000C;
			case script_opcode::OP_CheckClearParams:             return 0x000D;
			case script_opcode::OP_EvalLocalVariableRefCached:   return 0x0194;
			case script_opcode::OP_SetVariableField:             return 0x0110;
			case script_opcode::OP_Wait:                         return 0x00D5;
			case script_opcode::OP_WaitTillFrameEnd:             return 0x00F0;
			case script_opcode::OP_PreScriptCall:                return 0x000E;
			case script_opcode::OP_ScriptFunctionCall:           return 0x0076;
			case script_opcode::OP_ScriptFunctionCallPointer:    return 0x003A;
			case script_opcode::OP_ScriptMethodCall:             return 0x001B;
			case script_opcode::OP_ScriptMethodCallPointer:      return 0x0077;
			case script_opcode::OP_ScriptThreadCall:             return 0x01E5;
			case script_opcode::OP_ScriptThreadCallPointer:      return 0x00A0;
			case script_opcode::OP_ScriptMethodThreadCall:       return 0x026E;
			case script_opcode::OP_ScriptMethodThreadCallPointer: return 0x0377;
			case script_opcode::OP_DecTop:                       return 0x0112;
			case script_opcode::OP_CastFieldObject:              return 0x003F;
			case script_opcode::OP_BoolNot:                      return 0x0105;
			case script_opcode::OP_JumpOnFalse:                  return 0x0096;
			case script_opcode::OP_JumpOnTrue:                   return 0x005F;
			case script_opcode::OP_JumpOnFalseExpr:              return 0x00C0;
			case script_opcode::OP_JumpOnTrueExpr:               return 0x00F2;
			case script_opcode::OP_Jump:                         return 0x0022;
			case script_opcode::OP_Inc:                          return 0x0061;
			case script_opcode::OP_Dec:                          return 0x0095;
			case script_opcode::OP_Bit_Or:                       return 0x00AB;
			case script_opcode::OP_Bit_Xor:                      return 0x00AD;
			case script_opcode::OP_Bit_And:                      return 0x020A;
			case script_opcode::OP_Equal:                        return 0x0149;
			case script_opcode::OP_NotEqual:                     return 0x02DA;
			case script_opcode::OP_LessThan:                     return 0x0047;
			case script_opcode::OP_GreaterThan:                  return 0x0049;
			case script_opcode::OP_LessThanOrEqualTo:            return 0x00F6;
			case script_opcode::OP_GreaterThanOrEqualTo:         return 0x01B6;
			case script_opcode::OP_ShiftLeft:                    return 0x0018;
			case script_opcode::OP_ShiftRight:                   return 0x04BD;
			case script_opcode::OP_Plus:                         return 0x0191;
			case script_opcode::OP_Minus:                        return 0x01B7;
			case script_opcode::OP_Multiply:                     return 0x00D9;
			case script_opcode::OP_Divide:                       return 0x01BA;
			case script_opcode::OP_Modulus:                      return 0x04DB;
			case script_opcode::OP_SizeOf:                       return 0x0024;
			case script_opcode::OP_WaitTillMatch:                return 0x04FE;
			case script_opcode::OP_WaitTill:                     return 0x02B2;
			case script_opcode::OP_Notify:                       return 0x0046;
			case script_opcode::OP_EndOn:                        return 0x008B;
			case script_opcode::OP_Switch:                       return 0x006B;
			case script_opcode::OP_EndSwitch:                    return 0x0015;
			case script_opcode::OP_Vector:                       return 0x00B7;
			case script_opcode::OP_GetHash:                      return 0x0108;
			case script_opcode::OP_VectorConstant:               return 0x010E;
			case script_opcode::OP_IsDefined:                    return 0x0070;
			case script_opcode::OP_VectorScale:                  return 0x0103;
			case script_opcode::OP_GetTime:                      return 0x0117;
			case script_opcode::OP_FirstArrayKey:                return 0x00B2;
			case script_opcode::OP_NextArrayKey:                 return 0x0025;
			case script_opcode::OP_DevblockBegin:                return 0x0064;
			case script_opcode::OP_GetObjectType:                return 0x0030;
			case script_opcode::OP_WaitRealTime:                 return 0x0104;
			case script_opcode::OP_GetWorldObject:               return 0x00A2;
			case script_opcode::OP_Bit_Not:                      return 0x007A;
			case script_opcode::OP_GetWorld:                     return 0x0042;
			case script_opcode::OP_EvalLevelFieldVariable:       return 0x0027;
			case script_opcode::OP_EvalLevelFieldVariableRef:    return 0x0242;
			case script_opcode::OP_EvalSelfFieldVariable:        return 0x00CC;
			case script_opcode::OP_EvalSelfFieldVariableRef:     return 0x0109;
			case script_opcode::OP_SuperEqual:                   return 0x006C;
			case script_opcode::OP_SuperNotEqual:                return 0x00DC;
			default:                                             return static_cast<uint16_t>(op);
			}
		}

		// Structures for compilation
		struct hash_name_pair
		{
			uint32_t hash;
			std::string name;
			int line;
			uint8_t params;
		};

		struct string_entry
		{
			std::string value;
			uint32_t offset; // filled during commit
			// Each reference: (export_index, bytecode_offset_within_that_export)
			std::vector<std::pair<size_t, uint32_t>> references;
		};

		struct import_entry
		{
			uint32_t function_hash;
			uint32_t namespace_hash;
			uint8_t num_params;
			uint8_t flags;
			// Each reference: (export_index, bytecode_offset_within_that_export)
			std::vector<std::pair<size_t, uint32_t>> references;
		};

		struct export_entry
		{
			uint32_t function_hash;
			uint32_t namespace_hash;
			uint8_t num_params;
			uint8_t flags;
			std::vector<uint8_t> bytecode;
			uint32_t bytecode_offset; // filled during commit

			std::vector<uint32_t> local_hashes;
			std::unordered_map<uint32_t, uint8_t> local_indices;

			uint8_t add_local(uint32_t hash)
			{
				auto it = local_indices.find(hash);
				if (it != local_indices.end()) return it->second;
				uint8_t idx = static_cast<uint8_t>(local_hashes.size());
				local_hashes.push_back(hash);
				local_indices[hash] = idx;
				return idx;
			}

			uint8_t get_local_index(uint32_t hash) const
			{
				auto it = local_indices.find(hash);
				if (it == local_indices.end()) return 0xFF;
				return it->second;
			}
		};

		// Jump fixup: store where the jump instruction's offset is, and the target label
		struct jump_fixup
		{
			uint32_t offset_location; // where in bytecode the i16 offset is
			uint32_t jump_end;        // end of the jump instruction (where offset is relative from)
			int target_label;         // label ID to jump to
		};

		struct loop_context
		{
			int break_label;
			int continue_label;
		};

		struct emitter_state
		{
			export_entry* current_func;
			size_t current_export_index;
			uint32_t script_namespace;
			std::string script_name;

			std::vector<string_entry> strings;
			std::unordered_map<std::string, size_t> string_map;

			std::vector<import_entry> imports;

			// Include table (stores path strings, NOT hashes)
			std::vector<std::string> includes;

			std::vector<export_entry> exports;

			std::vector<jump_fixup> jump_fixups;
			std::unordered_map<int, uint32_t> label_positions; // label_id -> bytecode offset
			int next_label_id;

			std::vector<loop_context> loop_stack;

			// Temp variable counter for foreach/switch
			int temp_var_counter;

			std::vector<hash_name_pair> hash_names;

			void record_hash(const std::string& name, int line = 0, uint8_t params = 0)
			{
				std::string lower = name;
				std::transform(lower.begin(), lower.end(), lower.begin(),
					[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				uint32_t h = gsc_hash(lower);
				hash_names.push_back({h, lower, line, params});
			}

			emitter_state() : current_func(nullptr), current_export_index(0), script_namespace(0),
				next_label_id(0), temp_var_counter(0) {}

			int new_label() { return next_label_id++; }

			void set_label(int id)
			{
				label_positions[id] = static_cast<uint32_t>(current_func->bytecode.size());
			}

			void emit_op(script_opcode op)
			{
				write_u16(current_func->bytecode, map_opcode(op));
			}

			void emit_u8(uint8_t v) { write_u8(current_func->bytecode, v); }

			void emit_u16_aligned()
			{
				uint32_t pos = static_cast<uint32_t>(current_func->bytecode.size());
				uint32_t aligned = align_value(pos, 2);
				while (current_func->bytecode.size() < aligned)
					current_func->bytecode.push_back(0);
			}

			void emit_u32_aligned()
			{
				uint32_t pos = static_cast<uint32_t>(current_func->bytecode.size());
				uint32_t aligned = align_value(pos, 4);
				while (current_func->bytecode.size() < aligned)
					current_func->bytecode.push_back(0);
			}

			void emit_u16(uint16_t v) { write_u16(current_func->bytecode, v); }
			void emit_u32(uint32_t v) { write_u32(current_func->bytecode, v); }
			void emit_i16(int16_t v) { write_i16(current_func->bytecode, v); }
			void emit_float(float v) { write_float(current_func->bytecode, v); }

			// Emit a jump instruction; returns the fixup record index
			void emit_jump(script_opcode op, int target_label)
			{
				emit_op(op);
				emit_u16_aligned();
				uint32_t offset_loc = static_cast<uint32_t>(current_func->bytecode.size());
				emit_i16(0); // placeholder
				jump_fixups.push_back({offset_loc, offset_loc + 2, target_label});
			}

			size_t add_string(const std::string& str)
			{
				auto it = string_map.find(str);
				if (it != string_map.end()) return it->second;
				size_t idx = strings.size();
				strings.push_back({str, 0, {}});
				string_map[str] = idx;
				return idx;
			}

			void emit_string_ref(script_opcode op, const std::string& str)
			{
				size_t idx = add_string(str);
				emit_op(op);
				emit_u32_aligned();
				// Record reference location (will be patched by game loader via fixup table)
				strings[idx].references.push_back(
					{current_export_index, static_cast<uint32_t>(current_func->bytecode.size())});
				emit_u32(0xFFFFFFFF); // placeholder
			}

			uint64_t make_import_key(uint32_t func, uint32_t ns, uint8_t params, uint8_t flags) const
			{
				return (static_cast<uint64_t>(func) << 32) | (static_cast<uint64_t>(ns) << 16)
					| (static_cast<uint64_t>(params) << 8) | flags;
			}

			size_t add_import(uint32_t func_hash, uint32_t ns_hash, uint8_t num_params, uint8_t flags)
			{
				for (size_t i = 0; i < imports.size(); i++)
				{
					if (imports[i].function_hash == func_hash &&
						imports[i].namespace_hash == ns_hash &&
						imports[i].num_params == num_params &&
						imports[i].flags == flags)
						return i;
				}
				imports.push_back({func_hash, ns_hash, num_params, flags, {}});
				return imports.size() - 1;
			}

			void emit_call(uint32_t func_hash, uint32_t ns_hash, uint8_t num_params, bool is_method,
				bool is_thread, bool same_namespace)
			{
				// Import flags: FunctionFlags | CallFlags (matching reference compiler)
				uint8_t flags = 0;
				if (is_method)
					flags = is_thread ? IMPORT_FUNC_METHOD_THREAD : IMPORT_FUNC_METHOD;
				else
					flags = is_thread ? IMPORT_FUNC_THREAD : IMPORT_FUNC_CALL;
				if (same_namespace)
					flags |= IMPORT_CALL_LOCAL;

				script_opcode op;
				if (is_method)
					op = is_thread ? script_opcode::OP_ScriptMethodThreadCall : script_opcode::OP_ScriptMethodCall;
				else
					op = is_thread ? script_opcode::OP_ScriptThreadCall : script_opcode::OP_ScriptFunctionCall;

				size_t import_idx = add_import(func_hash, ns_hash, num_params, flags);

				// Import ref points to OPCODE START (Count - 2 in reference)
				uint32_t opcode_pos = static_cast<uint32_t>(current_func->bytecode.size());
				emit_op(op);
				imports[import_idx].references.push_back(
					{current_export_index, opcode_pos});

				emit_u8(num_params);
				// NO flags byte in bytecode (flags are in the import table only)
				// Aggressive QWord align: (pos + 8) & ~7, matching reference SetAlignedQWord
				{
					uint32_t pad_pos = static_cast<uint32_t>(current_func->bytecode.size());
					uint32_t aligned = (pad_pos + 8) & ~7u;
					while (current_func->bytecode.size() < aligned)
						current_func->bytecode.push_back(0);
				}
				emit_u32(func_hash); // will be patched by game loader
				emit_u32(0);         // zero padding
			}

			void emit_call_ptr(uint8_t num_params, bool is_method, bool is_thread)
			{
				script_opcode op;
				if (is_method)
					op = is_thread ? script_opcode::OP_ScriptMethodThreadCallPointer : script_opcode::OP_ScriptMethodCallPointer;
				else
					op = is_thread ? script_opcode::OP_ScriptThreadCallPointer : script_opcode::OP_ScriptFunctionCallPointer;

				emit_op(op);
				emit_u16(static_cast<uint16_t>(num_params)); // reference uses AddUshort (2 bytes)
			}

			std::string temp_var_name()
			{
				return "__temp_" + std::to_string(temp_var_counter++);
			}
		};

		// Forward declarations
		void emit_expression(emitter_state& s, const ast_ptr& node);
		void emit_statement(emitter_state& s, const ast_ptr& node);
		void emit_block(emitter_state& s, const ast_ptr& node);
		void emit_lvalue(emitter_state& s, const ast_ptr& node, bool is_ref);

		void pre_register_temps(emitter_state& s, const ast_ptr& node)
		{
			if (!node) return;

			if (node->type == node_type::n_foreach)
			{
				std::string array_temp = s.temp_var_name();
				s.current_func->add_local(gsc_hash(array_temp));

				std::string key_name = node->children[0]->value;
				if (key_name.empty())
				{
					key_name = s.temp_var_name();
					s.current_func->add_local(gsc_hash(key_name));
				}
			}
			else if (node->type == node_type::n_switch)
			{
				std::string switch_temp = s.temp_var_name();
				s.current_func->add_local(gsc_hash(switch_temp));
			}

			for (auto& child : node->children)
				pre_register_temps(s, child);
		}

		void collect_locals(const ast_ptr& node, std::vector<std::string>& locals,
			const std::vector<std::string>& params)
		{
			if (!node) return;

			if (node->type == node_type::n_assign)
			{
				if (node->children.size() > 0 && node->children[0]->type == node_type::n_identifier)
				{
					const std::string& name = node->children[0]->value;
					bool is_param = std::find(params.begin(), params.end(), name) != params.end();
					bool already = std::find(locals.begin(), locals.end(), name) != locals.end();
					if (!is_param && !already)
						locals.push_back(name);
				}
			}

			if (node->type == node_type::n_foreach)
			{
				const std::string& val_name = node->value;
				bool is_param = std::find(params.begin(), params.end(), val_name) != params.end();
				bool already = std::find(locals.begin(), locals.end(), val_name) != locals.end();
				if (!is_param && !already)
					locals.push_back(val_name);

				if (node->children.size() > 0 && !node->children[0]->value.empty())
				{
					const std::string& key_name = node->children[0]->value;
					is_param = std::find(params.begin(), params.end(), key_name) != params.end();
					already = std::find(locals.begin(), locals.end(), key_name) != locals.end();
					if (!is_param && !already)
						locals.push_back(key_name);
				}
			}

			if (node->type == node_type::n_waittill && node->children.size() > 1)
			{
				auto& args = node->children[1]; // args block
				for (size_t i = 1; i < args->children.size(); i++) // skip first (event name)
				{
					if (args->children[i]->type == node_type::n_identifier)
					{
						const std::string& name = args->children[i]->value;
						bool is_param = std::find(params.begin(), params.end(), name) != params.end();
						bool already = std::find(locals.begin(), locals.end(), name) != locals.end();
						if (!is_param && !already)
							locals.push_back(name);
					}
				}
			}

			for (auto& child : node->children)
				collect_locals(child, locals, params);
		}

		bool is_builtin(const std::string& name)
		{
			std::string lower = name;
			std::transform(lower.begin(), lower.end(), lower.begin(),
				[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return lower == "isdefined" || lower == "vectorscale" || lower == "gettime"
				|| lower == "firstarraykey" || lower == "nextarraykey";
		}

		bool try_emit_builtin(emitter_state& s, const std::string& name)
		{
			std::string lower = name;
			std::transform(lower.begin(), lower.end(), lower.begin(),
				[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

			if (lower == "isdefined") { s.emit_op(script_opcode::OP_IsDefined); return true; }
			if (lower == "vectorscale") { s.emit_op(script_opcode::OP_VectorScale); return true; }
			if (lower == "gettime") { s.emit_op(script_opcode::OP_GetTime); return true; }
			if (lower == "firstarraykey") { s.emit_op(script_opcode::OP_FirstArrayKey); return true; }
			if (lower == "nextarraykey") { s.emit_op(script_opcode::OP_NextArrayKey); return true; }
			return false;
		}

		void emit_get_number(emitter_state& s, int64_t value)
		{
			if (value == 0)
			{
				s.emit_op(script_opcode::OP_GetZero);
			}
			else if (value > 0 && value <= 255)
			{
				s.emit_op(script_opcode::OP_GetByte);
				s.emit_u16(static_cast<uint16_t>(value)); // reference uses AddUshort (2 bytes)
			}
			else if (value < 0 && value >= -255)
			{
				s.emit_op(script_opcode::OP_GetNegByte);
				s.emit_u16(static_cast<uint16_t>(-value)); // reference uses AddUshort (2 bytes)
			}
			else if (value > 0 && value <= 65535)
			{
				s.emit_op(script_opcode::OP_GetUnsignedShort);
				s.emit_u16_aligned();
				s.emit_u16(static_cast<uint16_t>(value));
			}
			else if (value < 0 && value >= -65535)
			{
				s.emit_op(script_opcode::OP_GetNegUnsignedShort);
				s.emit_u16_aligned();
				s.emit_u16(static_cast<uint16_t>(-value));
			}
			else
			{
				s.emit_op(script_opcode::OP_GetInteger);
				s.emit_u32_aligned();
				s.emit_u32(static_cast<uint32_t>(value));
			}
		}

		void emit_eval_local(emitter_state& s, const std::string& name, bool is_ref, bool is_waittill = false)
		{
			std::string lower = name;
			std::transform(lower.begin(), lower.end(), lower.begin(),
				[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

			uint32_t hash = gsc_hash(lower);
			uint8_t idx = s.current_func->get_local_index(hash);
			if (idx == 0xFF)
			{
				idx = s.current_func->add_local(hash);
			}

			if (is_waittill)
			{
				s.emit_op(script_opcode::OP_SetWaittillVariableFieldCached);
				s.emit_u16(static_cast<uint16_t>(idx)); // reference uses AddUshort
			}
			else if (is_ref)
			{
				s.emit_op(script_opcode::OP_EvalLocalVariableRefCached);
				s.emit_u16(static_cast<uint16_t>(idx)); // reference uses AddUshort
			}
			else
			{
				s.emit_op(script_opcode::OP_EvalLocalVariableCached);
				s.emit_u16(static_cast<uint16_t>(idx)); // reference uses AddUshort
			}
		}

		// Emit code to push an object reference for field/method operations (CastFieldObject for unknowns)
		void emit_object(emitter_state& s, const ast_ptr& node)
		{
			if (node->type == node_type::n_self)
				s.emit_op(script_opcode::OP_GetSelfObject);
			else if (node->type == node_type::n_level)
				s.emit_op(script_opcode::OP_GetLevelObject);
			else if (node->type == node_type::n_world)
				s.emit_op(script_opcode::OP_GetWorldObject);
			else if (node->type == node_type::n_anim)
				s.emit_op(script_opcode::OP_GetAnimObject);
			else
			{
				emit_expression(s, node);
				s.emit_op(script_opcode::OP_CastFieldObject);
			}
		}

		// Emit owner value for endon/notify/waittill/method calls (NOT the Object variant)
		void emit_owner(emitter_state& s, const ast_ptr& node)
		{
			if (node->type == node_type::n_self)
				s.emit_op(script_opcode::OP_GetSelf);
			else if (node->type == node_type::n_level)
				s.emit_op(script_opcode::OP_GetLevel);
			else if (node->type == node_type::n_world)
				s.emit_op(script_opcode::OP_GetWorld);
			else if (node->type == node_type::n_anim)
				s.emit_op(script_opcode::OP_GetAnim);
			else if (node->type == node_type::n_game)
				s.emit_op(script_opcode::OP_GetGame);
			else
				emit_expression(s, node);
		}

		void emit_expression(emitter_state& s, const ast_ptr& node)
		{
			if (!node) return;

			switch (node->type)
			{
			case node_type::n_number:
			{
				int64_t val = 0;
				const std::string& v = node->value;
				if (v.size() > 2 && v[0] == '0' && (v[1] == 'x' || v[1] == 'X'))
					val = std::stoll(v, nullptr, 16);
				else
					val = std::stoll(v);
				emit_get_number(s, val);
				break;
			}
			case node_type::n_float_number:
			{
				float fval = std::stof(node->value);
				s.emit_op(script_opcode::OP_GetFloat);
				s.emit_u32_aligned();
				s.emit_float(fval);
				break;
			}
			case node_type::n_string:
				s.emit_string_ref(script_opcode::OP_GetString, node->value);
				break;
			case node_type::n_istring:
				s.emit_string_ref(script_opcode::OP_GetIString, node->value);
				break;
			case node_type::n_hash_string:
			{
				uint32_t hash = gsc_hash(node->value);
				s.emit_op(script_opcode::OP_GetHash);
				s.emit_u32_aligned();
				s.emit_u32(hash);
				break;
			}
			case node_type::n_true_val:
				emit_get_number(s, 1);
				break;
			case node_type::n_false_val:
				s.emit_op(script_opcode::OP_GetZero);
				break;
			case node_type::n_undefined:
				s.emit_op(script_opcode::OP_GetUndefined);
				break;
			case node_type::n_self:
				s.emit_op(script_opcode::OP_GetSelf);
				break;
			case node_type::n_level:
				s.emit_op(script_opcode::OP_GetLevel);
				break;
			case node_type::n_game:
				s.emit_op(script_opcode::OP_GetGame);
				break;
			case node_type::n_anim:
				s.emit_op(script_opcode::OP_GetAnim);
				break;
			case node_type::n_world:
				s.emit_op(script_opcode::OP_GetWorld);
				break;
			case node_type::n_empty_array:
				s.emit_op(script_opcode::OP_GetEmptyArray);
				break;

			case node_type::n_identifier:
			{
				std::string lower = node->value;
				std::transform(lower.begin(), lower.end(), lower.begin(),
					[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				emit_eval_local(s, lower, false);
				break;
			}

			case node_type::n_field_access:
			{
				// children[0] = object
				emit_object(s, node->children[0]);
				uint32_t field_hash = gsc_hash(node->value);
				s.emit_op(script_opcode::OP_EvalFieldVariable);
				s.emit_u32_aligned();
				s.emit_u32(field_hash);
				break;
			}

			case node_type::n_array_access:
			{
				// children[0] = array, children[1] = key
				emit_expression(s, node->children[1]); // key first
				emit_expression(s, node->children[0]); // then array
				s.emit_op(script_opcode::OP_EvalArray);
				break;
			}

			case node_type::n_size:
			{
				emit_expression(s, node->children[0]);
				s.emit_op(script_opcode::OP_SizeOf);
				break;
			}

			case node_type::n_vector:
			{
				// Reference pushes z, y, x (reverse order)
				emit_expression(s, node->children[2]); // z
				emit_expression(s, node->children[1]); // y
				emit_expression(s, node->children[0]); // x
				s.emit_op(script_opcode::OP_Vector);
				break;
			}

			case node_type::n_ternary:
			{
				int false_label = s.new_label();
				int end_label = s.new_label();
				emit_expression(s, node->children[0]); // condition
				s.emit_jump(script_opcode::OP_JumpOnFalseExpr, false_label);
				emit_expression(s, node->children[1]); // true value
				s.emit_jump(script_opcode::OP_Jump, end_label);
				s.set_label(false_label);
				emit_expression(s, node->children[2]); // false value
				s.set_label(end_label);
				break;
			}

			case node_type::n_binary_op:
			{
				emit_expression(s, node->children[0]); // left

				// Short-circuit for && and ||
				if (node->value == "&&")
				{
					int skip_label = s.new_label();
					s.emit_jump(script_opcode::OP_JumpOnFalseExpr, skip_label);
					emit_expression(s, node->children[1]);
					s.set_label(skip_label);
					break;
				}
				if (node->value == "||")
				{
					int skip_label = s.new_label();
					s.emit_jump(script_opcode::OP_JumpOnTrueExpr, skip_label);
					emit_expression(s, node->children[1]);
					s.set_label(skip_label);
					break;
				}

				emit_expression(s, node->children[1]); // right

				if (node->value == "+") s.emit_op(script_opcode::OP_Plus);
				else if (node->value == "-") s.emit_op(script_opcode::OP_Minus);
				else if (node->value == "*") s.emit_op(script_opcode::OP_Multiply);
				else if (node->value == "/") s.emit_op(script_opcode::OP_Divide);
				else if (node->value == "%") s.emit_op(script_opcode::OP_Modulus);
				else if (node->value == "&") s.emit_op(script_opcode::OP_Bit_And);
				else if (node->value == "|") s.emit_op(script_opcode::OP_Bit_Or);
				else if (node->value == "^") s.emit_op(script_opcode::OP_Bit_Xor);
				else if (node->value == "<<") s.emit_op(script_opcode::OP_ShiftLeft);
				else if (node->value == ">>") s.emit_op(script_opcode::OP_ShiftRight);
				else if (node->value == "==") s.emit_op(script_opcode::OP_Equal);
				else if (node->value == "!=") s.emit_op(script_opcode::OP_NotEqual);
				else if (node->value == "<") s.emit_op(script_opcode::OP_LessThan);
				else if (node->value == ">") s.emit_op(script_opcode::OP_GreaterThan);
				else if (node->value == "<=") s.emit_op(script_opcode::OP_LessThanOrEqualTo);
				else if (node->value == ">=") s.emit_op(script_opcode::OP_GreaterThanOrEqualTo);
				else if (node->value == "===") s.emit_op(script_opcode::OP_SuperEqual);
				else if (node->value == "!==") s.emit_op(script_opcode::OP_SuperNotEqual);
				else throw std::runtime_error("Unknown binary operator: " + node->value);
				break;
			}

			case node_type::n_unary_op:
			{
				emit_expression(s, node->children[0]);
				if (node->value == "!") s.emit_op(script_opcode::OP_BoolNot);
				else if (node->value == "~") s.emit_op(script_opcode::OP_Bit_Not);
				else if (node->value == "-")
				{
					// Negate: push -1 and multiply
					emit_get_number(s, -1);
					s.emit_op(script_opcode::OP_Multiply);
				}
				break;
			}

			case node_type::n_func_ref:
			{
				// ::func or ns::func -> GetFunction
				uint32_t func_hash = gsc_hash(node->value);
				uint32_t ns_hash = s.script_namespace;
				if (!node->children.empty() && !node->children[0]->value.empty())
					ns_hash = gsc_hash(node->children[0]->value);

				// Import flags: GetFunction | CallFlags
				uint8_t flags = IMPORT_FUNC_GETFUNCTION;
				if (ns_hash == s.script_namespace)
					flags |= IMPORT_CALL_LOCAL;

				size_t import_idx = s.add_import(func_hash, ns_hash, 0, flags);

				// Import ref points to OPCODE START
				uint32_t opcode_pos = static_cast<uint32_t>(s.current_func->bytecode.size());
				s.emit_op(script_opcode::OP_GetFunction);
				s.imports[import_idx].references.push_back(
					{s.current_export_index, opcode_pos});
				{
					uint32_t pad_pos = static_cast<uint32_t>(s.current_func->bytecode.size());
					uint32_t aligned = static_cast<uint32_t>(align_value64(pad_pos, 8));
					while (s.current_func->bytecode.size() < aligned)
						s.current_func->bytecode.push_back(0);
				}
				s.emit_u32(func_hash);
				s.emit_u32(0); // zero padding
				break;
			}

			case node_type::n_call:
			{
				// children[0] = namespace node, children[1] = args
				std::string func_name = node->value;
				std::string lower_name = func_name;
				std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),
					[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

				auto& ns_node = node->children[0];
				auto& args_node = node->children[1];
				uint8_t num_params = static_cast<uint8_t>(args_node->children.size());

				if (ns_node->value.empty() && is_builtin(lower_name))
				{
					for (auto& arg : args_node->children)
						emit_expression(s, arg);
					try_emit_builtin(s, lower_name);
					break;
				}

				s.emit_op(script_opcode::OP_PreScriptCall);

				for (int i = static_cast<int>(args_node->children.size()) - 1; i >= 0; i--)
					emit_expression(s, args_node->children[i]);

				uint32_t func_hash = gsc_hash(lower_name);
				bool has_explicit_ns = !ns_node->value.empty();
				uint32_t ns_hash = has_explicit_ns ? gsc_hash(ns_node->value) : s.script_namespace;

				bool is_local = !has_explicit_ns;

				s.record_hash(lower_name, node->line, num_params);
				s.emit_call(func_hash, ns_hash, num_params, false, false, is_local);
				break;
			}

			case node_type::n_method_call:
			{
				// children[0] = object, children[1] = args, children[2] = namespace (optional)
				auto& obj = node->children[0];
				auto& args_node = node->children[1];
				uint8_t num_params = static_cast<uint8_t>(args_node->children.size());

				std::string func_name = node->value;
				std::string lower_name = func_name;
				std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),
					[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

				s.emit_op(script_opcode::OP_PreScriptCall);

				for (int i = static_cast<int>(args_node->children.size()) - 1; i >= 0; i--)
					emit_expression(s, args_node->children[i]);

				emit_expression(s, obj);

				uint32_t func_hash = gsc_hash(lower_name);
				bool has_explicit_ns = (node->children.size() > 2 && !node->children[2]->value.empty());
				uint32_t ns_hash = has_explicit_ns ? gsc_hash(node->children[2]->value) : s.script_namespace;

				bool is_local = !has_explicit_ns;

				s.record_hash(lower_name, node->line, num_params);
				s.emit_call(func_hash, ns_hash, num_params, true, false, is_local);
				break;
			}

			case node_type::n_call_ptr:
			{
				// children[0] = caller/nullptr, children[1] = func ptr expr, children[2] = args
				auto& args_node = node->children[2];
				uint8_t num_params = static_cast<uint8_t>(args_node->children.size());
				bool has_caller = node->children[0]->type != node_type::n_undefined;

				s.emit_op(script_opcode::OP_PreScriptCall);

				for (int i = static_cast<int>(args_node->children.size()) - 1; i >= 0; i--)
					emit_expression(s, args_node->children[i]);

				if (has_caller)
					emit_expression(s, node->children[0]);

				emit_expression(s, node->children[1]); // func ptr
				s.emit_call_ptr(num_params, has_caller, false);
				break;
			}

			case node_type::n_thread_call:
			{
				// children[0] = actual call/method_call
				auto& inner = node->children[0];
				if (inner->type == node_type::n_call)
				{
					auto& ns_node = inner->children[0];
					auto& args_node = inner->children[1];
					uint8_t num_params = static_cast<uint8_t>(args_node->children.size());

					std::string lower_name = inner->value;
					std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),
						[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

					s.emit_op(script_opcode::OP_PreScriptCall);
					for (int i = static_cast<int>(args_node->children.size()) - 1; i >= 0; i--)
						emit_expression(s, args_node->children[i]);

					uint32_t func_hash = gsc_hash(lower_name);
					bool has_explicit_ns = !ns_node->value.empty();
					uint32_t ns_hash = has_explicit_ns ? gsc_hash(ns_node->value) : s.script_namespace;
					bool is_local = !has_explicit_ns;
					s.record_hash(lower_name, inner->line, num_params);
					s.emit_call(func_hash, ns_hash, num_params, false, true, is_local);
				}
				else if (inner->type == node_type::n_method_call)
				{
					auto& obj = inner->children[0];
					auto& args_node = inner->children[1];
					uint8_t num_params = static_cast<uint8_t>(args_node->children.size());

					std::string lower_name = inner->value;
					std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),
						[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

					s.emit_op(script_opcode::OP_PreScriptCall);
					for (int i = static_cast<int>(args_node->children.size()) - 1; i >= 0; i--)
						emit_expression(s, args_node->children[i]);

					emit_expression(s, obj);

					uint32_t func_hash = gsc_hash(lower_name);
					bool has_ns = (inner->children.size() > 2 && !inner->children[2]->value.empty());
					uint32_t ns_hash = has_ns ? gsc_hash(inner->children[2]->value) : s.script_namespace;
					bool is_local = !has_ns;
					s.record_hash(lower_name, inner->line, num_params);
					s.emit_call(func_hash, ns_hash, num_params, true, true, is_local);
				}
				else if (inner->type == node_type::n_call_ptr)
				{
					auto& args_node = inner->children[2];
					uint8_t num_params = static_cast<uint8_t>(args_node->children.size());
					bool has_caller = inner->children[0]->type != node_type::n_undefined;

					s.emit_op(script_opcode::OP_PreScriptCall);
					for (int i = static_cast<int>(args_node->children.size()) - 1; i >= 0; i--)
						emit_expression(s, args_node->children[i]);

					if (has_caller)
						emit_expression(s, inner->children[0]);

					emit_expression(s, inner->children[1]);
					s.emit_call_ptr(num_params, has_caller, true);
				}
				break;
			}

			case node_type::n_inc_dec:
			{
				emit_lvalue(s, node->children[0], true);
				if (node->value == "post++" || node->value == "pre++")
					s.emit_op(script_opcode::OP_Inc);
				else
					s.emit_op(script_opcode::OP_Dec);
				break;
			}

			default:
				throw std::runtime_error("Cannot emit expression for node type "
					+ std::to_string(static_cast<int>(node->type))
					+ " at line " + std::to_string(node->line));
			}
		}

		void emit_lvalue(emitter_state& s, const ast_ptr& node, bool is_ref)
		{
			switch (node->type)
			{
			case node_type::n_identifier:
			{
				std::string lower = node->value;
				std::transform(lower.begin(), lower.end(), lower.begin(),
					[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				emit_eval_local(s, lower, is_ref);
				break;
			}
			case node_type::n_field_access:
			{
				emit_object(s, node->children[0]);
				uint32_t field_hash = gsc_hash(node->value);
				if (is_ref)
				{
					s.emit_op(script_opcode::OP_EvalFieldVariableRef);
					s.emit_u32_aligned();
					s.emit_u32(field_hash);
				}
				else
				{
					s.emit_op(script_opcode::OP_EvalFieldVariable);
					s.emit_u32_aligned();
					s.emit_u32(field_hash);
				}
				break;
			}
			case node_type::n_array_access:
			{
				emit_expression(s, node->children[1]); // key
				emit_lvalue(s, node->children[0], true); // array ref
				s.emit_op(script_opcode::OP_EvalArrayRef);
				break;
			}
			default:
				emit_expression(s, node);
				break;
			}
		}

		void emit_statement(emitter_state& s, const ast_ptr& node)
		{
			if (!node) return;

			switch (node->type)
			{
			case node_type::n_block:
				emit_block(s, node);
				break;

			case node_type::n_expression_stmt:
			{
				auto& expr = node->children[0];
				if (expr->type == node_type::n_endon || expr->type == node_type::n_notify
					|| expr->type == node_type::n_waittill || expr->type == node_type::n_assign
					|| expr->type == node_type::n_inc_dec)
				{
					emit_statement(s, expr);
					break;
				}
				emit_expression(s, expr);
				if (expr->type == node_type::n_call || expr->type == node_type::n_method_call
					|| expr->type == node_type::n_call_ptr || expr->type == node_type::n_thread_call)
				{
					s.emit_op(script_opcode::OP_DecTop);
				}
				break;
			}

			case node_type::n_assign:
			{
				auto& target = node->children[0];
				auto& value = node->children[1];
				const std::string& op = node->value;

				if (op == "=")
				{
					emit_expression(s, value);
				}
				else
				{
					emit_expression(s, target);
					emit_expression(s, value);
					if (op == "+=") s.emit_op(script_opcode::OP_Plus);
					else if (op == "-=") s.emit_op(script_opcode::OP_Minus);
					else if (op == "*=") s.emit_op(script_opcode::OP_Multiply);
					else if (op == "/=") s.emit_op(script_opcode::OP_Divide);
					else if (op == "%=") s.emit_op(script_opcode::OP_Modulus);
					else if (op == "&=") s.emit_op(script_opcode::OP_Bit_And);
					else if (op == "|=") s.emit_op(script_opcode::OP_Bit_Or);
					else if (op == "^=") s.emit_op(script_opcode::OP_Bit_Xor);
					else if (op == "<<=") s.emit_op(script_opcode::OP_ShiftLeft);
					else if (op == ">>=") s.emit_op(script_opcode::OP_ShiftRight);
				}

				emit_lvalue(s, target, true);
				s.emit_op(script_opcode::OP_SetVariableField);
				break;
			}

			case node_type::n_inc_dec:
			{
				emit_lvalue(s, node->children[0], true);
				if (node->value == "post++" || node->value == "pre++")
					s.emit_op(script_opcode::OP_Inc);
				else
					s.emit_op(script_opcode::OP_Dec);
				break;
			}

			case node_type::n_return:
			{
				if (!node->children.empty())
				{
					emit_expression(s, node->children[0]);
					s.emit_op(script_opcode::OP_Return);
				}
				else
				{
					s.emit_op(script_opcode::OP_End);
				}
				break;
			}

			case node_type::n_wait:
			{
				emit_expression(s, node->children[0]);
				s.emit_op(script_opcode::OP_Wait);
				break;
			}

			case node_type::n_waittillframeend:
				s.emit_op(script_opcode::OP_WaitTillFrameEnd);
				break;

			case node_type::n_waitrealtime:
			{
				emit_expression(s, node->children[0]);
				s.emit_op(script_opcode::OP_WaitRealTime);
				break;
			}

			case node_type::n_if:
			{
				int else_label = s.new_label();
				int end_label = s.new_label();

				emit_expression(s, node->children[0]); // condition
				s.emit_jump(script_opcode::OP_JumpOnFalse, else_label);

				emit_statement(s, node->children[1]); // if body

				if (node->children.size() > 2)
				{
					s.emit_jump(script_opcode::OP_Jump, end_label);
					s.set_label(else_label);
					emit_statement(s, node->children[2]); // else body
					s.set_label(end_label);
				}
				else
				{
					s.set_label(else_label);
				}
				break;
			}

			case node_type::n_while:
			{
				int loop_start = s.new_label();
				int loop_end = s.new_label();
				int loop_continue = loop_start;

				s.loop_stack.push_back({loop_end, loop_continue});

				s.set_label(loop_start);
				emit_expression(s, node->children[0]); // condition
				s.emit_jump(script_opcode::OP_JumpOnFalse, loop_end);
				emit_statement(s, node->children[1]); // body
				s.emit_jump(script_opcode::OP_Jump, loop_start);
				s.set_label(loop_end);

				s.loop_stack.pop_back();
				break;
			}

			case node_type::n_do_while:
			{
				int loop_start = s.new_label();
				int loop_end = s.new_label();
				int loop_continue = s.new_label();

				s.loop_stack.push_back({loop_end, loop_continue});

				s.set_label(loop_start);
				emit_statement(s, node->children[1]); // body
				s.set_label(loop_continue);
				emit_expression(s, node->children[0]); // condition
				s.emit_jump(script_opcode::OP_JumpOnTrue, loop_start);
				s.set_label(loop_end);

				s.loop_stack.pop_back();
				break;
			}

			case node_type::n_for:
			{
				// children: [0]=init, [1]=cond, [2]=iter, [3]=body
				int loop_cond = s.new_label();
				int loop_end = s.new_label();
				int loop_continue = s.new_label();

				s.loop_stack.push_back({loop_end, loop_continue});

				if (node->children[0]->type != node_type::n_undefined)
					emit_statement(s, node->children[0]);

				s.set_label(loop_cond);

				if (node->children[1]->type != node_type::n_true_val)
				{
					emit_expression(s, node->children[1]);
					s.emit_jump(script_opcode::OP_JumpOnFalse, loop_end);
				}

				emit_statement(s, node->children[3]);

				s.set_label(loop_continue);
				if (node->children[2]->type != node_type::n_undefined)
					emit_statement(s, node->children[2]);

				s.emit_jump(script_opcode::OP_Jump, loop_cond);
				s.set_label(loop_end);

				s.loop_stack.pop_back();
				break;
			}

			case node_type::n_foreach:
			{
				// value = node->value, children[0] = key name, [1] = array, [2] = body
				std::string val_name = node->value;
				std::string key_name = node->children[0]->value;
				bool has_key = !key_name.empty();

				std::string array_temp = s.temp_var_name();
				if (!has_key) key_name = s.temp_var_name();

				s.current_func->add_local(gsc_hash(array_temp));
				s.current_func->add_local(gsc_hash(key_name));
				s.current_func->add_local(gsc_hash(val_name));

				// array_temp = <array_expr>
				emit_expression(s, node->children[1]);
				emit_eval_local(s, array_temp, true);
				s.emit_op(script_opcode::OP_SetVariableField);

				// key = firstArrayKey(array_temp)
				emit_eval_local(s, array_temp, false);
				s.emit_op(script_opcode::OP_FirstArrayKey);
				emit_eval_local(s, key_name, true);
				s.emit_op(script_opcode::OP_SetVariableField);

				int loop_start = s.new_label();
				int loop_end = s.new_label();
				int loop_continue = s.new_label();

				s.loop_stack.push_back({loop_end, loop_continue});

				s.set_label(loop_start);

				// if (!isDefined(key)) break
				emit_eval_local(s, key_name, false);
				s.emit_op(script_opcode::OP_IsDefined);
				s.emit_jump(script_opcode::OP_JumpOnFalse, loop_end);

				// val = array_temp[key]
				emit_eval_local(s, key_name, false);
				emit_eval_local(s, array_temp, false);
				s.emit_op(script_opcode::OP_EvalArray);
				emit_eval_local(s, val_name, true);
				s.emit_op(script_opcode::OP_SetVariableField);

				emit_statement(s, node->children[2]);

				s.set_label(loop_continue);

				// key = nextArrayKey(array_temp, key)
				emit_eval_local(s, key_name, false);
				emit_eval_local(s, array_temp, false);
				s.emit_op(script_opcode::OP_NextArrayKey);
				emit_eval_local(s, key_name, true);
				s.emit_op(script_opcode::OP_SetVariableField);

				s.emit_jump(script_opcode::OP_Jump, loop_start);
				s.set_label(loop_end);

				s.loop_stack.pop_back();
				break;
			}

			case node_type::n_switch:
			{
				// children[0] = expr, children[1..] = cases
				std::string switch_temp = s.temp_var_name();
				s.current_func->add_local(gsc_hash(switch_temp));

				emit_expression(s, node->children[0]);
				emit_eval_local(s, switch_temp, true);
				s.emit_op(script_opcode::OP_SetVariableField);

				int switch_end = s.new_label();
				s.loop_stack.push_back({switch_end, -1}); // break goes to switch_end

				// Emit cases as if-else chain
				std::vector<int> case_labels;
				int default_label = -1;

				for (size_t i = 1; i < node->children.size(); i++)
				{
					auto& case_node = node->children[i];
					int label = s.new_label();
					case_labels.push_back(label);

					if (case_node->type == node_type::n_default_case)
					{
						default_label = label;
					}
					else
					{
						// Compare switch_temp == case_value
						emit_expression(s, case_node->children[0]);
						emit_eval_local(s, switch_temp, false);
						s.emit_op(script_opcode::OP_Equal);
						s.emit_jump(script_opcode::OP_JumpOnTrue, label);
					}
				}

				if (default_label >= 0)
					s.emit_jump(script_opcode::OP_Jump, default_label);
				else
					s.emit_jump(script_opcode::OP_Jump, switch_end);

				for (size_t i = 1; i < node->children.size(); i++)
				{
					s.set_label(case_labels[i - 1]);
					auto& case_node = node->children[i];

					if (case_node->type == node_type::n_case)
					{
						if (case_node->children.size() > 1)
							emit_statement(s, case_node->children[1]); // body
					}
					else // default
					{
						if (!case_node->children.empty())
							emit_statement(s, case_node->children[0]); // body
					}
				}

				s.set_label(switch_end);
				s.loop_stack.pop_back();
				break;
			}

			case node_type::n_break:
			{
				if (s.loop_stack.empty())
					throw std::runtime_error("'break' outside of loop at line " + std::to_string(node->line));
				s.emit_jump(script_opcode::OP_Jump, s.loop_stack.back().break_label);
				break;
			}

			case node_type::n_continue:
			{
				if (s.loop_stack.empty())
					throw std::runtime_error("'continue' outside of loop at line " + std::to_string(node->line));
				s.emit_jump(script_opcode::OP_Jump, s.loop_stack.back().continue_label);
				break;
			}

			case node_type::n_waittill:
			{
				// children[0] = object, children[1] = args (first is event name, rest are vars)
				auto& obj = node->children[0];
				auto& args = node->children[1];

				if (args->children.empty())
					throw std::runtime_error("waittill requires at least an event name at line " + std::to_string(node->line));

				emit_expression(s, args->children[0]); // event name
				emit_owner(s, obj); // object (uses GetLevel, not GetLevelObject)
				s.emit_op(script_opcode::OP_WaitTill);

				for (size_t i = 1; i < args->children.size(); i++)
				{
					if (args->children[i]->type == node_type::n_identifier)
					{
						emit_eval_local(s, args->children[i]->value, false, true);
					}
				}
				s.emit_op(script_opcode::OP_ClearParams);
				break;
			}

			case node_type::n_notify:
			{
				// children[0] = object, children[1] = args (first is event, rest are params)
				auto& obj = node->children[0];
				auto& args = node->children[1];

				s.emit_op(script_opcode::OP_PreScriptCall);

				for (int i = static_cast<int>(args->children.size()) - 1; i >= 1; i--)
					emit_expression(s, args->children[i]);

				emit_expression(s, args->children[0]);
				emit_owner(s, obj);
				s.emit_op(script_opcode::OP_Notify);
				break;
			}

			case node_type::n_endon:
			{
				// children[0] = object, children[1] = args (event name)
				auto& obj = node->children[0];
				auto& args = node->children[1];

				if (!args->children.empty())
					emit_expression(s, args->children[0]); // event name
				emit_owner(s, obj);
				s.emit_op(script_opcode::OP_EndOn);
				break;
			}

			case node_type::n_call:
			case node_type::n_method_call:
			case node_type::n_call_ptr:
			case node_type::n_thread_call:
				emit_expression(s, node);
				s.emit_op(script_opcode::OP_DecTop);
				break;

			default:
				emit_expression(s, node);
				break;
			}
		}

		void emit_block(emitter_state& s, const ast_ptr& node)
		{
			if (!node) return;
			for (auto& child : node->children)
				emit_statement(s, child);
		}

		void emit_function(emitter_state& s, const ast_ptr& node)
		{
			// node->value = function name
			// children[0] = autoexec flag, [1] = params, [2] = body
			std::string func_name = node->value;
			std::string lower_name = func_name;
			std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),
				[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

			auto& flags_node = node->children[0];
			auto& params_node = node->children[1];
			auto& body_node = node->children[2];

			export_entry exp{};
			exp.function_hash = gsc_hash(lower_name);
			exp.namespace_hash = s.script_namespace;
			exp.num_params = static_cast<uint8_t>(params_node->children.size());
			exp.flags = EXPORT_NONE;
			const std::string& flags_str = flags_node->value;
			if (flags_str.find("autoexec") != std::string::npos)
				exp.flags |= EXPORT_AUTOEXEC;
			if (flags_str.find("private") != std::string::npos)
				exp.flags |= EXPORT_PRIVATE;

			s.exports.push_back(std::move(exp));
			s.current_export_index = s.exports.size() - 1;
			s.current_func = &s.exports.back();

			s.jump_fixups.clear();
			s.label_positions.clear();
			s.loop_stack.clear();

			std::vector<std::string> param_names;
			for (auto& param : params_node->children)
			{
				std::string pname = param->value;
				std::transform(pname.begin(), pname.end(), pname.begin(),
					[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				s.current_func->add_local(gsc_hash(pname));
				param_names.push_back(pname);
			}

			std::vector<std::string> locals;
			collect_locals(body_node, locals, param_names);

			for (auto& local : locals)
			{
				std::string lower = local;
				std::transform(lower.begin(), lower.end(), lower.begin(),
					[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				s.current_func->add_local(gsc_hash(lower));
			}

			int saved_temp = s.temp_var_counter;
			pre_register_temps(s, body_node);
			s.temp_var_counter = saved_temp;

			if (s.current_func->local_hashes.empty())
			{
				s.emit_op(script_opcode::OP_CheckClearParams);
			}
			else
			{
				s.emit_op(script_opcode::OP_SafeCreateLocalVariables);
				s.emit_u8(static_cast<uint8_t>(s.current_func->local_hashes.size()));
				for (size_t vi = 0; vi < s.current_func->local_hashes.size(); vi++)
				{
					s.emit_u32_aligned();
					s.emit_u32(s.current_func->local_hashes[vi]); // hash
					s.emit_u8(0); // null terminator after each hash
				}
				s.emit_u8(0); // final null byte (no CheckClearParams!)

				// Reverse variable index mapping (last declared = index 0)
				uint8_t N = static_cast<uint8_t>(s.current_func->local_hashes.size());
				for (auto& [hash, idx] : s.current_func->local_indices)
					idx = N - 1 - idx;
			}

			emit_block(s, body_node);

			s.emit_op(script_opcode::OP_End);

			for (auto& fixup : s.jump_fixups)
			{
				auto it = s.label_positions.find(fixup.target_label);
				if (it == s.label_positions.end())
				{
					throw std::runtime_error("Unresolved jump label in function '"
						+ func_name + "'");
				}

				int16_t offset = static_cast<int16_t>(
					static_cast<int32_t>(it->second) - static_cast<int32_t>(fixup.jump_end));
				write_at_i16(s.current_func->bytecode, fixup.offset_location, offset);
			}
		}

		uint32_t crc32_calc(const uint8_t* data, size_t len)
		{
			uint32_t crc = 0xFFFFFFFF;
			for (size_t i = 0; i < len; i++)
			{
				crc ^= data[i];
				for (int j = 0; j < 8; j++)
					crc = (crc >> 1) ^ (0xEDB88320 & (0u - (crc & 1)));
			}
			return ~crc;
		}

		// Final binary assembly — section order matches reference compiler exactly:
		// Header → IncludeStrings+Name+CodeStrings → IncludeTable → Code → Exports → Imports → AnimTrees → StringTable
		std::vector<uint8_t> assemble(emitter_state& s)
		{
			std::vector<uint8_t> output;

			// Reserve header space (0x48 bytes)
			output.resize(sizeof(t7_script_header), 0);

			// -- String data section (NO padding between strings or after, matching reference) --
			// Reference order: include path strings FIRST, then script name, then code strings

			// 1. Include path strings (normalized: forward slashes, lowercase)
			std::vector<uint32_t> include_string_offsets;
			for (auto& inc_path : s.includes)
			{
				// Normalize: backslash→forward slash, lowercase (matching reference)
				std::string normalized = inc_path;
				for (char& c : normalized)
				{
					if (c == '\\') c = '/';
					c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
				}
				include_string_offsets.push_back(static_cast<uint32_t>(output.size()));
				for (char c : normalized)
					output.push_back(static_cast<uint8_t>(c));
				output.push_back(0);
			}

			// 2. Script name (null-terminated)
			uint32_t name_offset = static_cast<uint32_t>(output.size());
			for (char c : s.script_name)
				output.push_back(static_cast<uint8_t>(c));
			output.push_back(0);

			// 3. Script strings used in bytecode (GetString/GetIString)
			for (auto& str : s.strings)
			{
				str.offset = static_cast<uint32_t>(output.size());
				for (char c : str.value)
					output.push_back(static_cast<uint8_t>(c));
				output.push_back(0);
			}
			// NO padding after strings (reference has none)

			// -- Include table (4 bytes each: file offset to include string) --
			uint32_t include_offset = static_cast<uint32_t>(output.size());
			// Reference reverses includes before writing
			for (int i = static_cast<int>(include_string_offsets.size()) - 1; i >= 0; i--)
				write_u32(output, include_string_offsets[i]);
			// NO padding after include table (reference has none)

			// -- Code section (functions with alignment padding) --
			// Reference: CodeSectionStart = position BEFORE any function alignment
			uint32_t bytecode_start = static_cast<uint32_t>(output.size());

			for (size_t i = 0; i < s.exports.size(); i++)
			{
				auto& exp = s.exports[i];

				// Double QWord align before each function (reference: SetAlignedQWord(); SetAlignedQWord();)
				// SetAlignedQWord: (pos + 8) & ~7
				{
					uint32_t pos = static_cast<uint32_t>(output.size());
					uint32_t a1 = (pos + 8) & ~7u;
					output.resize(a1, 0);
					uint32_t a2 = (a1 + 8) & ~7u;
					output.resize(a2, 0);
				}

				exp.bytecode_offset = static_cast<uint32_t>(output.size());

				output.insert(output.end(), exp.bytecode.begin(), exp.bytecode.end());
			}

			// bytecode_size = total code section including alignment padding (reference: Count - CodeSectionStart)
			uint32_t bytecode_end = static_cast<uint32_t>(output.size());
			uint32_t total_bytecode_size = bytecode_end - bytecode_start;

			// Patch string placeholders (0xFFFFFFFF) in bytecode with actual string file offsets
			auto resolve_ref = [&](const std::pair<size_t, uint32_t>& ref) -> uint32_t
			{
				return s.exports[ref.first].bytecode_offset + ref.second;
			};

			for (auto& str : s.strings)
			{
				for (auto& ref : str.references)
				{
					uint32_t abs_offset = resolve_ref(ref);
					write_at_u32(output, abs_offset, str.offset);
				}
			}

			// -- Section 5: Export table (20 bytes each) --
			uint32_t export_offset = static_cast<uint32_t>(output.size());

			for (size_t i = 0; i < s.exports.size(); i++)
			{
				auto& exp = s.exports[i];
				// CRC32 computed from OUTPUT buffer (has patched string offsets), matching reference
				uint32_t crc = crc32_calc(output.data() + exp.bytecode_offset, exp.bytecode.size());

				write_u32(output, crc);
				write_u32(output, exp.bytecode_offset);
				write_u32(output, exp.function_hash);
				write_u32(output, exp.namespace_hash);
				write_u8(output, exp.num_params);
				write_u8(output, exp.flags);
				write_u16(output, 0); // Unknown, always 0
			}

			// -- Section 6: Import table --
			uint32_t import_offset = static_cast<uint32_t>(output.size());
			for (auto& imp : s.imports)
			{
				write_u32(output, imp.function_hash);
				write_u32(output, imp.namespace_hash);
				write_u16(output, static_cast<uint16_t>(imp.references.size()));
				write_u8(output, imp.num_params);
				write_u8(output, imp.flags);

				for (auto& ref : imp.references)
					write_u32(output, resolve_ref(ref));
			}
			// NO padding after imports (reference has none)

			// -- AnimTree section (empty, but offset must be recorded here) --
			uint32_t animtree_offset = static_cast<uint32_t>(output.size());

			// -- String fixup/reference table --
			// Format per string: u32 string_ptr | u32 num_refs | u32 refs[N]
			uint32_t string_fixup_offset = static_cast<uint32_t>(output.size());
			uint16_t string_count = 0;
			for (auto& str : s.strings)
			{
				if (str.references.empty()) continue;
				string_count++;

				write_u32(output, str.offset);
				write_u32(output, static_cast<uint32_t>(str.references.size()));

				for (size_t j = 0; j < str.references.size(); j++)
					write_u32(output, resolve_ref(str.references[j]));
			}
			// NO padding after string fixup (reference has none)

			uint32_t file_size = static_cast<uint32_t>(output.size());

			// -- Fill header --
			t7_script_header header{};
			header.magic = T7_MAGIC;
			header.source_crc = 0x4C492053;
			header.include_offset = include_offset;
			header.animtree_offset = animtree_offset;
			header.bytecode_offset = bytecode_start;
			header.string_offset = string_fixup_offset;
			header.debug_string_offset = file_size;
			header.export_offset = export_offset;
			header.import_offset = import_offset;
			header.fixup_offset = file_size;
			header.profile_offset = file_size;
			header.bytecode_size = total_bytecode_size;
			header.name_offset = name_offset;
			header.string_count = string_count;
			header.export_count = static_cast<uint16_t>(s.exports.size());
			header.import_count = static_cast<uint16_t>(s.imports.size());
			header.fixup_count = 0;
			header.profile_count = 0;
			header.debug_string_count = 0;
			header.include_count = static_cast<uint8_t>(s.includes.size());
			header.animtree_count = 0;
			header.flags = 0;

			std::memcpy(output.data(), &header, sizeof(header));

			return output;
		}
	}

	emitter_result emit(const ast_ptr& root, const std::string& script_name)
	{
		emitter_result result{};
		emitter_state state;
		state.script_name = script_name;
		state.script_namespace = gsc_hash("ilcustom"); // default namespace

		try
		{
			// First pass: collect namespace and local function hashes
			for (auto& child : root->children)
			{
				if (child->type == node_type::n_namespace)
					state.script_namespace = gsc_hash(child->value);
				else if (child->type == node_type::n_function_def)
				{
					std::string lower = child->value;
					std::transform(lower.begin(), lower.end(), lower.begin(),
						[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
					state.record_hash(lower);
				}
			}

			// Reset namespace for second pass (in case it was set above)
			state.script_namespace = gsc_hash("ilcustom");

			// Second pass: process directives and emit functions
			for (auto& child : root->children)
			{
				if (child->type == node_type::n_namespace)
				{
					state.script_namespace = gsc_hash(child->value);
				}
				else if (child->type == node_type::n_include)
				{
					state.includes.push_back(child->value);
				}
				else if (child->type == node_type::n_function_def)
				{
					emit_function(state, child);
				}
			}

			result.data = assemble(state);
			for (auto& hn : state.hash_names)
				result.hash_names.push_back({hn.hash, std::move(hn.name), hn.line, hn.params});
			result.success = true;
		}
		catch (const std::runtime_error& e)
		{
			result.success = false;
			result.error = e.what();
		}

		return result;
	}
}
