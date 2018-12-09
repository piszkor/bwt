#include "BWT.hh"
#include <algorithm>
#include <numeric>

std::tuple<std::string, std::vector<size_t>>
BWT::transform(const std::string &_data) {
  std::string data = char(2) + _data;
  std::vector<size_t> sorted;
  sorted.resize(data.size());
  std::iota(sorted.begin(), sorted.end(), 0);
  std::sort(sorted.begin(), sorted.end(),
            [&data](const size_t &lhs, const size_t &rhs) {
              return data.substr(lhs) + data.substr(0, lhs) <
                     data.substr(rhs) + data.substr(0, rhs);
            });

  std::vector<size_t> indexes_for_sorted;
  indexes_for_sorted.resize(data.size());
  for (auto i = 0; i < sorted.size(); ++i) {
    indexes_for_sorted.at(sorted.at(i)) = i;
  }

  std::vector<size_t> indexes;
  indexes.reserve(data.size());
  for (const auto &i : sorted) {
    indexes.push_back(indexes_for_sorted.at(i == data.size() - 1 ? 0 : i + 1));
  }

  return std::tuple<std::string, std::vector<size_t>>{
      std::accumulate(sorted.begin(), sorted.end(), std::string(""),
                      [&data](std::string accumlator, const size_t &rhs) {
                        return std::move(accumlator) +
                               data.at(rhs == 0 ? data.size() - 1 : rhs - 1);
                      }),
      indexes};
}

std::string BWT::inverse_transform(const std::string &transformed,
                                   const std::vector<size_t> &indexes) {
  std::string result = "";
  auto index = indexes.at(*indexes.begin()); // drops the deliminiter
  for (auto i = 1; i < transformed.size(); ++i) {
    result.push_back(transformed.at(index));
    index = indexes.at(index);
  }
  return result;
}
