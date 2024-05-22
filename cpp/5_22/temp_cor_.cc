#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

template <class T, T... Ints>
struct integer_sequence
{
};

template <size_t... Ints>
using index_sequence = ::integer_sequence<size_t, Ints...>;

// Helper to create index_sequence
template <size_t N, size_t... Ints>
struct index_sequence_helper
{
  using type = typename ::index_sequence_helper<N - 1, N - 1, Ints...>::type;
};

template <size_t... Ints>
struct index_sequence_helper<0, Ints...>
{
  using type = ::index_sequence<Ints...>;
};

template <size_t N>
using make_index_sequence = typename ::index_sequence_helper<N>::type;

// Helper function to apply the function to the tuple elements
template <class F, class Tuple, size_t... I>
auto my_apply_impl(F &&f, Tuple &&t, ::index_sequence<I...>)
    -> decltype(f(std::get<I>(std::forward<Tuple>(t))...))
{
  return f(std::get<I>(std::forward<Tuple>(t))...);
}

// Main my_apply function
template <class F, class Tuple>
void my_apply(F &&f, Tuple &&t)
{
  constexpr size_t size = std::tuple_size<typename std::decay<Tuple>::type>::value;
  my_apply_impl(std::forward<F>(f), std::forward<Tuple>(t), make_index_sequence<size>{});
}

int main()
{
  auto t = std::make_tuple(1, 2.0, "hello");
  auto f = [](int i, double d, const char *s)
  {
    std::cout << i << ", " << d << ", " << s << std::endl;
  };
  my_apply(f, t);
  my_apply([](std::string name, int age)
           { std::cout << "your name is : " << name << " your age is : " << age << std::endl; }, std::make_tuple("he_wenxin", 21));
  return 0;
}
