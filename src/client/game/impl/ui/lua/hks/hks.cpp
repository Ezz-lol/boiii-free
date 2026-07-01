#include "hks.hpp"
#include "../../../../../std_include.hpp"
namespace game {
namespace ui {
namespace lua {
namespace hks {

#include <cmath>
#include <cstring>

constexpr hksSize HKS_STR_LEN_BITS_MASK =
    ((1ULL << 62ULL) - 1ULL) /* 0x3FFFFFFFFFFFFFFFLL - masks to first 62 bits*/;

constexpr HksNumber MAX_SAFE_FLOAT_TO_INT = 1073741800.0f;
constexpr HksNumber FP_ERROR = -1.0f;

__optimize __inline_def void checkInstructionPointer(lua_State *s,
                                                     const hksInstruction *ptr,
                                                     const Method *method) {
  const hksInstruction *base = method->instructions.data;
  if (ptr < base || ptr > &base[method->instructions.size] ||
      ((reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(base)) &
       3) != 0) {
    hksi_luaL_error(s, "VM Kernel error: invalid instruction pointer.");
  }
}

__optimize __inline_def HksNumber getNumber(lua_State *s, HksObject *obj) {
  HksNumber result = 0.0f;
  if (obj->type() == HksObjectType::TNUMBER) {
    result = obj->v.number;
  } else if (obj->type() != HksObjectType::TSTRING ||
             stringToNumber(obj->v.str->m_data,
                            obj->v.str->m_lengthbits & HKS_STR_LEN_BITS_MASK,
                            &result) == 0) {
    [[unlikely]] hksi_luaL_error(s, "number expected for loop variables");
  }

  return result;
}

/*
  9th bit of register is a flag indicating whether the first 8 bits
  represent a constant index (1) or a register index (0).
*/
constexpr hksUint32 RK_FLAG_BIT_IDX = 8; // zero-indexed

// Register-Constant
inline constexpr bool rk_reg(hksUint32 reg) {
  return static_cast<bool>(reg >> RK_FLAG_BIT_IDX);
}
// Register-Register
inline constexpr bool rr_reg(hksUint32 reg) { return !rk_reg(reg); }

__optimize __inline_def HksObject tableLookup(const HashTable *table,
                                              const HksObject *keyObj) {
  if (!table->m_hashPart) {
    return *NilValue;
  }

  const hksUint32 hash = static_cast<hksUint32>(keyObj->v.raw) ^
                         static_cast<hksUint32>(keyObj->v.raw >> 5) ^
                         static_cast<hksUint32>(keyObj->v.raw >> 8);

  const HksObjectType targetType = keyObj->type();
  HashTable::Node *node = &table->m_hashPart[hash & table->m_mask];

  // The next-pointer array sits immediately before m_hashPart in memory.
  // For a node at index i, its next pointer is at nextPtrs[-i - 1].
  // The allocation is split at time of creation at index `m_mask + 1` to allow
  // symmetric, neighbouring hash and next-ptr node list alllocation and, later,
  // accesses.
  HashTable::Node **nextPtrs =
      reinterpret_cast<HashTable::Node **>(table->m_hashPart);

  while (true) {
    if (node->m_key.type() == targetType &&
        node->m_key.v.ptr == keyObj->v.ptr) {
      return node->m_value;
    }

    const size_t nodeIdx = (reinterpret_cast<char *>(node) -
                            reinterpret_cast<char *>(table->m_hashPart)) /
                           sizeof(HashTable::Node);
    HashTable::Node *nextNode = nextPtrs[-static_cast<ptrdiff_t>(nodeIdx) - 1];

    if (!nextNode) {
      return *NilValue;
    }

    node = nextNode;
  }
}

// hks::execute<(HksBytecodeSharingMode)2>
__optimize __inline_def luaReturnCount_e executeSharedSecure(
    lua_State *s, const hksInstruction *PC, hksInt32 call_depth) {
  hksInt32 current_call_depth = call_depth;
  HksObject *fp = s->m_apistack.base;
  HksObject *m_base = fp;

  HksClosure *closure = fp[-1].v.closure;
  const HksObject *consts = closure->m_cache.consts;

  HksObject *call_m_end = nullptr;
  HksObject *tailcall_m_end = nullptr;

  // Registers that persist across instruction dispatch (used heavily in fused
  // GETFIELD/SETFIELD logic)
  HksRegister opReg = *NilValue;
  HksRegister opReg1 = *NilValue;

  checkInstructionPointer(s, PC, closure->m_method);

  while (true) {
    s->m_callStack.m_current_lua_pc = PC;
    Method *active_method = closure->m_method;
    hksInstruction instruction = *PC;

    ++PC;
    checkInstructionPointer(s, PC, active_method);

    // Decode Standard Instruction Operands
    Opcode opcode = instruction.Opcode();
    hksUint32 opA = instruction.opA;
    hksUint32 opC = instruction.opC;       // 8 bits
    hksUint32 opC_RK = instruction.opC_RK; // 9 bits
    hksUint32 opB = instruction.opB;       // 8 bits
    hksUint32 opB_RK = instruction.opB_RK; // 9 bits

    hksUint32 opBx = instruction.opBx;
    hksInt32 opSBx = instruction.OpSBx();

    HksObject *destReg = &fp[opA];

    switch (opcode) {
    case Opcode::MOVE: {
      opReg = fp[opB];
      *destReg = opReg;
      opReg1 = opReg;
      break;
    }

    case Opcode::LOADK: {
      destReg->t = consts[opBx].type();
      destReg->v.ptr = consts[opBx].v.ptr;
      break;
    }

    case Opcode::LOADBOOL: {
      destReg->v.native = opB;
      destReg->t = HksObjectType::TBOOLEAN;
      PC += opC;
      checkInstructionPointer(s, PC, closure->m_method);
      break;
    }

    case Opcode::LOADNIL: {
      for (HksObject *iter = &fp[opB]; destReg <= iter; ++destReg) {
        destReg->t = HksObjectType::TNIL;
      }
      break;
    }

    case Opcode::GETUPVAL: {
      HksObject *loc = closure->m_upvalues[opB]->loc;
      *destReg = *loc;
      opReg = *loc;
      opReg1 = opReg;
      break;
    }

    case Opcode::SETUPVAL: {
      opReg = *destReg;
      opReg1 = *destReg;
      [[fallthrough]];
    }
    case Opcode::SETUPVAL_R1: {
      UpValue *upval = closure->m_upvalues[opB];
      *upval->loc = opReg;

      if (s->m_global->m_collector.m_phase == 1) {
        GarbageCollector::writeBarrier(&s->m_global->m_collector, upval,
                                       upval->loc);
        consts = closure->m_cache.consts;
        call_m_end = tailcall_m_end;
      }
      break;
    }

    case Opcode::GETGLOBAL:
    case Opcode::GETGLOBAL_MEM: {
      HashTable *env = closure->m_env;
      HksRegister key = consts[opBx];
      opReg.t = HksObjectType::TTABLE;
      HashTable::getByString(env, &opReg, &key);
      opReg1 = opReg;

      if (opReg.type() != HksObjectType::TNIL) {
        consts = closure->m_cache.consts;
        call_m_end = tailcall_m_end;
        *destReg = opReg;
      } else {
        MetaTable *meta = env->m_meta;
        if (meta != nullptr) {
          HksRegister handler;
          HKS_METATABLE_GET(&handler, s, meta, Metamethod::M_INDEX);
          if (handler.type() != HksObjectType::TNIL) {
            HksRegister tableObj;
            tableObj.t = HksObjectType::TTABLE;
            tableObj.v.table = env;

            gettable_event_string_outofline_tail(&opReg, s, &tableObj, &key, PC,
                                                 &handler, 0x20);

            fp = s->m_apistack.base;
            consts = closure->m_cache.consts;
            call_m_end = tailcall_m_end;
            m_base = fp;
            *destReg = opReg;
            opReg1 = opReg;
          } else {
            consts = closure->m_cache.consts;
            call_m_end = tailcall_m_end;
            destReg->t = HksObjectType::TNIL;
          }
        } else {
          consts = closure->m_cache.consts;
          call_m_end = tailcall_m_end;
          destReg->t = HksObjectType::TNIL;
        }
      }
      break;
    }

    case Opcode::SETGLOBAL: {
      HashTable *env = closure->m_env;
      InternString *keyString = consts[opBx].v.str;

      if (env->m_meta == nullptr) {
        HashTable::insertString(env, s, keyString, destReg);
      } else {
        HksRegister tableObj;
        tableObj.t = HksObjectType::TTABLE;
        tableObj.v.table = env;
        settable_event_outofline_string(s, &tableObj, keyString, destReg, PC,
                                        0x20);
      }
      fp = s->m_apistack.base;
      consts = closure->m_cache.consts;
      call_m_end = tailcall_m_end;
      m_base = fp;
      break;
    }

    case Opcode::GETFIELD: {
      opReg = fp[opB];
      opReg1 = opReg;
      [[fallthrough]];
    }
    case Opcode::GETFIELD_R1: {
      if (opReg1.type() != HksObjectType::TTABLE) {
        HksRegister keyObj = consts[opC];
        gettable_event_string_outofline_other(&opReg, s, &opReg, &keyObj, PC,
                                              0x20);
      } else {
        HksObject res = tableLookup(opReg1.v.table, &consts[opC]);
        if (res.type() != HksObjectType::TNIL) {
          opReg = res;
        } else {
          MetaTable *meta = opReg1.v.table->m_meta;
          if (meta == nullptr) {
            opReg = *NilValue;
          } else {
            HksRegister handler;
            HKS_METATABLE_GET(&handler, s, meta, Metamethod::M_INDEX);
            if (handler.type() == HksObjectType::TNIL) {
              opReg = *NilValue;
            } else {
              HksRegister tableObj = opReg1;
              HksRegister keyObj = consts[opC];
              gettable_event_string_outofline_tail(&opReg, s, &tableObj,
                                                   &keyObj, PC, &handler, 0x20);
            }
          }
        }
      }
      fp = s->m_apistack.base;
      opReg1 = opReg;
      *destReg = opReg;
      m_base = fp;
      PC += 2;
      checkInstructionPointer(s, PC, closure->m_method);
      break;
    }

    case Opcode::SETFIELD: {
      opReg = *destReg;
      opReg1 = *destReg;
      [[fallthrough]];
    }
    case Opcode::SETFIELD_R1: {
      const HksObject *fieldStackObj =
          rr_reg(opC_RK) ? &fp[opC_RK] : &consts[opC];
      InternString *keyStr = consts[opB].v.str;

      if (opReg1.type() != HksObjectType::TTABLE ||
          opReg1.v.table->m_meta != nullptr) {
        settable_event_outofline_string(s, &opReg, keyStr, fieldStackObj, PC,
                                        0x20);
      } else {
        HashTable::insertString(opReg1.v.table, s, keyStr, fieldStackObj);
      }
      fp = s->m_apistack.base;
      consts = closure->m_cache.consts;
      call_m_end = tailcall_m_end;
      m_base = fp;
      break;
    }

    case Opcode::GETFIELD_MM: {
      hksUint32 tableStackObjIdx = opB;
      HksObject fieldStackObj = fp[tableStackObjIdx];

      if (fieldStackObj.type() != HksObjectType::TTABLE) {
        HksRegister keyObj = consts[opC];
        gettable_event_string_outofline_other(&opReg, s, &fieldStackObj,
                                              &keyObj, PC, 0x20);
      } else {
        HksObject res = tableLookup(fieldStackObj.v.table, &consts[opC]);
        if (res.type() != HksObjectType::TNIL) {
          opReg = res;
        } else {
          MetaTable *meta = fieldStackObj.v.table->m_meta;
          if (meta == nullptr) {
            opReg = *NilValue;
          } else {
            HksRegister handler;
            HKS_METATABLE_GET(&handler, s, meta, Metamethod::M_INDEX);
            if (handler.type() == HksObjectType::TNIL) {
              opReg = *NilValue;
            } else {
              HksRegister keyObj = consts[opC];
              HksRegister tableObj = fp[tableStackObjIdx];
              gettable_event_string_outofline_tail(&opReg, s, &tableObj,
                                                   &keyObj, PC, &handler, 0x20);
            }
          }
        }
      }
      fp = s->m_apistack.base;
      opReg1 = opReg;
      *destReg = opReg;
      m_base = fp;
      PC += 2;
      checkInstructionPointer(s, PC, closure->m_method);
      break;
    }

    case Opcode::TEST: {
      opReg = *destReg;
      opReg1 = *destReg;
      [[fallthrough]];
    }
    case Opcode::TEST_R1: {
      if (opReg1.truthy() == static_cast<bool>(opC)) {
        PC += PC->OpSBx() + 1;
      } else {
        ++PC;
      }
      checkInstructionPointer(s, PC, closure->m_method);
      break;
    }

    case Opcode::TESTSET: {
      HksObject *targetObj = &fp[opB];

      if (targetObj->truthy() == static_cast<bool>(opC)) {
        *destReg = *targetObj;
        PC += PC->OpSBx() + 1;
      } else {
        ++PC;
      }
      checkInstructionPointer(s, PC, closure->m_method);
      break;
    }

    case Opcode::NOT: {
      opReg1 = fp[opB];
      opReg = opReg1;
      [[fallthrough]];
    }
    case Opcode::NOT_R1: {
      destReg->v.native = !opReg1.truthy();
      destReg->t = HksObjectType::TBOOLEAN;
      break;
    }

    case Opcode::CALL_I:
    case Opcode::CALL_C:
    case Opcode::CALL_M:
    case Opcode::CALL:
    case Opcode::CALL_I_R1: {
      HksObject *funcObj = nullptr;
      HksObject *top = nullptr;
      hksUint32 bVal = opB;
      hksUint32 cVal = opC;
      hksInt32 expectedReturns = cVal - 1;

      if (destReg->type() == HksObjectType::TIFUNCTION ||
          destReg->type() == HksObjectType::TCFUNCTION) {
        funcObj = call_m_end;
      } else {
        HksObject *argEnd = (bVal != 0) ? &destReg[bVal] : s->m_apistack.top;
        funcObj = non_function_call(s, destReg, argEnd);
        call_m_end = funcObj;
        destReg = &s->m_apistack.base[opA];

        HksObjectType objType = destReg->type();
        if (objType != HksObjectType::TIFUNCTION &&
            objType != HksObjectType::TCFUNCTION) {
          hksi_luaL_error(
              s, "function expected instead of %s",
              s_compilerTypeName->pool[static_cast<hksUint32>(objType)]);
        }
      }

      opReg = *destReg;
      opReg1 = *destReg;

      if (destReg->type() == HksObjectType::TIFUNCTION) {
        if (funcObj != nullptr) {
          top = funcObj;
          call_m_end = nullptr;
        } else if (bVal != 0) {
          top = &destReg[bVal];
        } else {
          top = s->m_apistack.top;
        }

        closure = destReg->v.closure;
        ++current_call_depth;
        HksRegister *returnCount = CallStack::functionCall(
            &s->m_callStack, s, expectedReturns, destReg, top, PC);
        PC = closure->m_cache.inst;
        fp = returnCount;
        m_base = returnCount;
        consts = closure->m_cache.consts;
        checkInstructionPointer(s, PC, closure->m_method);
      } else {
        if (funcObj != nullptr) {
          top = funcObj;
          call_m_end = nullptr;
        } else if (bVal != 0) {
          top = &destReg[bVal];
        } else {
          top = s->m_apistack.top;
        }

        cclosure *cClosure = destReg->v.cClosure;
        CallStack::push(&s->m_callStack, s, expectedReturns, destReg, top, PC);
        lua_CFunction m_function = cClosure->m_function;

        if (++s->m_numberOfCCalls > 128) {
          hksi_luaL_error(
              s, "C stack overflow: too many (%d) nested C function calls.\n",
              128);
        }

        luaReturnCount_e retCount =
            static_cast<luaReturnCount_e>(m_function(s));
        --s->m_numberOfCCalls;

        if (retCount == luaReturnCount_e::INVALID) {
          return retCount;
        }

        HksObject *popped = CallStack::pop(&s->m_callStack, s, retCount);
        consts = closure->m_cache.consts;
        fp = popped;
        m_base = popped;
      }
      break;
    }

    case Opcode::TAILCALL_I:
    case Opcode::TAILCALL_C:
    case Opcode::TAILCALL_M:
    case Opcode::TAILCALL:
    case Opcode::TAILCALL_I_R1: {
      HksObjectType objType = destReg->type();

      if (objType != HksObjectType::TIFUNCTION &&
          objType != HksObjectType::TCFUNCTION) {
        HksObject *argEnd = opB == 0 ? s->m_apistack.top : &destReg[opB];
        HksObject *res = non_function_call(s, destReg, argEnd);
        fp = s->m_apistack.base;
        destReg = &fp[opA];
        call_m_end = res;
        tailcall_m_end = res;
        objType = destReg->type();

        if (objType != HksObjectType::TIFUNCTION &&
            objType != HksObjectType::TCFUNCTION) {
          hksi_luaL_error(
              s, "function expected instead of %s",
              s_compilerTypeName->pool[static_cast<hksUint32>(objType)]);
        }
      }

      if (objType == HksObjectType::TIFUNCTION) {
        opReg1 = *destReg;
        HksObject *argEnd;
        if (call_m_end != nullptr) {
          argEnd = call_m_end;
          tailcall_m_end = nullptr;
        } else if (opB != 0) {
          argEnd = &destReg[opB];
        } else {
          argEnd = s->m_apistack.top;
        }

        if (closure->m_mayHaveUpvalues != 0) {
          closePendingUpvalues(s, fp);
        }

        closure = destReg->v.closure;
        CallStack::functionTailCall(&s->m_callStack, s, destReg, argEnd);

        PC = closure->m_cache.inst;
        consts = closure->m_cache.consts;
        checkInstructionPointer(s, PC, closure->m_method);

        fp = s->m_apistack.base;
        opReg = opReg1;
        call_m_end = tailcall_m_end;
        m_base = fp;
      } else {
        HksObject *top;
        if (call_m_end != nullptr) {
          top = call_m_end;
          tailcall_m_end = nullptr;
        } else if (opB != 0) {
          top = &destReg[opB];
        } else {
          top = s->m_apistack.top;
        }

        cclosure *cClosure = destReg->v.cClosure;
        CallStack::push(&s->m_callStack, s, -1, destReg, top, PC);
        lua_CFunction m_function = cClosure->m_function;

        if (++s->m_numberOfCCalls > 128) {
          hksi_luaL_error(
              s, "C stack overflow: too many (%d) nested C function calls.\n",
              128);
        }

        luaReturnCount_e retCount =
            static_cast<luaReturnCount_e>(m_function(s));
        --s->m_numberOfCCalls;

        if (retCount < luaReturnCount_e::NONE) {
          return retCount;
        }

        HksObject *popped = CallStack::pop(&s->m_callStack, s, retCount);
        consts = closure->m_cache.consts;
        fp = popped;
        m_base = popped;
      }
      break;
    }

    case Opcode::EQ:
    case Opcode::EQ_BK: {
      const HksObject *opLeft = rr_reg(opB_RK) ? &fp[opB_RK] : &consts[opB];
      const HksObject *opRight = rr_reg(opC_RK) ? &fp[opC_RK] : &consts[opC];

      HksObjectType typeLeft = opLeft->type();
      HksObjectType typeRight = opRight->type();
      bool isEqual = false;

      if (typeLeft == typeRight) {
        if (typeLeft == HksObjectType::TNIL) {
          isEqual = true;
        } else if (typeLeft == HksObjectType::TBOOLEAN) {
          isEqual = (opLeft->v.native == opRight->v.native);
        } else if (typeLeft == HksObjectType::TNUMBER) {
          isEqual = (opLeft->v.number == opRight->v.number);
        } else if (typeLeft == HksObjectType::TSTRING ||
                   typeLeft == HksObjectType::TLIGHTUSERDATA) {
          isEqual = (opLeft->v.ptr == opRight->v.ptr);
        } else {
          if (opLeft->v.ptr == opRight->v.ptr) {
            isEqual = true;
          } else if (typeLeft == HksObjectType::TTABLE ||
                     typeLeft == HksObjectType::TUSERDATA) {
            HksRegister handler;

            getcomphandler(&handler, s, opLeft, opRight, Metamethod::M_EQ);
            if (handler.type() != HksObjectType::TNIL) {
              if (&s->m_apistack.top[3] > s->m_apistack.alloc_top) {
                CallStack::growApiStack(&s->m_callStack, s, 3);
                fp = s->m_apistack.base;
                opLeft = rr_reg(opB_RK) ? &fp[opB_RK] : &consts[opB];
                opRight = rr_reg(opC_RK) ? &fp[opC_RK] : &consts[opC];
              }
              HksObject *opStack = s->m_apistack.top;
              opStack[0] = handler;
              opStack[1] = *opLeft;
              opStack[2] = *opRight;
              s->m_apistack.top += 3;

              hksi_lua_call(s, 2, 1, PC);

              HksObject *res = s->m_apistack.top - 1;
              fp = s->m_apistack.base;
              m_base = fp;
              s->m_apistack.top = res;

              HksObjectType resType = res->type();
              if (resType != HksObjectType::TNIL &&
                  (resType != HksObjectType::TBOOLEAN || res->v.native != 0)) {
                isEqual = true;
              }
              closure = fp[-1].v.closure;
              consts = closure->m_cache.consts;
            }
          }
        }
      }

      if (isEqual == opA) {
        PC += PC->OpSBx() + 1;
      } else {
        ++PC;
      }
      checkInstructionPointer(s, PC, closure->m_method);
      break;
    }

    case Opcode::LT:
    case Opcode::LT_BK:
    case Opcode::LE:
    case Opcode::LE_BK: {
      const HksObject *opLeft = rr_reg(opB_RK) ? &fp[opB_RK] : &consts[opB];
      const HksObject *opRight = rr_reg(opC_RK) ? &fp[opC_RK] : &consts[opC];

      bool conditionMet = false;

      if (opLeft->type() == HksObjectType::TNUMBER &&
          opRight->type() == HksObjectType::TNUMBER) {
        if (opcode == Opcode::LT || opcode == Opcode::LT_BK) {
          conditionMet = opLeft->v.number < opRight->v.number;
        } else {
          conditionMet = opLeft->v.number <= opRight->v.number;
        }
      } else if (opLeft->type() == HksObjectType::TSTRING &&
                 opRight->type() == HksObjectType::TSTRING) {
        hksInt32 cmp = compareString(opLeft->v.str, opRight->v.str);
        if (opcode == Opcode::LT || opcode == Opcode::LT_BK) {
          conditionMet = (cmp < 0);
        } else {
          conditionMet = (cmp <= 0);
        }
      } else {
        HksRegister handler;
        if (opcode == Opcode::LT || opcode == Opcode::LT_BK) {
          getrelhandler(&handler, s, opLeft, opRight, Metamethod::M_LT, "<");
        } else {
          getcomphandler(&handler, s, opLeft, opRight, Metamethod::M_LE);
          if (handler.type() == HksObjectType::TNIL) {
            // Fallback for LE is not(LT(B, A))
            getrelhandler(&handler, s, opLeft, opRight, Metamethod::M_LT, "<");
            std::swap(opLeft, opRight);
            conditionMet = true; // Inverted check flag
          }
        }

        if (&s->m_apistack.top[3] > s->m_apistack.alloc_top) {
          CallStack::growApiStack(&s->m_callStack, s, 3);
          fp = s->m_apistack.base;
          opLeft = rr_reg(opB_RK) ? &fp[opB_RK] : &consts[opB];
          opRight = rr_reg(opC_RK) ? &fp[opC_RK] : &consts[opC];
        }

        HksObject *opStack = s->m_apistack.top;
        opStack[0] = handler;
        opStack[1] = *opLeft;
        opStack[2] = *opRight;
        s->m_apistack.top += 3;

        hksi_lua_call(s, 2, 1, PC);

        HksObject *res = s->m_apistack.top - 1;
        fp = s->m_apistack.base;
        m_base = fp;
        s->m_apistack.top = res;

        bool resultTruthy = res->truthy();

        if (opcode == Opcode::LE || opcode == Opcode::LE_BK) {
          if (conditionMet)
            resultTruthy =
                !resultTruthy; // Apply inversion if fallback was used
          conditionMet = resultTruthy;
        } else {
          conditionMet = resultTruthy;
        }
        closure = fp[-1].v.closure;
        consts = closure->m_cache.consts;
      }

      if (conditionMet == opA) {
        PC += PC->OpSBx() + 1;
      } else {
        ++PC;
      }
      checkInstructionPointer(s, PC, closure->m_method);
      break;
    }

    case Opcode::SELF: {
      HksObject *tableObj = &fp[opB];
      const HksObject *keyObj = rr_reg(opC_RK) ? &fp[opC_RK] : &consts[opC];

      destReg[1] = *tableObj; // Store 'self' in R(A+1)

      if (tableObj->type() == HksObjectType::TTABLE) {
        HashTable::getByString(tableObj->v.table, &opReg, keyObj);
        if (opReg.type() == HksObjectType::TNIL) {
          gettable_event_string_outofline_table(&opReg, s, tableObj, keyObj, PC,
                                                0x20);
        }
      } else {
        gettable_event_string_outofline_other(&opReg, s, tableObj, keyObj, PC,
                                              0x20);
      }

      fp = s->m_apistack.base;
      consts = closure->m_cache.consts;
      call_m_end = tailcall_m_end;
      m_base = fp;

      *destReg = opReg; // Store method in R(A)
      opReg1 = opReg;
      break;
    }

    case Opcode::GETTABLE_S:
    case Opcode::GETTABLE_N:
    case Opcode::GETTABLE: {
      HksObject *tableObj = &fp[opB];
      const HksObject *keyObj = rr_reg(opC_RK) ? &fp[opC_RK] : &consts[opC];

      if (tableObj->type() != HksObjectType::TTABLE) {
        gettable_event_outofline_other(destReg, s, tableObj, keyObj, PC, 0x20);
        fp = s->m_apistack.base;
      } else {
        HksRegister m_value = *NilValue;
        HksObjectType keyType = keyObj->type();

        if (keyType == HksObjectType::TSTRING) {
          HashTable::getByString(tableObj->v.table, &m_value, keyObj);
        } else if (keyType == HksObjectType::TNUMBER) {
          HksNumber number = keyObj->v.number;
          if (std::floor(number) == number &&
              (number - 1) < tableObj->v.table->m_arraySize && number >= 1.0f) {
            m_value = tableObj->v.table
                          ->m_arrayPart[static_cast<hksUint32>(number) - 1];
          } else {
            HashTable::Node *node =
                HashTable::findKeyPosition(tableObj->v.table, keyObj);
            if (node) {
              m_value = node->m_value;
            }
          }
        } else {
          HashTable::Node *node =
              HashTable::findKeyPosition(tableObj->v.table, keyObj);
          if (node) {
            m_value = node->m_value;
          }
        }

        if (m_value.type() != HksObjectType::TNIL) {
          fp = s->m_apistack.base;
          *destReg = m_value;
        } else {
          MetaTable *meta = tableObj->v.table->m_meta;
          if (meta != nullptr) {
            HksRegister handler;
            HKS_METATABLE_GET(&handler, s, meta, Metamethod::M_INDEX);
            if (handler.type() != HksObjectType::TNIL) {
              gettable_event_outofline_tail(destReg, s, tableObj, keyObj, PC,
                                            &handler, 0x20);
              fp = s->m_apistack.base;
            } else {
              fp = s->m_apistack.base;
              *destReg = *NilValue;
            }
          } else {
            fp = s->m_apistack.base;
            *destReg = *NilValue;
          }
        }
      }
      m_base = fp;
      PC += 2;
      checkInstructionPointer(s, PC, closure->m_method);
      break;
    }

    case Opcode::SETTABLE_S:
    case Opcode::SETTABLE_S_BK:
    case Opcode::SETTABLE_N:
    case Opcode::SETTABLE_N_BK:
    case Opcode::SETTABLE:
    case Opcode::SETTABLE_BK: {
      const HksObject *keyObj = rr_reg(opB_RK) ? &fp[opB_RK] : &consts[opB];
      const HksObject *valObj = rr_reg(opC_RK) ? &fp[opC_RK] : &consts[opC];

      if (destReg->type() == HksObjectType::TTABLE) {
        HashTable *table = destReg->v.table;
        if (table->m_meta == nullptr) {
          HashTable::tableInsert(table, s, keyObj, valObj);
        } else {
          settable_event_outofline(s, destReg, keyObj, valObj, PC, 0x20);
        }
      } else {
        settable_event_outofline(s, destReg, keyObj, valObj, PC, 0x20);
      }

      fp = s->m_apistack.base;
      consts = closure->m_cache.consts;
      call_m_end = tailcall_m_end;
      m_base = fp;
      break;
    }

    case Opcode::JMP: {
      PC += opSBx;
      checkInstructionPointer(s, PC, closure->m_method);
      break;
    }

    case Opcode::RETURN: {
      if (closure->m_mayHaveUpvalues != 0) {
        closePendingUpvalues(s, fp);
      }

      luaReturnCount_e returnLength = static_cast<luaReturnCount_e>(opB - 1);

      if (current_call_depth > 0) {
        CallStack::ActivationRecord *current_record = s->m_callStack.m_current;
        luaReturnCount_e expectedReturns = current_record->m_numExpectedReturns;

        hksInt64 stackDiff = (reinterpret_cast<char *>(s->m_apistack.top) -
                              reinterpret_cast<char *>(destReg)) /
                             sizeof(HksObject);
        --current_call_depth;

        luaReturnCount_e actualReturns = returnLength;
        if (static_cast<luaReturnCount_e>(stackDiff) < actualReturns)
          actualReturns = static_cast<luaReturnCount_e>(stackDiff);

        luaReturnCount_e finalReturns = expectedReturns;
        if (actualReturns < finalReturns)
          finalReturns = actualReturns;

        HksObject *retBase =
            &s->m_apistack
                 .base[-static_cast<hksInt32>(current_record->m_numVarargs) -
                       1];

        for (hksUint32 i = 0; i < static_cast<hksUint32>(finalReturns); ++i) {
          retBase[i] = destReg[i];
        }

        fp = current_record->m_base;
        m_base = current_record->m_base;
        closure = m_base[-1].v.closure;

        HksObject *newTop;
        newTop = &retBase[static_cast<hksUint32>(finalReturns)];
        if (expectedReturns != luaReturnCount_e::INVALID) {
          HksObject *limit = &fp[current_record->m_numVarargs +
                                 closure->m_cache.m_numRegisters];
          for (; newTop < limit; ++newTop) {
            newTop->t = HksObjectType::TNIL;
          }
        }

        PC = current_record->m_returnAddress;
        s->m_apistack.base = fp;
        s->m_apistack.top = newTop;
        s->m_callStack.m_current = current_record - 1;

        consts = closure->m_cache.consts;
        checkInstructionPointer(s, PC, closure->m_method);
        call_m_end = tailcall_m_end;
      } else {
        if (opB != 0) {
          s->m_apistack.top = &fp[static_cast<hksUint32>(returnLength) + opA];
          return returnLength;
        } else {
          return static_cast<luaReturnCount_e>(
              (reinterpret_cast<char *>(
                   &s->m_apistack.top[-static_cast<hksInt32>(opA)]) -
               reinterpret_cast<char *>(fp)) /
              sizeof(HksObject));
        }
      }
      break;
    }

    // Arithmetic Operations
    case Opcode::ADD:
    case Opcode::ADD_BK:
    case Opcode::SUB:
    case Opcode::SUB_BK:
    case Opcode::MUL:
    case Opcode::MUL_BK:
    case Opcode::DIV:
    case Opcode::DIV_BK:
    case Opcode::MOD:
    case Opcode::MOD_BK:
    case Opcode::POW:
    case Opcode::POW_BK: {
      const HksObject *opLeft = rr_reg(opB_RK) ? &fp[opB_RK] : &consts[opB];
      const HksObject *opRight = rr_reg(opC_RK) ? &fp[opC_RK] : &consts[opC];

      bool numericOp = opLeft->type() == HksObjectType::TNUMBER &&
                       opRight->type() == HksObjectType::TNUMBER;

      if (numericOp) {
        destReg->t = HksObjectType::TNUMBER;
        if (opcode == Opcode::ADD || opcode == Opcode::ADD_BK) {
          destReg->v.number = opLeft->v.number + opRight->v.number;
        } else if (opcode == Opcode::SUB || opcode == Opcode::SUB_BK) {
          destReg->v.number = opLeft->v.number - opRight->v.number;
        } else if (opcode == Opcode::MUL || opcode == Opcode::MUL_BK) {
          destReg->v.number = opLeft->v.number * opRight->v.number;
        } else if (opcode == Opcode::DIV || opcode == Opcode::DIV_BK) {
          destReg->v.number = opLeft->v.number / opRight->v.number;
        } else if (opcode == Opcode::MOD || opcode == Opcode::MOD_BK) {
          destReg->v.number =
              opLeft->v.number -
              (std::floor(opLeft->v.number / opRight->v.number) *
               opRight->v.number);
        } else if (opcode == Opcode::POW || opcode == Opcode::POW_BK) {
          destReg->v.number = std::pow(opLeft->v.number, opRight->v.number);
        }
      } else {
        HksRegister result;
        HksObject leftCopy = *opLeft;
        HksObject rightCopy = *opRight;

        if (opcode == Opcode::ADD || opcode == Opcode::ADD_BK) {
          operator_outofline<OpcodeAdd>(&result, s, &leftCopy, &rightCopy, PC);
        } else if (opcode == Opcode::SUB || opcode == Opcode::SUB_BK) {
          operator_outofline<OpcodeSub>(&result, s, &leftCopy, &rightCopy, PC);
        } else if (opcode == Opcode::MUL || opcode == Opcode::MUL_BK) {
          operator_outofline<OpcodeMul>(&result, s, &leftCopy, &rightCopy, PC);
        } else if (opcode == Opcode::DIV || opcode == Opcode::DIV_BK) {
          operator_outofline<OpcodeDiv>(&result, s, &leftCopy, &rightCopy, PC);
        } else if (opcode == Opcode::MOD || opcode == Opcode::MOD_BK) {
          operator_outofline<OpcodeMod>(&result, s, &leftCopy, &rightCopy, PC);
        } else {
          operator_outofline<OpcodePow>(&result, s, &leftCopy, &rightCopy, PC);
        }

        fp = s->m_apistack.base;
        consts = closure->m_cache.consts;
        call_m_end = tailcall_m_end;
        m_base = fp;
        destReg = &fp[opA];
        *destReg = result;
      }
      break;
    }

    // Bitwise and Shifts
    case Opcode::LEFT_SHIFT:
    case Opcode::LEFT_SHIFT_BK:
    case Opcode::RIGHT_SHIFT:
    case Opcode::RIGHT_SHIFT_BK:
    case Opcode::BIT_AND:
    case Opcode::BIT_AND_BK:
    case Opcode::BIT_OR:
    case Opcode::BIT_OR_BK: {
      const HksObject *opLeft = rr_reg(opB_RK) ? &fp[opB_RK] : &consts[opB];
      const HksObject *opRight = rr_reg(opC_RK) ? &fp[opC_RK] : &consts[opC];

      bool isNumbers = opLeft->type() == HksObjectType::TNUMBER &&
                       opRight->type() == HksObjectType::TNUMBER;

      if (isNumbers) {
        destReg->t = HksObjectType::TNUMBER;
        if (opcode == Opcode::LEFT_SHIFT || opcode == Opcode::LEFT_SHIFT_BK) {
          destReg->v.number = static_cast<HksNumber>(
              static_cast<hksInt32>(opLeft->v.number)
              << static_cast<hksInt32>(opRight->v.number));
        } else if (opcode == Opcode::RIGHT_SHIFT ||
                   opcode == Opcode::RIGHT_SHIFT_BK) {
          destReg->v.number =
              static_cast<HksNumber>(static_cast<hksInt32>(opLeft->v.number) >>
                                     static_cast<hksInt32>(opRight->v.number));
        } else if (opcode == Opcode::BIT_AND || opcode == Opcode::BIT_AND_BK) {
          destReg->v.number =
              static_cast<HksNumber>(static_cast<hksInt32>(opLeft->v.number) &
                                     static_cast<hksInt32>(opRight->v.number));
        } else if (opcode == Opcode::BIT_OR || opcode == Opcode::BIT_OR_BK) {
          destReg->v.number =
              static_cast<HksNumber>(static_cast<hksInt32>(opLeft->v.number) |
                                     static_cast<hksInt32>(opRight->v.number));
        }
      } else {
        HksRegister result;
        HksObject leftCopy = *opLeft;
        HksObject rightCopy = *opRight;

        if (opcode == Opcode::LEFT_SHIFT || opcode == Opcode::LEFT_SHIFT_BK)

          operator_outofline<OpcodeLShift>(&result, s, &leftCopy, &rightCopy,
                                           PC);
        else if (opcode == Opcode::RIGHT_SHIFT ||
                 opcode == Opcode::RIGHT_SHIFT_BK)

          operator_outofline<OpcodeRShift>(&result, s, &leftCopy, &rightCopy,
                                           PC);
        else if (opcode == Opcode::BIT_AND || opcode == Opcode::BIT_AND_BK)

          operator_outofline<OpcodeBitAnd>(&result, s, &leftCopy, &rightCopy,
                                           PC);
        else

          operator_outofline<OpcodeBitOr>(&result, s, &leftCopy, &rightCopy,
                                          PC);

        fp = s->m_apistack.base;
        consts = closure->m_cache.consts;
        call_m_end = tailcall_m_end;
        m_base = fp;
        destReg = &fp[opA];
        *destReg = result;
      }
      break;
    }

    case Opcode::UNM: {
      HksObject *opObj = &fp[opB];
      if (opObj->type() == HksObjectType::TNUMBER) {
        destReg->t = HksObjectType::TNUMBER;
        destReg->v.number = -opObj->v.number;
      } else {
        HksRegister result;

        operator_outofline<OpcodeUnm>(&result, s, opObj, opObj, PC);
        fp = s->m_apistack.base;
        consts = closure->m_cache.consts;
        call_m_end = tailcall_m_end;
        m_base = fp;
        destReg = &fp[opA];
        *destReg = result;
      }
      break;
    }

    case Opcode::LEN: {
      HksObject *target = &fp[opB];
      HksObjectType targetType = target->type();

      if (targetType == HksObjectType::TSTRING) {
        destReg->t = HksObjectType::TNUMBER;
        destReg->v.number = static_cast<HksNumber>(target->v.str->m_lengthbits &
                                                   HKS_STR_LEN_BITS_MASK);
      } else if (targetType == HksObjectType::TTABLE) {
        destReg->t = HksObjectType::TNUMBER;
        destReg->v.number = static_cast<HksNumber>(
            HashTable::contiguousArraySize(target->v.table));
      } else {
        if (&s->m_apistack.top[3] > s->m_apistack.alloc_top) {
          CallStack::growApiStack(&s->m_callStack, s, 3);
          target = &s->m_apistack.base[opB];
          destReg = &s->m_apistack.base[opA];
        }
        HksObject dummyObj = *NilValue;
        callBinaryHandler(s, target, &dummyObj, Metamethod::M_LEN, "#", PC);

        HksObject *res = s->m_apistack.top - 1;
        *destReg = *res;
        s->m_apistack.top = res;
      }
      break;
    }

    case Opcode::NEWTABLE: {
      hksUint32 arraySize = floatingpointbyte2uint(opB);
      hksUint32 hashSize = floatingpointbyte2uint(opC);

      HashTable *newTable = HashTable::Create(s, arraySize, hashSize);
      destReg->t = HksObjectType::TTABLE;
      destReg->v.table = newTable;

      opReg = *destReg;
      opReg1 = *destReg;
      consts = closure->m_cache.consts;
      call_m_end = tailcall_m_end;
      break;
    }

    case Opcode::CONCAT: {
      hksInt32 start = opB;
      hksInt32 end = opC;
      hksInt32 count = end - start + 1;

      if (&s->m_apistack.top[count] > s->m_apistack.alloc_top) {
        CallStack::growApiStack(&s->m_callStack, s, count);
        fp = s->m_apistack.base;
      }

      std::memmove(s->m_apistack.top, &fp[start], count * sizeof(HksObject));
      s->m_apistack.top += count;

      hksi_lua_concat(s, count);

      HksObject *res = s->m_apistack.top - 1;
      fp = s->m_apistack.base;
      m_base = fp;
      destReg = &fp[opA];
      *destReg = *res;
      s->m_apistack.top = res;

      consts = closure->m_cache.consts;
      call_m_end = tailcall_m_end;
      break;
    }

    case Opcode::FORPREP: {
      HksNumber startVal = getNumber(s, &destReg[0]);
      HksNumber limitVal = getNumber(s, &destReg[1]);
      HksNumber stepVal = getNumber(s, &destReg[2]);

      HksNumber delta = (limitVal - startVal) / stepVal;
      if (delta < 0.0f || (MAX_SAFE_FLOAT_TO_INT - delta) < 0.0f) {
        delta = FP_ERROR;
      }

      destReg[0].t = HksObjectType::TNUMBER;
      destReg[0].v.number = startVal - stepVal;

      destReg[1].t = HksObjectType::TLIGHTUSERDATA;
      destReg[1].v.raw = static_cast<hksInt32>(delta) + 1;

      destReg[2].t = HksObjectType::TNUMBER;
      destReg[2].v.number = stepVal;

      destReg[3].t = HksObjectType::TNUMBER;
      destReg[3].v.number = startVal;

      PC += opSBx;
      checkInstructionPointer(s, PC, closure->m_method);
      break;
    }

    case Opcode::FORLOOP: {
      hksInt32 nativeVal = destReg[1].v.native;
      HksNumber currentVal = destReg[2].v.number + destReg[0].v.number;

      if (nativeVal > 0) {
        destReg[1].v.raw = static_cast<hksUint32>(nativeVal) - 1;
        PC += opSBx;
        checkInstructionPointer(s, PC, closure->m_method);

        destReg[0].t = HksObjectType::TNUMBER;
        destReg[0].v.number = currentVal;

        destReg[3].t = HksObjectType::TNUMBER;
        destReg[3].v.number = currentVal;
      }
      break;
    }

    case Opcode::TFORLOOP: {
      hksInt32 nresults = opC;

      if (nresults == 2 && destReg->type() == HksObjectType::TCFUNCTION) {
        if (destReg->v.cClosure->m_function == *vm_intern_inext) {
          HksRegister tbl;
          HksObject *nextVal =
              hks_obj_rawgeti(destReg[1].v.table, &tbl,
                              static_cast<hksInt32>(destReg[2].v.number) + 1);

          destReg[4] = *nextVal;
          if (nextVal->type() != HksObjectType::TNIL) {
            destReg[2].v.number += 1.0f;
          } else {
            destReg[2].t = HksObjectType::TNIL;
          }
          destReg[3] = destReg[2];

          if (destReg[3].type() != HksObjectType::TNIL) {
            PC += PC->OpSBx() + 1;
          } else {
            ++PC;
          }
          checkInstructionPointer(s, PC, closure->m_method);
          break;
        } else if (destReg->v.cClosure->m_function == *vm_intern_next) {
          HksRegister keyObj = destReg[2];
          HashTable::Node *nextNode =
              HashTable::getNext(destReg[1].v.table, &keyObj, &destReg[2]);

          if (nextNode->m_key.type() == HksObjectType::TNIL) {
            destReg[2].t = HksObjectType::TNIL;
          }

          destReg[4] = nextNode->m_value;
          destReg[3] = destReg[2];

          if (destReg[3].type() != HksObjectType::TNIL) {
            PC += PC->OpSBx() + 1;
          } else {
            ++PC;
          }
          checkInstructionPointer(s, PC, closure->m_method);
          break;
        } else if (destReg->v.cClosure->m_function == *hks_next_placeholder) {
          HashTable::hksNext(destReg[1].v.table, &destReg[2], &destReg[3],
                             &destReg[4]);
          if (destReg[2].type() == HksObjectType::TNIL) {
            destReg[3].t = HksObjectType::TNIL;
          }

          if (destReg[3].type() != HksObjectType::TNIL) {
            PC += PC->OpSBx() + 1;
          } else {
            ++PC;
          }
          checkInstructionPointer(s, PC, closure->m_method);
          break;
        }
      }

      // Fallback generic TFORLOOP implementation
      if (&s->m_apistack.top[3] > s->m_apistack.alloc_top) {
        CallStack::growApiStack(&s->m_callStack, s, 3);
        fp = s->m_apistack.base;
        destReg = &fp[opA];
      }

      HksObject *opStack = s->m_apistack.top;
      opStack[0] = destReg[0];
      opStack[1] = destReg[1];
      opStack[2] = destReg[2];
      s->m_apistack.top += 3;

      hksi_lua_call(s, 2, nresults, PC);

      fp = s->m_apistack.base;
      m_base = fp;
      destReg = &fp[opA];

      if (nresults - 1 >= 0) {
        HksObject *top = s->m_apistack.top;
        HksObject *storePtr = &destReg[nresults + 2];
        for (int i = 0; i < nresults; ++i) {
          --top;
          --storePtr;
          *storePtr = *top;
        }
        s->m_apistack.top = top;
      }

      destReg[2] = destReg[3];

      if (destReg[3].type() != HksObjectType::TNIL) {
        PC += PC->OpSBx() + 1;
      } else {
        ++PC;
      }
      checkInstructionPointer(s, PC, closure->m_method);
      break;
    }

    case Opcode::SETLIST: {
      HashTable *tbl = destReg->v.table;
      hksUint32 insertLen = opB;
      hksUint32 nextOpStackIdxInc = opC;

      if (nextOpStackIdxInc == 0) {
        ++PC;
        nextOpStackIdxInc = opBx;
        checkInstructionPointer(s, PC, closure->m_method);
      }

      hksUint32 insertIdx = 50 * nextOpStackIdxInc - 50;

      if (insertLen == 0) {
        insertLen = (reinterpret_cast<char *>(s->m_apistack.top) -
                     reinterpret_cast<char *>(destReg) - sizeof(HksObject)) /
                    sizeof(HksObject);
        HashTable::setArraySize(tbl, s, insertIdx + insertLen);
      }

      HashTable::arrayInserts(tbl, s, insertIdx + 1, insertIdx + insertLen,
                              destReg + 1);

      HksObject *top = s->m_apistack.top;
      HksObject *endLimit =
          &s->m_apistack.base[closure->m_cache.m_numRegisters];
      for (; top < endLimit; ++top) {
        top->t = HksObjectType::TNIL;
      }
      s->m_apistack.top = endLimit;

      consts = closure->m_cache.consts;
      call_m_end = tailcall_m_end;
      break;
    }

    case Opcode::CLOSE: {
      closePendingUpvalues(s, destReg);
      consts = closure->m_cache.consts;
      call_m_end = tailcall_m_end;
      break;
    }

    case Opcode::CLOSURE: {
      HksClosure *newClosure = createHksClosure(
          s, closure->m_method->children.data[opBx], closure->m_env);

      destReg->t = HksObjectType::TIFUNCTION;
      destReg->v.closure = newClosure;

      if (newClosure->m_method->num_upvals != 0) {
        for (hksUint32 i = 0; i < newClosure->m_method->num_upvals; ++i) {
          newClosure->m_upvalues[i] = nullptr;
        }

        for (hksUint32 i = 0; i < newClosure->m_method->num_upvals; ++i) {
          ++PC;
          checkInstructionPointer(s, PC, closure->m_method);

          UpValue *pending = nullptr;

          if (opA == 1) { // Local upvalue binding
            pending = s->pending;
            // opBx == argIdx
            while (pending != nullptr && pending->loc != &fp[opBx]) {
              pending = pending->m_next;
            }

            if (pending == nullptr) {
              closure->m_mayHaveUpvalues = 1;
              pending = reinterpret_cast<UpValue *>(
                  getMemory(s, 0x30, AllocTypes::Upvalue));
              pending->m_storage.t = HksObjectType::TNIL;
              pending->m_storage.v.ptr = nullptr;
              pending->loc = &fp[opBx];
              pending->m_next = s->pending;

              if (s->m_global->m_collector.m_phase == 1) {
                pending->m_flags |= 1;
              }
              s->pending = pending;
            }
          } else { // Inherited upvalue binding
            pending = closure->m_upvalues[opBx];
          }

          if (s->m_global->m_collector.m_phase == 1) {
            GarbageCollector::writeBarrier(&s->m_global->m_collector,
                                           newClosure, pending->loc);
          }
          newClosure->m_upvalues[i] = pending;
        }
      }
      consts = closure->m_cache.consts;
      call_m_end = tailcall_m_end;
      break;
    }

    case Opcode::VARARG: {
      HksObject *top = s->m_apistack.top;
      hksUint32 currentNumVarargs = s->m_callStack.m_current->m_numVarargs;
      hksUint32 copyCount = (std::min)(opB, currentNumVarargs);

      hksInt64 extraSpace =
          ((reinterpret_cast<char *>(destReg) + copyCount * sizeof(HksObject)) -
           reinterpret_cast<char *>(top)) /
          sizeof(HksObject);
      if (extraSpace > 0 && &top[extraSpace] > s->m_apistack.alloc_top) {
        CallStack::growApiStack(&s->m_callStack, s, extraSpace);
        fp = s->m_apistack.base;
        m_base = fp;
        destReg = &fp[opA];
        consts = closure->m_cache.consts;
        call_m_end = tailcall_m_end;
      }

      HksObject *varargBase =
          &fp[-static_cast<hksInt32>(currentNumVarargs) - 1];
      for (hksUint32 i = 0; i < copyCount; ++i) {
        destReg[i] = varargBase[i];
      }

      // opB == expectedNumArgs
      for (hksUint32 c = copyCount; c < opB; ++c) {
        destReg[c].t = HksObjectType::TNIL;
      }

      if (opB == 0) {
        s->m_apistack.top = &destReg[copyCount];
      }
      break;
    }

    default:
      break;
    }
  }
}

luaReturnCount_e executeShared(lua_State *s, const hksInstruction *PC,
                               hksInt32 call_depth) {
  // TODO: actual impl
  return execute1(s, PC, call_depth);
}
luaReturnCount_e executeIsolated(lua_State *s, const hksInstruction *PC,
                                 hksInt32 call_depth) {
  // TODO: actual impl
  return execute0(s, PC, call_depth);
}

} // namespace hks
} // namespace lua
} // namespace ui
} // namespace game