#include "BWT.hh"
#include <algorithm>
#include <cmath>
#include <list>
#include <numeric>
#include <parallel/algorithm>

#include <iostream>

namespace {
size_t pow(size_t base, size_t p) {
  size_t res = 1;
  for (size_t i = 0; i < p; ++i)
    res = res * base;
  return res;
}

std::vector<size_t> createIndexTable(const std::vector<size_t> &data) {
  std::vector<size_t> index;
  index.resize(data.size());
  for (auto i = 0; i < data.size(); ++i) {
    index.at(data.at(i)) = i;
  }
  return index;
}

/*length < str.size()*/
std::string cycl_substr(const std::string &str, size_t left, size_t length) {
  return left + length < str.size() ? str.substr(left, length)
                      : str.substr(left) + str.substr(0, length - (str.size() - left));
}

void updateSeparatorSection(const std::string &data,
                            const std::vector<size_t> &sorted,
                            std::list<size_t> &separators, size_t depth_i,
                            std::list<size_t>::iterator left_bound,
                            std::list<size_t>::iterator right_bound) {
  auto base = depth_i == 0 ? 0 : pow(2, depth_i - 1);
  auto depth  = depth_i == 0 ? 1 : base;
  // std::cout << base << "b" << depth << "d" <<std::endl;
  auto lastOfTheSame =
      cycl_substr(data, (sorted.at(*left_bound) + base) % data.size(), depth);
  // std::cout << lastOfTheSame << base << "b" << depth << "d" <<std::endl;
  for (auto i = (*left_bound) + 1; i != *right_bound; ++i) {
    auto current = cycl_substr(data, (sorted.at(i) + base) % data.size(), depth);
    // std::cout << "lots " << lastOfTheSame << "\ncurr " <<current << std::endl;
    if (current != lastOfTheSame) {
      // std::cout << i <<std::endl;
      separators.insert(right_bound, i);
      lastOfTheSame = current;
    }
  }
  // std::cout <<"section_"<<depth_i<<std::endl;
}

void updateSeparators(const std::string &data,
                      const std::vector<size_t> &sorted,
                      std::list<size_t> &separators, size_t depth_i) {
  auto curr = separators.begin();
  auto prev = curr++;
  for (; curr != separators.end(); prev = curr++) {
  // std::cout <<"sepupdate_" << *curr<<std::endl;
    if (*prev + 1 != *curr) {
      // std::cout<< "wut" << std::endl;
      updateSeparatorSection(data, sorted, separators, depth_i, prev, curr);
    }
  }
  // std::cout <<"sepupdate_"<<std::endl;
}
}

std::tuple<std::string, std::vector<size_t>>
BWT::transform(const std::string &_data) {
  const std::string data = "^" + _data;
  std::vector<size_t> index, sorted;

  sorted.resize(data.size());
  std::iota(sorted.begin(), sorted.end(), 0);

  { // order by 1st char
    std::sort(sorted.begin(), sorted.end(),
              [&](const auto &lhs, const auto &rhs) {
                return data.at(lhs) < data.at(rhs);
              });
    index = createIndexTable(sorted);
  }

  // create separators
  std::list<size_t> separators;
  separators.push_back(0);
  separators.push_back(data.size());
  
  size_t step = 0;
  for (size_t depth = 1; depth < data.size(); depth = depth * 2) {
    // std::cout <<"???" << std::endl;
    updateSeparators(data, sorted, separators, step++);
    auto right_it = separators.begin();
    auto left_it = right_it++;
    // std::cout <<"???" << std::endl;
    for (; right_it != separators.end(); left_it = right_it++) {
      auto l = sorted.begin();
      std::advance(l, *left_it);
      auto r = sorted.begin();
      std::advance(r, *right_it);

      std::sort(l, r, [&](const auto &lhs, const auto &rhs) {
        return index.at((lhs + depth) % data.size()) <
               index.at((rhs + depth) % data.size());
      });
    }
    // std::cout <<"???" << std::endl;
    index = createIndexTable(sorted);
  }

  std::vector<size_t> indexes;
  indexes.reserve(data.size());
  for (const auto &i : sorted) {
    indexes.push_back(index.at(i == data.size() - 1 ? 0 : i + 1));
  }
/*
  for(auto i : sorted)
  {
    std::cout << cycl_substr(data, i, data.size()) << std::endl;
  }*/

  return std::tuple<std::string, std::vector<size_t>>{
      std::accumulate(sorted.begin(), sorted.end(), std::string(""),
                      [&](auto accumlator, const auto &rhs) {
                        return std::move(accumlator) +
                               data.at(rhs == 0 ? data.size() - 1 : rhs - 1);
                      }),
      indexes};
}

std::string BWT::inverse_transform(const std::string &transformed,
                                   const std::vector<size_t> &indexes) {
  std::string result = "";
  auto index = *indexes.begin(); // drops the deliminiter
  for (auto i = 0; i < transformed.size(); ++i) {
    result.push_back(transformed.at(index));
    index = indexes.at(index);
  }
  return result;
}
