#include "wait.hpp"

void ztd::wait_pool::wait()
{
  std::unique_lock<std::mutex> lck(m_mtx);
  m_cv.wait(lck);
}

void ztd::wait_pool::notify_one()
{
  m_cv.notify_one();
}

void ztd::wait_pool::notify_all()
{
  m_cv.notify_all();
}
