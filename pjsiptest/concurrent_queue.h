#pragma once

#include <queue>

template<typename Data>
class concurrent_queue
{
private:
  std::queue<Data> the_queue;
  mutable boost::mutex the_mutex;

public:

  void push(Data const& data)
  {
    boost::mutex::scoped_lock lock(the_mutex);
    the_queue.push(data);
  }

  bool empty() const
  {
    boost::mutex::scoped_lock lock(the_mutex);
    return the_queue.empty();
  }

  bool try_pop(Data& popped_value)
  {
    boost::mutex::scoped_lock lock(the_mutex);
    if(the_queue.empty())
    {
      return false;
    }

    popped_value=the_queue.front();
    the_queue.pop();
    return true;
  }

};