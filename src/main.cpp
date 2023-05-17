#include <ledger.h>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <num_of_threads> <leader_file>\n";
    exit(-1);
  }

  InitBank(atoi(argv[1]), argv[2]);

  return 0;
}
