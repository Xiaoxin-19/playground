#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

// 定义整数序列模板
template <class T, T... Ints>
struct integer_sequence
{
};

// 特例化 interger_sequence
template <size_t... Ints>
using index_sequence = ::integer_sequence<size_t, Ints...>;

// 递归处理，序列生成
template <size_t N, size_t... Ints>
struct index_sequence_helper
{
  using type = typename index_sequence_helper<N - 1, N - 1, Ints...>::type;
};

// 递归出口
template <size_t... Ints>
struct index_sequence_helper<0, Ints...>
{
  using type = index_sequence<Ints...>;
};

// 生成序列
template <size_t N>
using make_index_sequence = typename ::index_sequence_helper<N>::type;

template <class F, class Tuple, size_t... I>
auto my_apply_impl(F &&f, Tuple &&t, index_sequence<I...>)
    -> decltype(f(std::get<I>(std::forward<Tuple>(t))...))
{
  return f(std::get<I>(std::forward<Tuple>(t))...);
}

template <class F, class Tuple>
auto my_apply(F &&f, Tuple &&t) -> decltype(my_apply_impl(std::forward<F>(f), std::forward<Tuple>(t), ::make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>{}))
{
  return my_apply_impl(std::forward<F>(f), std::forward<Tuple>(t), ::make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>{});
}

int main()
{
  auto t = std::make_tuple(1, 2.0, "hello");
  auto f = [](int i, double d, const char *s)
  { std::cout << i << ", " << d << ", " << s << std::endl;  return 9999; };
  std::cout << my_apply(f, t) << std::endl;
  return 0;
}
