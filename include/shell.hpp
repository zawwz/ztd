#ifndef SHELL_HPP
#define SHELL_HPP

#include <string>
#include <utility>
#include <queue>
#include <stdio.h>

#include <thread>


#include "wait.hpp"

/*! \file shell.hpp
*  @brief Shell functionality and interaction
*/

namespace ztd
{

  /// SHELL CALLS ///

  //! @brief Execute a shell command and retrieve its output
  /*!
  @param command Shell command to execute
  Doesn't output to console
  @return Output of command
  */
  std::string sh(const std::string& command);

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
  Doesn't output to console
  @return @b first Output of command\n@b second Return value of command
  */
  std::pair<std::string, int> shp(const std::string& command);

  //! @brief Shell call class
  class shc
  {
  public:
    //! @brief constructor
    shc(std::string const& cmd="");
    virtual ~shc();

    //! @brief Start the command
    void run();
    //! @brief Kill the command (SIGINT)
    int kill_int();

    //! @brief Wait until the command gives output
    void wait_output();
    inline bool has_output() { return this->output.size() > 0; }
    //! @brief Retrieve a line from the command's output
    std::string get_output();

    //! @brief Wait for the command to finish
    void wait_finish();

    //! @brief Command to execute
    std::string command;

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

    std::thread thread;

  private:
    static void run_process(shc* p, ztd::wait_pool* wp);
  };


  //// POPEN WITH PID ////

  //// EXEC EXTENSIONS ////

  /// open/close extensions

  //! @brief Similar to popen2() but calls an executable instead of a shell
  /*!
  @param type Mode of the execution (r/w)
  @param pid Pointer to an @a int in which the process's pid will be stored
  @param bin Binary to execute. Has PATH resolution
  @param args
  @return File descriptor for the stream in question
  */
  FILE* eopen(const char* type, int* pid, const char* bin, std::vector<char*> args);
  //! @brief Similar to pclose2() but for eopen()
  /*!
  @param fd
  @param pid Pid of the opened process
  @return Return value of the command
  */
  int eclose(FILE* fd, pid_t pid);

  //! @brief popen C function with added pid functionality
  /*!
  @param command Shell command to execute
  @param type Mode of the execution (r/w)
  @param pid Pointer to an @a int in which the process's pid will be stored
  @return File descriptor for the stream in question
  @see <a href="http://man7.org/linux/man-pages/man3/popen.3.html">popen(), pclose()</a>
  */
  inline FILE* popen2(const char* command, const char* type, int* pid) { std::vector<char*> tvec = {"-c",(char*) command}; return eopen(type, pid, "/bin/sh", tvec) ; }
  //! @brief pclose C function with added pid functionality
  /*!
  @param fd
  @param pid Pid of the opened process
  @return Return value of the command
  @see <a href="http://man7.org/linux/man-pages/man3/popen.3.html">popen(), pclose()</a>
  */
  inline int pclose2(FILE* fd, pid_t pid) { return eclose(fd, pid); }


  /// exec extensions

  //! @brief Execute a binary and retrieve its outputs
  /*!
  @param bin Binary file to execute. Has PATH resolution
  @param args Arguments given to the binary
  */
  std::pair<std::string, int> exec(std::string const& bin, std::vector<char*> const& args);
  //! @brief @see exec(std::string const& bin, std::vector<char*> const& args)
  std::pair<std::string, int> exec(std::string const& bin, std::vector<std::string> const& args);

}

#endif //SHELL_HPP
