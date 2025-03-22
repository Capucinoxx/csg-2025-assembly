#pragma once

#include <tuple>
#include <optional>
#include <variant>
#include <algorithm>
#include "consts.h"
#include "statement.h"
#include "instructions/primitives_instructions.h"

namespace instruction {
using statement_t = statement::statement_t;

constexpr auto s_if = pipeline_stage::FETCH;
constexpr auto s_id = pipeline_stage::DECODE;
constexpr auto s_ex = pipeline_stage::EXECUTE;
constexpr auto s_mem = pipeline_stage::MEMORY_ACCESS;
constexpr auto s_wb = pipeline_stage::WRITE_BACK;

struct add_t : public ternary_primitive_t<add_t> {
  using ternary_primitive_t<add_t>::ternary_primitive_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if | s_id | s_ex | s_wb);

  std::string mnemonic_id() { return "ADD"; }
  static auto mnemonic() { return "ADD"_lit; }
  static auto apply_operation(int lhs, int rhs) { return lhs + rhs; }
};

struct jle_t : public ternary_label_t<jle_t> {
  using ternary_label_t<jle_t>::ternary_label_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if | s_id | s_ex | s_wb);

  std::string mnemonic_id() { return "JLE"; }
  static auto mnemonic() { return "JLE"_lit; }
  static auto apply_operation(int lhs, int rhs) { return lhs <= rhs; }
};

struct mov_t : public binary_primitive_t<mov_t> {
  using binary_primitive_t<mov_t>::binary_primitive_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if | s_id | s_wb);

  std::string mnemonic_id() { return "MOV"; }
  static auto mnemonic() { return "MOV"_lit; }
  static auto apply_operation(int src) { return src; }
};

struct ldr_t : public ternary_primitive_t<ldr_t> {
  using ternary_primitive_t<ldr_t>::ternary_primitive_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if | s_id | s_ex | s_mem | s_wb);

  std::string mnemonic_id() { return "LDR"; }
  static auto mnemonic() { return "LDR"_lit; }
  static auto apply_operation(reg_t lhs, reg_t rhs) { return rhs; }

  void execute_memory_access_impl(statement_t& s, operands_t ops) {
    reg_t result = 0;
    for (size_t i = 0; i < rhs && i < 4; ++i) 
      result |= (*s.mem)[lhs + i] << (i * 8);
    rhs = result;
  }
};

struct str_t : public binary_primitive_t<str_t> {
  using binary_primitive_t<str_t>::binary_primitive_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if | s_id | s_ex | s_mem);

  std::string mnemonic_id() { return "STR"; }
  static auto mnemonic() { return "STR"_lit; }
  static auto apply_operation(int src) { return src; }
    
  void execute_memory_access_impl(statement_t& s, operands_t ops) {
    s.map->set(lhs, rhs);
  }
};

struct inc_t : public unary_primitive_t<inc_t> {
  using unary_primitive_t<inc_t>::unary_primitive_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if | s_id | s_ex | s_wb);

  std::string mnemonic_id() { return "INC"; }
  static auto mnemonic() { return "INC"_lit; }
  static auto apply_operation(int src) { return src + 1; }
};

struct zer_t : public unary_primitive_t<zer_t> {
  using unary_primitive_t<zer_t>::unary_primitive_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if | s_wb);

  std::string mnemonic_id() { return "ZER"; }
  static auto mnemonic() { return "ZER"_lit; }
  static auto apply_operation(int src) { return 0; }

  void execute_write_back_impl(statement_t& s, operands_t ops) {
    dst = std::get<parser::register_t>(ops[0]).id;
    s.regs.x[dst] = 0;
  }
};

struct top_t : public unary_primitive_t<top_t> {
  using unary_primitive_t<top_t>::unary_primitive_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if | s_mem | s_wb);

  std::string mnemonic_id() { return "TOP"; }
  static auto mnemonic() { return "TOP"_lit; }
  static auto apply_operation(int src) { return src; }
    
  void execute_memory_access_impl(statement_t& s, operands_t ops) {
    dst = std::get<parser::register_t>(ops[0]).id;
    src = s.stack.top();
  }
};

