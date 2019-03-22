#ifndef ASIO_EXAMPLES_BUFFER_H
#define ASIO_EXAMPLES_BUFFER_H

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <ctime>

using boost::asio::ip::tcp;

class shared_const_buffer {
public:
    explicit shared_const_buffer(const std::string &data)
            :   m_data(new std::vector<char>(data.begin(), data.end())),
                m_buffer(boost::asio::buffer(*m_data)), i(0)
    {
        ++i;
        std::cout << "shared_const_buffer() i = " << i << std::endl;
    }

    ~shared_const_buffer()
    {
        --i;
        std::cout << "~shared_const_buffer() i = " << i << std::endl;
    }

    shared_const_buffer(const shared_const_buffer &scb) {
        std::cout << "shared_const_buffer(const shared_const_buffer &scb)" << std::endl;
        m_data = scb.m_data;
        m_buffer = scb.m_buffer;
        i = scb.i;
    }

    typedef boost::asio::const_buffer value_type;
    typedef const boost::asio::const_buffer* const_iterator;

    const_iterator begin() const {return &m_buffer; }
    const_iterator end()   const {return &m_buffer + 1;}

private:
    std::shared_ptr<std::vector<char>> m_data;
    value_type m_buffer;
    int i ;
};

class session : public std::enable_shared_from_this<session> {
public:
    session(tcp::socket socket_) : m_socket(std::move(socket_)) {
        std::cout << "session()" << std::endl;
    }

    ~session() {
        std::cout << "~session()" << std::endl;
    }

    void start() {
        do_write();
    }

private:
    void do_write() {
        std::time_t  now = std::time(nullptr);
        shared_const_buffer buffer(std::ctime(&now));
        auto self(shared_from_this());
        //buffer 经历多次拷贝构造
        boost::asio::async_write(m_socket, buffer, [this, self](auto ec, auto len){
            if (!ec) {
                std::cout << "send " << len << " bytes data" << std::endl;
            } else {
                std::cout << "error: " << ec.message() << std::endl;
            }
        });

    }


    tcp::socket m_socket;
};

class server {
public:
    server(boost::asio::io_context &io_context, short port)
        : m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }

private:
    void do_accept() {
        m_acceptor.async_accept([this](auto ec, auto socket_){
            if (!ec) {
                std::make_shared<session>(std::move(socket_))->start();
            }
            do_accept();
        });
    }

    tcp::acceptor m_acceptor;
};

#endif //ASIO_EXAMPLES_BUFFER_H
