#pragma once

#include <string>
#include "consts.h"
#include "statement.h"
#include "instructions/base_instructions.h"

namespace instruction {
using statement_t = statement::statement_t; 

template<typename Derived>
struct ternary_primitive_t : public base_instruction_t<Derived, 3> {
  using base_t = base_instruction_t<Derived, 3>;
  using typename base_t::operands_t;

  using base_t::resolve_operand;
  using base_t::base_t;

  reg_t lhs, rhs, dst;

  static auto parse(iterator& it, iterator end, operands_t& ops) -> bool {
    return combine(Derived::mnemonic(),
                   parse_register(ops[0]), ','_symb,
                   parse_register(ops[1]), ','_symb,
                   parse_register(ops[2]))(it, end);
  }

  void execute_decode_impl(statement_t& s, operands_t ops) {
    dst = std::get<parser::register_t>(ops[0]).id;
    lhs = resolve_operand(s, ops[1]);
    rhs = resolve_operand(s, ops[2]);
  }

  void execute_write_back_impl(statement_t& s, operands_t ops) {
    s.regs.x[dst] = Derived::apply_operation(lhs, rhs);
  }
};

template<typename Derived>
struct ternary_label_t : public base_instruction_t<Derived, 3> {
  using base_t = base_instruction_t<Derived, 3>;
  using typename base_t::operands_t;

  using base_t::resolve_operand;
  using base_t::resolve_label;
  using base_t::base_t;

  reg_t lhs, rhs, lbl;
  bool branch;

  static auto parse(iterator& it, iterator end, operands_t& ops) -> bool {
    return combine(Derived::mnemonic(),
                   parse_register(ops[0]), ','_symb,
                   parse_register(ops[1]), ','_symb,
                   parse_str(ops[2]))(it, end);
  }

  void execute_decode_impl(statement_t& s, operands_t ops) {
    lhs = resolve_operand(s, ops[0]);
    rhs = resolve_operand(s, ops[1]);
    lbl = resolve_label(s, ops[2]);
  }

  void execute_execute_impl(statement_t& s, operands_t ops) {
    branch = Derived::apply_operation(lhs, rhs);
  }

  void execute_write_back_impl(statement_t& s, operands_t ops) {
    if (branch) s.pc = lbl;
  }

  bool branch_taken() const { return branch; }
};


template<typename Derived>
struct binary_primitive_t : public base_instruction_t<Derived, 2> {
  using base_t = base_instruction_t<Derived, 2>;
  using typename base_t::operands_t;

  using base_t::resolve_operand;
  using base_t::base_t;

  reg_t lhs, rhs, dst;

  static auto parse(iterator& it, iterator end, operands_t& ops) -> bool {
    return combine(Derived::mnemonic(),
                   parse_register(ops[0]), ','_symb,
                   parse_register(ops[1]))(it, end);
  }

  void execute_decode_impl(statement_t& s, operands_t ops) {
    lhs = resolve_operand(s, ops[0]);
    rhs = resolve_operand(s, ops[1]);
    dst = std::get<parser::register_t>(ops[0]).id;
  }

  void execute_write_back_impl(statement_t& s, operands_t ops) {
    s.regs.x[dst] = Derived::apply_operation(rhs);
  }
};


template<typename Derived>
struct unary_primitive_t : public base_instruction_t<Derived, 1> {
  using base_t = base_instruction_t<Derived, 1>;
  using typename base_t::operands_t;

  using base_t::resolve_operand;
  using base_t::base_t;

  reg_t dst, src;

  static auto parse(iterator& it, iterator end, operands_t& ops) -> bool {
    return combine(Derived::mnemonic(),
                   parse_register(ops[0]))(it, end);
  }

  void execute_decode_impl(statement_t& s, operands_t ops) {
    dst = std::get<parser::register_t>(ops[0]).id;
    src = s.regs.x[dst];
  }

  void execute_write_back_impl(statement_t& s, operands_t ops) {
    s.regs.x[dst] = Derived::apply_operation(src);
  }
};

template<typename Derived>
struct unary_label_t : public base_instruction_t<Derived, 1> {
  using base_t = base_instruction_t<Derived, 1>;
  using typename base_t::operands_t;

  using base_t::resolve_label;
  using base_t::base_t;

  reg_t lbl;

  static auto parse(iterator& it, iterator end, operands_t& ops) -> bool {
    return combine(Derived::mnemonic(),
                   parse_str(ops[0]))(it, end);
  }

  void execute_write_back_impl(statement_t& s, operands_t ops) {
    lbl = resolve_label(s, ops[0]);
    s.lr = s.pc;
    s.pc = lbl;
  }
};


template<typename Derived>
struct nullary_primitive_t : public base_instruction_t<Derived, 0> {
  using base_t = base_instruction_t<Derived, 0>;
  using typename base_t::operands_t;

  using base_t::base_t;

  static auto parse(iterator& it, iterator end, operands_t& ops) -> bool {
    return combine(Derived::mnemonic())(it, end);
  }
};

template<typename Derived>
struct label_primitive_t : public base_instruction_t<Derived, 1> {
  using base_t = base_instruction_t<Derived, 1>;
  using typename base_t::operands_t;

  using base_t::resolve_label;
  using base_t::base_t;

  static auto parse(iterator& it, iterator end, operands_t& ops) -> bool {
    return combine(parse_str(ops[0]), ':'_symb)(it, end);
  }
};
};
