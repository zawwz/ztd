#ifndef ZTHREAD_HPP
#define ZTHREAD_HPP

#include <vector>
#include <thread>

/*! \file zthread.hpp
*  @brief Thread management
*/

namespace ztd
{
  //! @brief Coordinated thread pool
  class thread_pool
  {
  public:
    ~thread_pool();
    //! @brief Join and delete all threads of the pool
    void join();

    //! @brief Detach all threads of the pool
    void detach();

    //! @brief Delete all threads of the pool
    void clear();

    //! @brief Instanciate new thread
    template <class Fn, class... Args>
    void thread(Fn&& fn, Args&&... args)
    {
      m_threads.push_back(new std::thread(fn, args...));
    }

  private:
    std::vector<std::thread*> m_threads;
  };
}


#endif //ZTHREAD_HPP
