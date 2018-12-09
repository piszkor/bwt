#include "BWT.hh"
#include <algorithm>
#include <cmath>
#include <list>
#include <numeric>
#include <parallel/algorithm>
#include <thread>

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
  for (size_t i = 0; i < data.size(); ++i) {
    index.at(data.at(i)) = i;
  }
  return index;
}

/*length <= str.size()*/
std::string cycl_substr(const std::string &str, size_t left, size_t length) {
  return left + length < str.size()
             ? str.substr(left, length)
             : str.substr(left) + str.substr(0, length - (str.size() - left));
}

void updateSeparatorSection(const std::string &data,
                            const std::vector<size_t> &sorted,
                            std::list<size_t> &separators, size_t depth_i,
                            std::list<size_t>::iterator left_bound,
                            std::list<size_t>::iterator right_bound) {
  auto base = depth_i == 0 ? 0 : pow(2, depth_i - 1);
  auto depth = depth_i == 0 ? 1 : base;
  auto lastOfTheSame =
      cycl_substr(data, (sorted.at(*left_bound) + base) % data.size(), depth);
  for (auto i = (*left_bound) + 1; i != *right_bound; ++i) {
    auto current =
        cycl_substr(data, (sorted.at(i) + base) % data.size(), depth);
    if (current != lastOfTheSame) {
      separators.insert(right_bound, i);
      lastOfTheSame = current;
    }
  }
}

void updateSeparators(const std::string &data,
                      const std::vector<size_t> &sorted,
                      std::list<size_t> &separators, size_t depth_i) {
  auto curr = separators.begin();
  auto prev = curr++;
  std::vector<std::thread> threads;

  for (; curr != separators.end(); prev = curr++) {
    if (*prev + 1 != *curr) {
      threads.emplace_back(updateSeparatorSection, std::ref(data),
                           std::ref(sorted), std::ref(separators), depth_i,
                           prev, curr);
    }
  }
  for (auto &t : threads) {
    t.join();
  }
}
}

std::tuple<std::string, std::vector<size_t>>
BWT::transform(const std::string &_data) {
  const std::string data = char(2) + _data;
  std::vector<size_t> index, sorted;

  sorted.resize(data.size());
  std::iota(sorted.begin(), sorted.end(), 0);

  // order by 1st char
  __gnu_parallel::sort(sorted.begin(), sorted.end(),
                       [&](const auto &lhs, const auto &rhs) {
                         return data.at(lhs) < data.at(rhs);
                       });

  // create separators
  std::list<size_t> separators;
  separators.push_back(0);
  separators.push_back(data.size());

  size_t step = 0;
  for (size_t depth = 1; depth < data.size(); depth = depth * 2) {
    auto separatorUpdater =
        std::thread(updateSeparators, std::ref(data), std::ref(sorted),
                    std::ref(separators), step++);
    index = createIndexTable(sorted);

    const auto &ind = index;
    const auto cmp = [&data, &ind, depth](const size_t &lhs,
                                          const size_t &rhs) {
      return ind.at((lhs + depth) % data.size()) <
             ind.at((rhs + depth) % data.size());
    };
    separatorUpdater.join();

    std::vector<std::thread> sorting_threads;
    auto right_it = separators.begin();
    auto left_it = right_it++;
    for (; right_it != separators.end(); left_it = right_it++) {
      auto l = sorted.begin();
      std::advance(l, *left_it);
      auto r = l;
      std::advance(r, *right_it - *left_it);
      __gnu_parallel::sort(l, r, cmp);
      /*
            sorting_threads.emplace_back(
                static_cast<void (*)(decltype(l), decltype(r), decltype(cmp))>(
                    __gnu_parallel::sort),
                l, r, cmp);*/
    }

    for (auto &t : sorting_threads)
      t.join();
    // std::cout <<"???" << std::endl;
  }

  index = createIndexTable(sorted);
  std::vector<size_t> indexes;
  indexes.reserve(data.size());
  for (const auto &i : sorted) {
    indexes.push_back(index.at(i == data.size() - 1 ? 0 : i + 1));
  }

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
  auto index = *indexes.begin();
  for (size_t i = 0; i < transformed.size(); ++i) {
    result.push_back(transformed.at(index));
    index = indexes.at(index);
  }
  return result;
}
