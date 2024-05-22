#include <iostream>
#include <future>
int find_the_answer_to_ltuae()
{
  std::cout << "find ltuae" << std::endl;
  return 88;
}

void do_other_stuff()
{
  std::cout << "do other stuff" << std::endl;
}
int main()
{
  std::future<int> the_answer = std::async(find_the_answer_to_ltuae);
  do_other_stuff();
  std::cout << "the answer is : " << the_answer.get() << std::endl;
  return EXIT_SUCCESS;
}