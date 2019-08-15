#ifndef ZTIME_HPP
#define ZTIME_HPP

#include <sys/time.h>

/*! @file ztime.hpp
*  @brief Time measuring and managing
*/

namespace ztd
{
  //! Measure time intervals
  /*! Class for quick and easy time measurement
  */
  class timer
  {
  public:
    timer();

    //! Get measured time
    /*!
      If the timer was stopped, returns the time between start and stop\n
      If the timer is still running, returns time between start and now
      @return
        In seconds, µs granularity
    */
    double time() const;

    //! Start the timer
    /*!
      If the timer was already running, resets the timer to this point
    */
    void start();
    //! Stop the timer
    /*!
    Store time difference
    @return
      @b true if timer was running, @b false otherwise
    */
    bool stop();

    //! Reset the timer
    void reset();

  private:
    bool m_stopped;
    double m_sec;
    struct timeval m_start, m_end;
  };
}


#endif //ZTIME_HPP
