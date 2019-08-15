#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <thread>
#include <vector>
#include <iostream>

// #include <arpa/inet.h>
#include <netdb.h>

#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable


/*! @file socket.hpp
*  @brief Manage network sockets
*/

namespace ztd
{
  //! @brief Main socket abstract object
  /*! Abstract object, doesn't work on its own. Provides operations and templates for child classes. \n \n
      Provides automatic background data reading with parallel_on() \n
      Wait for reading of data with wait_data() \n
      Retrieve data with retrieve() \n
  */
  class socket_abstract
  {
  public:
    socket_abstract();
    virtual ~socket_abstract();

    //! @brief Get the socket's file descriptor
    inline int fd() { return m_fd; }
    //! @brief Valid connection established, data can be sent and/or recieved
    inline bool operational() { return m_operational; }
    //! @brief Close connection
    void close_socket();

    //! @brief Send string data
    bool send_string(std::string const& text);
    //! @brief Recieve string data
    std::string read_string();
    //! @brief Send byte data
    /*! @param data Array of bytes
        @param size Size of the array
        @return @a True if successful, @a False otherwise
    */
    bool send_data(uint8_t *data, uint32_t size);
    //! @brief Send byte data
    /*! @param data Pre-allocated array of bytes
        @param size Pointer to size of data array. Number of bytes read stored to pointed location if successful reading
        @return @a True if successful, @a False otherwise
    */
    bool read_data(uint8_t *data, uint32_t *size);

    //! @brief State of automatic parallel data reading
    inline bool parallel() { return m_parallel; }
    //! @brief Turn on automatic parallel data reading
    /*! Socket has to be operational. \n Read data stored for later use with retrieve()
        @param bufferSize size of the reading buffer
    */
    void parallel_on(uint32_t bufferSize=256);
    //! @brief Turn off automatic parallel data reading
    void parallelOff();
    //! @brief Forcibly kill automatic parallel data reading
    void killParallel();

    inline std::vector<std::vector<uint8_t>>& data() { return m_dataSet; }
    //! @brief Data is available
    /*! Parallel reading enabled
    */
    inline bool data_available() { return m_dataSet.size() > 0; }
    //! @brief Retrieve pending data (parallel reading)
    std::vector<uint8_t> retrieve();

    inline std::condition_variable& cv() { return m_cv; }
    inline std::mutex& mtx() { return m_mtx; }
    //! @brief Wait until data is recieved (parallel reading)
    void wait_data();

  private:
    static void parallel_reading(socket_abstract *s, uint32_t bufferSize);

  protected:
    int m_fd;
    bool m_operational;

    bool m_parallel;
    std::thread *m_thread;
    std::vector<std::vector<uint8_t>> m_dataSet;
    std::condition_variable m_cv;
    std::mutex m_mtx;
  };

  //! @brief TCP socket abstract object
  /*! Abstract object, doesn't work on its own. Provides operations and templates for child classes
  */
  class tcpsocket_abstract : public socket_abstract
  {
  public:
    tcpsocket_abstract();
    virtual ~tcpsocket_abstract();

    //! @brief Init socket for IPv4 TCP
    bool init_ipv4();
    //! @brief Init socket for IPv6 TCP
    bool init_ipv6();

  };

  //! @brief TCP server
  /*! Does not provide an operational socket, use tcpsocket_server_instance for a connection to a client \n \ny
      Run init_ipv4() then listen_ipv4() to open the server on ipv4 \n
      Run init_ipv6() then listen_ipv6() to open the server on IPv6
  */
  class tcpsocket_server : public tcpsocket_abstract
  {
  public:
    tcpsocket_server();
    virtual ~tcpsocket_server();

    //! @brief Server is open
    inline bool is_open() { return m_listening; }
    //! @brief Listening port. 0 if not open
    inline uint16_t port() { return m_port; }

    inline struct sockaddr_in addr() { return m_servaddr; }

    //! @brief Open server for IPv4
    /*! @param port Listening port, != 0
        @param backlog Backlog value
        @return @a True if successful, @a False otherwise
    */
    bool listen_ipv4(uint16_t port, int backlog=5);

    //! @brief Open server for IPv6
    /*! @param port Listening port, != 0
        @param backlog Backlog value
        @return @a True if successful, @a False otherwise
    */
    bool listen_ipv6(uint16_t port, int backlog=5);


  private:
    bool m_listening;
    uint16_t m_port;
    struct sockaddr_in m_servaddr;
  };

  //! @brief Instance for TCP server
  /*! Instance of a TCP connection to a client. Can send/read data. Refer to socket_abstract methods
  */
  class tcpsocket_server_instance : public socket_abstract
  {
  public:
    //! @brief Constructor.
    /*! @param server Pointer to parent TCP server
    */
    tcpsocket_server_instance(tcpsocket_server *server);
    virtual ~tcpsocket_server_instance();

    //! @brief Wait for a client connection.
    /*! Socket becomes operational if connection successful
        @return @a True if a valid connection is established, @a False otherwise
    */
    bool accept_connection();

    inline struct sockaddr_in client() { return m_cliaddr; }
    //! @brief Get client address
    char* client_address();
    //! @brief Get client port
    uint16_t client_port();

  private:
    tcpsocket_server *m_server;
    struct sockaddr_in m_cliaddr;

    socklen_t m_clilen;

  };

  //! @brief TCP client
  /*! Can send/read data. Refer to socket_abstract methods \n
    Run init_ipv4() then connect_ipv4() for an operational IPv4 connection \n
    Run init_ipv6() then connect_ipv6() for an operational IPv6 connection
  */
  class tcpsocket_client : public tcpsocket_abstract
  {
  public:
    tcpsocket_client();
    virtual ~tcpsocket_client();

    //! @brief Connect to an IPv4 TCP server
    /*! @param addr IPv4 string address of the target server
        @param port Port of the target server
        @return @a True if successful, @a False otherwise
    */
    bool connect_ipv4(std::string const& addr, uint16_t port);
    //! @brief Connect to an IPv6 TCP server
    /*! @param addr IPv6 string address of the target server
    @param port Port of the target server
    @return @a True if successful, @a False otherwise
    */
    bool connect_ipv6(std::string const& addr, uint16_t port);

    //! @brief String address of target server
    inline std::string addr() { return m_addr; }
    //! @brief Port of target server
    inline uint16_t port() { return m_port; }


    private:
      struct hostent *m_server;
      struct sockaddr_in m_servaddr;
      std::string m_addr;
      uint16_t m_port;
    };

}


#endif //SOCKET_H
