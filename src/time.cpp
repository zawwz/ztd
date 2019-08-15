#include "time.hpp"

#include <stdlib.h>

ztd::timer::timer()
{
  m_sec=0;
  m_stopped=true;
}

double ztd::timer::time() const
{
  if(m_stopped)
    return m_sec;
  struct timeval t_time;
  gettimeofday(&t_time, NULL);
  return t_time.tv_sec - m_start.tv_sec + (t_time.tv_usec - m_start.tv_usec) / 1000000.0;
}

void ztd::timer::start()
{
  m_sec=0;
  gettimeofday(&m_start, NULL);
  m_stopped=false;
}

bool ztd::timer::stop()
{
  if(m_stopped)
    return false;
  gettimeofday(&m_end, NULL);
  m_stopped=true;
  m_sec = m_end.tv_sec - m_start.tv_sec + (m_end.tv_usec - m_start.tv_usec) / 1000000.0;
  return true;
}

void ztd::timer::reset()
{
  m_sec=0;
  m_stopped=true;
}
