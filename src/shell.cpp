#include "shell.hpp"

#include <thread>

#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

std::string ztd::sh(const std::string& command, bool to_console)
{
  return ztd::shp(command, to_console).first;
}

int ztd::shr(const std::string& command)
{
  return WEXITSTATUS(system(command.c_str()));
}

std::pair<std::string, int> ztd::shp(const std::string& command, bool to_console)
{
  std::string ret;
  FILE *stream = popen(command.c_str(), "r");
  char* buff = NULL;
  size_t buff_size = 0;
  while (getline(&buff, &buff_size, stream) > 0)
  {
    if(to_console)
      printf("%s", buff);

    ret += buff;
  }
  return std::make_pair(ret, WEXITSTATUS(pclose(stream)));
}

FILE* ztd::popen2(const char* command, const char* type, int* pid)
{
    const int READ=0, WRITE=1;
    pid_t child_pid;
    int fd[2];
    pipe(fd);

    if((child_pid = fork()) == -1)
    {
        perror("fork");
        exit(1);
    }

    /* child process */
    if (child_pid == 0)
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
        execl("/bin/sh", "/bin/sh", "-c", command, NULL);
        exit(0);
    }
    else
    {
        if ( index(type, 'r') != NULL )
        {
            close(fd[WRITE]); //Close the WRITE end of the pipe since parent's fd is read-only
        }
        else
        {
            close(fd[READ]); //Close the READ end of the pipe since parent's fd is write-only
        }
    }
    if(pid != NULL)
      *pid = child_pid;

    if ( index(type, 'r') != NULL )
    {
        return fdopen(fd[READ], "r");
    }

    return fdopen(fd[WRITE], "w");
}

int ztd::pclose2(FILE* fp, pid_t pid)
{
    int stat;

    fclose(fp);
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

// SHC

ztd::shc::shc(std::string const& cmd, bool const cout)
{
  this->command=cmd;
  this->to_console=cout;
}
ztd::shc::~shc()
{
  if(this->running)
    this->kill_int();
}

void ztd::shc::run()
{
  std::thread(ztd::shc::run_process, this).detach();
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
  while(this->output.size() <= 0)
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

void ztd::shc::run_process(shc* p)
{
  if(p->running)
    return;

  char* buff = NULL;
  size_t buff_size = 0;
  int pid = 0;

  FILE *stream = ztd::popen2(p->command.c_str(), "r", &pid);
  p->pid = pid;
  p->running = true;

  std::string ln;
  while ( getline(&buff, &buff_size, stream) > 0 ) //retrieve device lines
  {
    if(p->to_console)
      printf("%s", buff);

    ln = std::string(buff, buff_size);
    p->output.push(ln);
    p->wp_output.notify_all();
  }

  p->running = false;
  p->wp_finish.notify_all();
  p->return_value = WEXITSTATUS(ztd::pclose2(stream, pid));
}
