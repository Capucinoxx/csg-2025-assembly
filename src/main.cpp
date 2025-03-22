#include <iostream>
#include <fstream>
#include <memory>

#include "lego/block.h"
#include "instructions/instructions.h"
#include "visualizer/visualizer.h"
#include "visualizer/help.h"
#include "pipeline/pipeline.h"
#include "sandbox/sandbox.h"
#include "statement.h"
#include "arguments.h"


using namespace pipeline;
using namespace instruction;


auto init_sandbox(const arguments_t& args) -> std::shared_ptr<sandbox_t> {
  auto sandbox = std::make_shared<sandbox_t>();
  sandbox->load(
    std::move(arguments_t::open_file(args.program_file)),
    args.breakpoints);

  if (!sandbox->errors.empty()) {
    for (const auto& error : sandbox->errors) 
      std::cerr << error << std::endl;
    return nullptr;
  }

  return sandbox;
}

void load_manual(const arguments_t& args, std::shared_ptr<sandbox_t> sandbox) {
  blocks_t blocks;
  blocks.load(std::move(arguments_t::open_file(args.manual_file)));
  blocks.store(sandbox->stmt->mem);
}

void start_sandbox(std::shared_ptr<sandbox_t> sandbox) {
  visualizer::visualizer_t visualizer{};
  visualizer.setup_panels(
    sandbox,
    sandbox->stmt,
    sandbox->stmt->mem,
    sandbox->stmt->map,
    sandbox->pipeline,
    help_t::get());

  visualizer.hook('s', [&sandbox, &visualizer]() {
    try {
      sandbox->stepi();
    } catch (const std::exception& e) {
      visualizer.display_error(e.what());
    }
  });

  visualizer.hook('c', [&sandbox, &visualizer]() {
    try {
      sandbox->stepi_until_breakpoint();
    } catch(const std::exception& e) {
      visualizer.display_error(e.what());
    }
  });

  visualizer.run();
}

int main(int argc, char** argv) {
  auto args = arguments_t::parse(argc, argv);

  auto sandbox = init_sandbox(args);
  if (!sandbox) return 1;

  load_manual(args, sandbox);
  start_sandbox(sandbox);


  //sandbox->stepi_until_breakpoint();
  //sandbox->print();
  return 0;
}
