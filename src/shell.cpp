#include "shell.hpp"

#include <thread>
#include <fstream>

#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

//// SHELL CALLS ////

std::string ztd::sh(const std::string& command)
{
  return ztd::shp(command).first;
}

int ztd::shr(const std::string& command)
{
  return WEXITSTATUS(system(command.c_str()));
}

std::pair<std::string, int> ztd::shp(const std::string& command)
{
  std::string ret;
  FILE *stream = popen(command.c_str(), "r");
  char* buff = NULL;
  size_t buff_size = 0;
  while (getline(&buff, &buff_size, stream) > 0)
  {
    ret += buff;
    free(buff);
  }
  return std::make_pair(ret, WEXITSTATUS(pclose(stream)));
}

// SHC

ztd::shc::shc(std::string const& cmd)
{
  this->command=cmd;
  this->running=false;
  this->pid=0;
}
ztd::shc::~shc()
{
  if(this->running)
    this->kill_int();
  this->thread.join();
}

void ztd::shc::run()
{
  ztd::wait_pool wait_for_start;
  if(!this->running)
    this->thread = std::thread(ztd::shc::run_process, this, &wait_for_start);
  if(!this->running)
    wait_for_start.wait();
}

int ztd::shc::kill_int()
{
  if(running)
    return kill(this->pid, SIGINT);
  else
    return 1;
}

void ztd::shc::wait_output()
{
  while(this->running && !(this->has_output()) )
    this->wp_output.wait();
}

std::string ztd::shc::get_output()
{
  if(output.size() > 0)
  {
    std::string ret = this->output.front();
    this->output.pop();
    return ret;
  }
  else
    return "";
}

void ztd::shc::wait_finish()
{
  while(this->running)
    this->wp_finish.wait();
}

void ztd::shc::run_process(shc* p, ztd::wait_pool* wp)
{
  if(p->running)
    return;

  p->running = true;
  wp->notify_all();

  char* buff = NULL;
  size_t buff_size = 0;
  int pid = 0;
  FILE *stream = ztd::popen2(p->command.c_str(), "r", &pid);
  p->pid = pid;

  std::string ln;
  while ( getline(&buff, &buff_size, stream) > 0 ) //retrieve lines
  {
    p->output.push(std::string(buff));
    p->wp_output.notify_all();
    free(buff);
  }

  p->return_value = WEXITSTATUS(ztd::pclose2(stream, pid));
  p->running = false;
  p->wp_output.notify_all();
  p->wp_finish.notify_all();
}

//// EXEC EXTENTIONS ////

// open/close calls

FILE* ztd::eopen(const char* type, int* pid, const char* bin, std::vector<char*> args)
{
  const int READ=0, WRITE=1;
  pid_t child_pid;
  int fd[2];
  if(pipe(fd) != 0)
    perror("pipe");

  args.push_back(NULL);                   // NULL terminated array for execv()
  args.insert(args.begin(), (char*) bin); // first arg is name of the exec

  // forking
  if((child_pid = fork()) == -1)
  {
    perror("fork");
    exit(1);
  }
  if (child_pid == 0) // child process
  {
    if ( index(type, 'r') != NULL )
    {
      close(fd[READ]);    //Close the READ end of the pipe since the child's fd is write-only
      dup2(fd[WRITE], 1); //Redirect stdout to pipe
    }
    else
    {
      close(fd[WRITE]);    //Close the WRITE end of the pipe since the child's fd is read-only
      dup2(fd[READ], 0);   //Redirect stdin to pipe
    }
    setpgid(child_pid, child_pid); //Needed so negative PIDs can kill children of /bin/sh
    execvp(bin, args.data());
    exit(1);
  }
  else // main process
  {
    if ( index(type, 'r') != NULL )
      close(fd[WRITE]); //Close the WRITE end of the pipe since parent's fd is read-only
    else
      close(fd[READ]); //Close the READ end of the pipe since parent's fd is write-only
  }
  if(pid != NULL)
    *pid = child_pid;

  if ( index(type, 'r') != NULL )
  {
    return fdopen(fd[READ], "r");
  }

  return fdopen(fd[WRITE], "w");
}

int ztd::eclose(FILE* fd, pid_t pid)
{
  int stat;

  fclose(fd);
  while (waitpid(pid, &stat, 0) == -1)
  {
    if (errno != EINTR)
    {
      stat = -1;
      break;
    }
  }

  return stat;
}

// STANDALONE EXEC

// exec calls
// function itself
std::pair<std::string, int> ztd::exec(std::string const& bin, std::vector<std::string> const& args)
{
  std::string ret;
  pid_t pid;
  std::vector<char*> rargs;
  for(auto it=args.begin(); it!=args.end() ; it++)
    rargs.push_back((char*) it->c_str());

  FILE *stream = eopen("r", &pid, bin.c_str(), rargs);
  char* buff = NULL;
  size_t buff_size = 0;
  while (getline(&buff, &buff_size, stream) > 0)
  {
    ret += buff;
    free(buff);
  }
  return std::make_pair(ret, WEXITSTATUS(eclose(stream, pid)));
}
std::pair<std::string, int> ztd::script(std::string const& data, std::vector<std::string> const& args)
{
  // generate path
  const std::string tmpdir = (getenv("TMPDIR") != NULL) ? getenv("TMPDIR") : "/tmp" ;
  const std::string filepath = tmpdir + "/ztdscript_" + ztd::sh("tr -dc '[:alnum:]' < /dev/urandom | head -c10");
  // create stream
  std::ofstream stream(filepath);
  if(!stream)
    throw std::runtime_error("Failed to write to file '"+filepath+'\'');

  // output
  stream << data;
  stream.close();
  ztd::sh("chmod +x "+filepath);

  // execute script
  auto ret = ztd::exec(filepath, args);

  // delete file
  if( remove(filepath.c_str()) != 0 )
    throw std::runtime_error("Failed to delete file '"+filepath+'\'');
  return std::make_pair(ret.first, ret.second);
}
