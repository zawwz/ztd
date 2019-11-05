#ifndef SHELL_HPP
#define SHELL_HPP

#include <string>
#include <utility>
#include <stdio.h>

/*! \file shell.hpp
*  @brief Shell functionality and interaction
*/

namespace ztd
{
  //! @brief Execute a shell command and retrieve its output
  /*!
  @param command Shell command to execute
  @param to_console Output to console
  @return Output of the command
  */
  std::string sh(const std::string& command, bool to_console=false);

  //! @brief Execute a shell command and retrieve its output and return value
  /*!
  @param command Shell command to execute
  @param to_console Output to console
  @return @b first Output of command\n@b second Return value of command
  */
  std::pair<std::string, int> shp(const std::string& command, bool to_console=false);

  //! @brief popen C function with added pid functionality
  /*!
  @param pid Pointer to an @a int in which the process's pid will be stored
  @see <a href="http://man7.org/linux/man-pages/man3/popen.3.html">popen(), pclose()</a>
  */
  FILE* popen2(const char* command, const char* type, int* pid);
  //! @brief pclose C function with added pid functionality
  /*!
  @param pid Pid of the opened process
  @see <a href="http://man7.org/linux/man-pages/man3/popen.3.html">popen(), pclose()</a>
  */
  int pclose2(FILE* fp, pid_t pid);

  // class shell
  // {
  // public:
  // private:
  // };
}

#endif //SHELL_HPP