struct pop_t : public unary_primitive_t<pop_t> {
  using unary_primitive_t<pop_t>::unary_primitive_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if | s_mem | s_wb);

  std::string mnemonic_id() { return "POP"; }
  static auto mnemonic() { return "POP"_lit; }
  static auto apply_operation(int src) { return src; }
    
  void execute_memory_access_impl(statement_t& s, operands_t ops) {
    dst = std::get<parser::register_t>(ops[0]).id;
    src = s.stack.pop();
  }
};

struct push_t : public unary_primitive_t<push_t> {
  using unary_primitive_t<push_t>::unary_primitive_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if | s_id | s_mem);

  std::string mnemonic_id() { return "PUSH"; }
  static auto mnemonic() { return "PUSH"_lit; }
  static auto apply_operation(int src) {return src; }

  void execute_memory_access_impl(statement_t& s, operands_t ops) {
    s.stack.push(src);
  }
};

struct call_t : public unary_label_t<call_t> {
  using unary_label_t<call_t>::unary_label_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if | s_wb);

  std::string mnemonic_id() { return "CALL"; }
  static auto mnemonic() { return "CALL"_lit; } 
};

struct ret_t : public nullary_primitive_t<ret_t> {
  using nullary_primitive_t<ret_t>::nullary_primitive_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if | s_id | s_wb);

  std::string mnemonic_id() { return "RET"; }
  static auto mnemonic() { return "RET"_lit; }

  reg_t lr;

  void execute_decode_impl(statement_t& s, operands_t ops) {
    lr = s.lr;
  }

  void execute_write_back_impl(statement_t& s, operands_t ops) {
    s.pc = lr - 4;
  }
};

struct swap_t : public nullary_primitive_t<swap_t> {
  using nullary_primitive_t<swap_t>::nullary_primitive_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if | s_mem);

  std::string mnemonic_id() { return "SWAP"; }
  static auto mnemonic() { return "SWAP"_lit; }

  void execute_memory_access_impl(statement_t& s, operands_t ops) {
    s.stack.swap(); 
  }
};

struct nop_t : public nullary_primitive_t<nop_t> {
  using nullary_primitive_t<nop_t>::nullary_primitive_t;
  static constexpr stage_mask_t applicable_stages = 
    static_cast<stage_mask_t>(s_if);

  std::string mnemonic_id() { return "NOP"; }
  static auto mnemonic() { return "NOP"_lit; }
};

struct lbl_t : public label_primitive_t<lbl_t> {
  using label_primitive_t<lbl_t>::label_primitive_t;
  static constexpr stage_mask_t applicable_stages = {};
    
  std::string mnemonic_id() { return std::string{}; }
  static auto mnemonic() { return ""_lit; }

  std::string name() const { 
    return std::get<parser::label_t>(operands[0]).name;
  }
};
};


namespace instruction {
struct match {
  template<typename T>
  struct variant_wrapper;

  template<typename... Ts>
  struct variant_wrapper<std::tuple<Ts...>> { 
    using type = std::variant<Ts...>; };

  template<typename T>
  using variant_wrapper_t = typename variant_wrapper<T>::type;

  using instructions = std::tuple<
    mov_t,
    add_t, inc_t, zer_t, top_t,
    push_t, pop_t, jle_t, ldr_t, str_t,
    nop_t, swap_t,
    lbl_t, call_t, ret_t
  >;
  using variant_instructions = variant_wrapper_t<instructions>;
  using op_variant_ins = std::optional<variant_instructions>;

  op_variant_ins operator()(iterator& it, iterator end) const {
    op_variant_ins res;

    std::apply(
      [&](auto ... instr) {
        ((res ? void() : [&]() {
          auto validate = decltype(instr)::validate;
          if (auto [ok, derived] = validate(it, end); ok)
            res = variant_instructions(derived);
        }()), ...);
      }, instructions{}
    );

    return res;
  };
};
    
constexpr auto parse = [](iterator& it, iterator end) -> auto {
  return match{}(it, end);
};
};
