#ifndef BWT_HH
#define BWT_HH

#include <string>
#include <tuple>
#include <vector>

class BWT {
public:
  BWT() = delete;
  static std::tuple<std::string, std::vector<size_t>>
  transform(const std::string &data);
  static std::string inverse_transform(const std::string &transformed,
                                       const std::vector<size_t> &indexes);
};

#endif