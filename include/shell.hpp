#ifndef SHELL_HPP
#define SHELL_HPP

#include <string>
#include <utility>
#include <queue>
#include <stdio.h>


#include "wait.hpp"

/*! \file shell.hpp
*  @brief Shell functionality and interaction
*/

namespace ztd
{
  //! @brief Execute a shell command and retrieve its output
  /*!
  @param command Shell command to execute
  @param to_console Output to console
  @return Output of command
  */
  std::string sh(const std::string& command, bool to_console=false);

  //! @brief Execute a shell command and retrieve its return value
  /*!
  @param command Shell command to execute
  Always outputs to console
  @return Return value of command
  */
  int shr(const std::string& command);

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


  //! @brief Shell call class
  class shc
  {
  public:
    //! @brief constructor
    shc(std::string const& cmd="", bool const cout=false);
    virtual ~shc();

    //! @brief Start the command
    void run();
    //! @brief Kill the command (SIGINT)
    int kill_int();

    //! @brief Wait until the command gives output
    void wait_output();
    //! @brief Retrieve a line from the command's output
    std::string get_output();

    //! @brief Wait for the command to finish
    void wait_finish();

    //! @brief Command to execute
    std::string command;
    //! @brief Output the command result to console
    bool to_console;

    //! @brief Run status of the command
    bool running;
    //! @brief PID of the command (only during execution)
    pid_t pid;

    //! @brief Output lines of the command
    std::queue<std::string> output;
    //! @brief Return value on the command (only after execution)
    int return_value;

    ztd::wait_pool wp_output;
    ztd::wait_pool wp_finish;

  private:
    static void run_process(shc* p);
  };

}

#endif //SHELL_HPP
