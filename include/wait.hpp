#ifndef ZTD_WAIT_HPP
#define ZTD_WAIT_HPP

#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

/*! \file wait.hpp
*  @brief Thread stalling and waiting
*/

namespace ztd
{
  //! @brief Thread waiting object
  class wait_pool
  {
  public:

    //! Wait until notified
    void wait();

    //! Notify one instance of wait()
    void notify_one();

    //! Notify all instances of wait()
    void notify_all();

  private:
    std::condition_variable m_cv;
    std::mutex m_mtx;
  };
}

#endif //ZTD_WAIT_HPP
