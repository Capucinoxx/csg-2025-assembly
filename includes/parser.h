#pragma once

#include <cctype>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include "consts.h"

namespace parser {
using iterator = std::string::const_iterator;

struct register_t { int id; };
struct immediate_t { int value; };
struct label_t { std::string name; };

using element_t =
  std::variant<std::monostate, register_t, immediate_t, label_t>;

struct element_printer {
  std::ostream &os;
  element_printer(std::ostream &os) : os(os) {}

  void operator()(const std::monostate &) const { 
    os << "Empty "; }
  void operator()(const register_t &reg) const { 
    os << "R" << reg.id << " "; }
  void operator()(const immediate_t &imm) const {
    os << "#" << imm.value << " "; }
  void operator()(const label_t &label) const { 
    os << label.name << " "; }
};

std::ostream &operator<<(std::ostream &os, const element_t &elem) {
  std::visit(element_printer{os}, elem);
  return os;
}

constexpr element_t make_register(int id) { 
  return register_t{id}; }
constexpr element_t make_immediate(int value) { 
  return immediate_t{value}; }
constexpr element_t make_label(std::string name) { 
  return label_t{name}; }

template <typename F> struct generic_parser {
  F fn;

  constexpr explicit generic_parser(F f) : fn(f) {}
  constexpr bool operator()(iterator &it, iterator end) const {
    return fn(it, end); }
};

template <typename F> constexpr auto make_parser(F f) {
  return generic_parser<F>{f};
}

constexpr auto operator""_lit(const char *str, size_t) {
  return make_parser([str](iterator &it, iterator end) {
    for (size_t i = 0; str[i]; ++i, ++it) {
      if (it == end || *it != str[i])
        return false;
    }
    return true;
  });
}

constexpr auto operator""_symb(char c) {
  return make_parser([c](iterator &it, iterator end) {
    if (it != end && *it == c) {
      ++it;
      return true;
    }
    return false;
  });
}

constexpr auto ws = make_parser([](iterator &it, iterator end) {
  while (it != end && std::isspace(*it)) ++it;
  return true;
});

template <typename... P> 
constexpr auto combine(P... parsers) {
  return ws && (... && (parsers && ws));
}

template <typename P1, typename P2> 
struct and_parser_t {
  P1 p1;
  P2 p2;

  constexpr and_parser_t(P1 p1, P2 p2) : p1(p1), p2(p2) {}
  constexpr bool operator()(iterator &it, iterator end) const {
    auto original_it = it;
    if (p1(it, end) && p2(it, end)) return true;
    it = original_it;
    return false;
  }
};

template <typename P1, typename P2> 
constexpr auto operator&&(P1 p1, P2 p2) {
  return and_parser_t<P1, P2>{p1, p2};
}

template <typename P1, typename P2> 
struct or_parser_t {
  P1 p1;
  P2 p2;

  constexpr or_parser_t(P1 p1, P2 p2) : p1(p1), p2(p2) {}
  constexpr bool operator()(iterator &it, iterator end) const {
    auto original_it = it;
    if (p1(it, end))
      return true;
    it = original_it;
    return p2(it, end);
  }
};

template <typename P1, typename P2> 
constexpr auto operator||(P1 p1, P2 p2) {
  return or_parser_t<P1, P2>{p1, p2};
}

constexpr auto optional = [](auto parser) {
  return make_parser([parser](iterator &it, iterator end) {
    parser(it, end);
    return true;
  });
};

constexpr auto parse_register = [](element_t &ref) -> auto {
  return make_parser([&ref](iterator &it, iterator end) {
    if (it == end || *it++ != 'R')
      return false;

    if (it == end || !std::isdigit(*it))
      return false;
    int reg_id = *it++ - '0';

    if (reg_id > (n_regs - 1))
      return false;
    ref = make_register(reg_id);
    return true;
  });
};

constexpr auto parse_imm = [](element_t &ref) -> auto {
  return make_parser([&ref](iterator &it, iterator end) {
    if (it == end || *it++ != '0' || it == end || *it++ != 'x')
      return false;

    int value = 0;
    while (it != end && std::isxdigit(*it)) {
      value = value * 16 + (*it++ - '0');
    }

    ref = make_immediate(value);
    return true;
  });
};

constexpr auto parse_str = [](element_t &ref) -> auto {
  return make_parser([&ref](iterator &it, iterator end) {
    if (it == end || !std::isalpha(*it))
      return false;

    std::string name;
    while (it != end && (std::isalnum(*it) || *it == '_')) {
      name.push_back(*it++);
    }

    if (name.empty())
      return false;

    ref = make_label(name);

    return true;
  });
};

std::string prepare_input(const std::string &input) {
  auto trim = [](const std::string &str) -> std::string {
    std::size_t first = str.find_first_not_of(" \t");
    std::size_t last = str.find_last_not_of(" \t");
    return (first == std::string::npos) 
      ? "" : str.substr(first, last - first + 1);
  };

  auto remove_comments = [](const std::string &str) -> std::string {
    std::size_t pos = str.find(';');
    return pos == std::string::npos ? str : str.substr(0, pos);
  };

  return trim(remove_comments(input));
}
} // namespace parser
