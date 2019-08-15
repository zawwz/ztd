#include "zthread.hpp"

ztd::thread_pool::~thread_pool()
{
  for(auto it : m_threads)
  {
    delete it;
  }
}

void ztd::thread_pool::join()
{
  for(auto it : m_threads)
  {
    it->join();
    delete it;
  }
  m_threads.clear();
}

void ztd::thread_pool::detach()
{
  for(auto it : m_threads)
  {
    it->detach();
  }
}

void ztd::thread_pool::clear()
{
  for(auto it : m_threads)
  {
    delete it;
  }
  m_threads.clear();
}
