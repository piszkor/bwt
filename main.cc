#include "BWT.hh"
#include <fstream>
#include <iostream>

int main(int argc, char const *argv[]) {
  std::string origin, line;
  if (argc != 2) {
    std::cerr << "ERROR: Usage sould be: " << argv[0] << " /path/to/file "
              << std::endl;
    return 1;
  }
  std::ifstream input_file(argv[1]);
  while (!input_file.eof()) {
    std::getline(input_file, line);
    origin.append(line);
  }
  std::string transformed;
  std::vector<size_t> indexes;
  std::tie(transformed, indexes) = BWT::transform(origin);

  std::string inv = BWT::inverse_transform(transformed, indexes);

  std::cout << "Origin: " << origin << std::endl
            << "Transformed: " << transformed << std::endl
            << "Inv: " << inv << std::endl;

  return 0;
}
