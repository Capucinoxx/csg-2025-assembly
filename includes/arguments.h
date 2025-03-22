#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <unordered_set>
#include <sstream>
#include <ranges>
#include <vector>

#include "consts.h"

struct arguments_t {
  std::string program_file;
  std::string manual_file;
  std::unordered_set<reg_t> breakpoints;

  static auto parse(int argc, char** argv) -> arguments_t {
    arguments_t args;
    args.program_file = "program.asm";
    args.manual_file = "manual.txt";

    std::vector<std::string> arguments(argv + 1, argv + argc);

    auto match = [&](const std::string& flag) {
      return std::ranges::find(arguments, flag) != arguments.end(); };

    if (match("-h") || match("--help")) {
      std::cout << usage() << std::endl;
      std::exit(0);
    }
      
    auto val = [&](const std::string& flag) 
      -> std::optional<std::string> {
      auto it = std::ranges::find(arguments, flag);
      if (it != arguments.end() && std::next(it) != arguments.end())
        return *std::next(it);
      return std::nullopt;
    };

    if (auto v = val("-p")) 
      args.program_file = *v;
    
    if (auto v = val("-m")) 
      args.manual_file = *v;

    if (auto v = val("-b")) {
      std::vector<unsigned int> bps;
      for (auto&& rng : *v | std::views::split(','))
        bps.push_back(std::stoi(std::string(rng.begin(), rng.end())));
      args.breakpoints = std::unordered_set<unsigned int>(bps.begin(), 
                                                          bps.end());
    }

    return args;
  }

  static auto open_file(const std::string& path) -> std::ifstream {
    std::ifstream file(path);
    if (!file.is_open()) {
      std::cerr << "Error: could not open file " << path << std::endl;
      std::exit(1);
    }
    return file;
  }

  static auto usage() -> std::string {
    std::ostringstream oss;
    oss << "Usage: ./asm [options]\n"
        << "Options:\n"
        << "  -h, --help    Show this help message\n"
        << "  -p            Specify the program file\n"
        << "  -m            Specify the manual file\n"
        << "  -b            Specify breakpoints bp1,bp2,...\n"
        << "Example:\n"
        << "./asm -p program.asm -m manual.txt -b 42,172\n";
    return oss.str();
  }

};
