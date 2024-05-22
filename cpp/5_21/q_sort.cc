#include <iostream>
#include <list>
#include <algorithm>
#include <future>

template <typename T>
std::list<T> serial_q_sort(std::list<T> input)
{
  if (input.empty())
  {
    return input;
  }
  std::list<T> result;
  result.splice(result.begin(), input, input.begin());
  T const &pivot = *result.begin();

  auto divide_point = std::partition(input.begin(), input.end(), [&](T const &t)
                                     { return t < pivot; });

  std::list<T> lower_part;
  lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
  auto new_lower(serial_q_sort(std::move(lower_part)));
  auto new_higher(serial_q_sort(std::move(input)));
  result.splice(result.end(), new_higher);
  result.splice(result.begin(), new_lower);
  return result;
}

template <typename T>
std::list<T> parallel_q_sort(std::list<T> input)
{
  if (input.empty())
  {
    return input;
  }

  std::list<T> result;
  result.splice(result.begin(), input, input.begin());
  T const &pivot = *result.begin();
  auto divide_point = std::partition(input.begin(), input.end(), [&](T const &t)
                                     { return t < pivot; });

  std::list<T> lower_part;
  lower_part.splice(lower_part.begin(), input, input.begin(), divide_point);
  // 这里两个函数并行计算
  std::future<std::list<T>> new_lower(std::async(&parallel_q_sort<T>, std::move(lower_part)));
  auto new_higher(parallel_q_sort(std::move(input)));
  result.splice(result.end(), new_higher);
  result.splice(result.begin(), new_lower.get());
  return result;
}