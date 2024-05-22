#include <iostream>
#include <functional>
#include <unordered_map>
#include <string>
#include <tuple>
#include <utility>
#include <memory>

// 1. 主线程收到一个请求，解析请求
// 2. 根据请求特征，通过工厂方法生成一个对应的可调用对象。
// 3. 将可调用对象加入工作队列
// 4. 工作线程拿到任务之后。直接调用可调用对象即可
// 5. 可调用对象支持不同的类型，不同数量的参数

class EventHalderI
{
public:
  ~EventHalderI() = default;
};

class CreateEvent
{
private:
  std::unordered_map<std::string, EventHalderI> event_map;

public:
  CreateEvent() = default;
  EventHalderI get_event(std::string flag)
  {
    if (event_map.find(flag) == event_map.end())
    {
      return EventHalderI();
    }
    return event_map.at(flag);
  }
};