// TODO: add file description

#include "../include/private/abort_manager.hpp"
#include <iostream>

namespace DualMemoryManager {

// TODO: maybe add MPI
// TODO: add description
void abort_manager(std::string &message) {

  /* make sure standard output is flushed */
  std::cout.flush();

  /* return abort message */
  std::cerr << "DualMemoryManager error: " << message << std::endl;

  /* exit program */
  exit(1);

  return;
}

} // namespace DualMemoryManager
