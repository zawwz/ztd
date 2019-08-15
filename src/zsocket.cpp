#include "zsocket.hpp"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <unistd.h> // read/write/close
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

ztd::socket_abstract::socket_abstract()
{
  m_fd=-1;
  m_operational=false;
  m_thread=nullptr;
  m_parallel=false;
}

ztd::socket_abstract::~socket_abstract()
{
  killParallel();
  close_socket();
}

void ztd::socket_abstract::close_socket()
{
  m_operational=false;
  close(m_fd);
  m_fd=-1;
  parallelOff();
}

bool ztd::socket_abstract::send_string(std::string const& text)
{
  if(m_fd < 0 || !m_operational)
  return false;
  if(send(m_fd, text.c_str(), text.size()+1, 0) < 0 )
  {
    close_socket();
    return false;
  }
  return true;
}

std::string ztd::socket_abstract::read_string()
{
  if(m_fd < 0 || !m_operational)
  return "";
  char buf[256]={0};
  if(read(m_fd, buf, 255) <= 0)
  {
    close_socket();
    return "";
  }
  m_cv.notify_all();
  return buf;
}

bool ztd::socket_abstract::send_data(uint8_t *data, uint32_t size)
{
  if(m_fd < 0 || !m_operational)
  return false;
  if(send(m_fd, data, size, 0) < 0 )
  {
    close_socket();
    return false;
  }
  return true;
}

bool ztd::socket_abstract::read_data(uint8_t *data, uint32_t *size)
{
  if(m_fd < 0 || !m_operational)
    return false;
  uint32_t n = read(m_fd, data, (*size));
  if(n <= 0)
  {
    close_socket();
    return false;
  }
  *size = n;
  return true;
}

std::vector<uint8_t> ztd::socket_abstract::retrieve()
{
  if(m_dataSet.size() > 0)
  {
    std::vector<uint8_t> ret = *m_dataSet.begin();
    m_dataSet.erase(m_dataSet.begin());
    return ret;
  }
  return std::vector<uint8_t>();
}

void ztd::socket_abstract::parallel_on(uint32_t bufferSize)
{
  if(!m_parallel)
  {
    m_parallel=true;
    m_thread= new std::thread(parallel_reading, this, bufferSize);
    m_thread->detach();
  }
}

void ztd::socket_abstract::parallelOff()
{
  m_parallel=false;
  m_thread=nullptr;
  m_cv.notify_all();
}

void ztd::socket_abstract::killParallel()
{
  if(m_thread!=nullptr)
  {
    m_parallel=false;
    delete m_thread;
    m_thread=nullptr;
  }
}

void ztd::socket_abstract::wait_data()
{
  if(!m_parallel || !m_operational)
    return;
  std::unique_lock<std::mutex> lck(m_mtx);
  while(!data_available() && m_parallel && m_operational) m_cv.wait(lck);
}

void ztd::socket_abstract::parallel_reading(ztd::socket_abstract *s, uint32_t bufferSize)
{
  uint8_t *buffer = (uint8_t*) malloc(bufferSize);
  uint32_t tmp_size;
  while(s->parallel() && s->operational())
  {
    tmp_size=bufferSize;
    if(s->read_data(buffer, &tmp_size))
    {
      s->data().push_back(std::vector<uint8_t>(buffer, buffer+tmp_size));
    }
    s->cv().notify_all();
  }
  free(buffer);
}

ztd::tcpsocket_abstract::tcpsocket_abstract()
{
}

ztd::tcpsocket_abstract::~tcpsocket_abstract()
{
}

bool ztd::tcpsocket_abstract::init_ipv4()
{
  m_fd = socket(AF_INET, SOCK_STREAM, 0);
  return m_fd >=0;
}
bool ztd::tcpsocket_abstract::init_ipv6()
{
  m_fd = socket(AF_INET6, SOCK_STREAM, 0);
  return m_fd >=0;
}

ztd::tcpsocket_server::tcpsocket_server()
{
  m_port=0;
  bzero(&m_servaddr, sizeof(m_servaddr));
  m_listening=false;
}
ztd::tcpsocket_server::~tcpsocket_server()
{
}

bool ztd::tcpsocket_server::listen_ipv4(uint16_t port, int backlog)
{
  m_port=port;
  if(m_port == 0)
    return false;
  m_servaddr.sin_family = AF_INET;
  m_servaddr.sin_addr.s_addr = INADDR_ANY;
  m_servaddr.sin_port = htons(m_port);
  if(bind(m_fd, (struct sockaddr *) &m_servaddr, sizeof(m_servaddr)) < 0) //binding
    return false;
  listen(m_fd, 5);
  if(m_fd < 0)
    return false;
  m_listening=true;
    return true;
}

bool ztd::tcpsocket_server::listen_ipv6(uint16_t port, int backlog)
{
  m_port=port;
  if(m_port == 0)
    return false;
  m_servaddr.sin_family = AF_INET6;
  m_servaddr.sin_addr.s_addr = INADDR_ANY;
  m_servaddr.sin_port = htons(m_port);
  if(bind(m_fd, (struct sockaddr *) &m_servaddr, sizeof(m_servaddr)) < 0) //binding
    return false;
  listen(m_fd, backlog);
  if(m_fd < 0)
    return false;
  m_listening=true;
  return true;
}

ztd::tcpsocket_server_instance::tcpsocket_server_instance(ztd::tcpsocket_server *server)
{
  m_server=server;
  m_clilen=sizeof(m_cliaddr);
}
ztd::tcpsocket_server_instance::~tcpsocket_server_instance()
{
}

bool ztd::tcpsocket_server_instance::accept_connection()
{
  if(m_server == nullptr)
    return false;
  if(!m_server->is_open())
    return false;
  m_fd = accept(m_server->fd(), (struct sockaddr *) &m_cliaddr, &m_clilen);
  if(m_fd < 0)
    return false;

  m_operational=true;
  return true;
}

char* ztd::tcpsocket_server_instance::client_address()
{
  return inet_ntoa(m_cliaddr.sin_addr);
}
uint16_t ztd::tcpsocket_server_instance::client_port()
{
  return ntohs(m_cliaddr.sin_port);
}

ztd::tcpsocket_client::tcpsocket_client()
{
  bzero( (char *) &m_servaddr, sizeof(m_servaddr));
  m_server=nullptr;
}
ztd::tcpsocket_client::~tcpsocket_client()
{
}

bool ztd::tcpsocket_client::connect_ipv4(std::string const& addr, uint16_t port)
{
  m_addr=addr;
  m_port = port;
  m_server = gethostbyname(m_addr.c_str());
  m_servaddr.sin_family = AF_INET;
  bcopy((char *)m_server->h_addr, (char *)&m_servaddr.sin_addr.s_addr, m_server->h_length);
  m_servaddr.sin_port = htons(m_port);
  if(connect(m_fd, (struct sockaddr *) &m_servaddr, sizeof(m_servaddr)) < 0)
    return false;
  m_operational = true;
  return true;
}

bool ztd::tcpsocket_client::connect_ipv6(std::string const& addr, uint16_t port)
{
  m_addr=addr;
  m_port = port;
  m_server = gethostbyname(m_addr.c_str());
  m_servaddr.sin_family = AF_INET6;
  bcopy((char *)m_server->h_addr, (char *)&m_servaddr.sin_addr.s_addr, m_server->h_length);
  m_servaddr.sin_port = htons(m_port);
  if(connect(m_fd, (struct sockaddr *) &m_servaddr, sizeof(m_servaddr)) < 0)
    return false;
  m_operational = true;
  return true;
}
