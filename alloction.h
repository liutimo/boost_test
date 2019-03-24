//
// Created by liuzheng on 19-3-21.
//

#ifndef BOOST_ASIO_EXAMPLE_ALLOCTION_H
#define BOOST_ASIO_EXAMPLE_ALLOCTION_H

#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;


class handler_memory {
public:
    handler_memory() : m_in_use(false) {

    }

    handler_memory(const handler_memory &) = delete;
    handler_memory&operator=(const handler_memory&) = delete;

    void *allocate(std::size_t size) {
        if (!m_in_use && size < sizeof(m_storae)) {
            m_in_use = true;
            return &m_storae;
        } else {
            return ::operator new(size);
        }
    }

    void deallocate(void *pointer) {
        if (pointer == &m_storae) {
            m_in_use = false;
        } else {
            ::operator delete(pointer);
        }
    }

private:
    typename std::aligned_storage<1024>::type m_storae;
    bool m_in_use;
};

template <typename T>
class handler_allocator {
public:
    using value_type = T;

    explicit handler_allocator(handler_memory &mem) : m_memory(mem)
    {}

    template <typename U>
    handler_allocator(const handler_allocator<U>& other) noexcept
            : m_memory(other.m_memory)
    {}

    bool operator=(const handler_allocator& other) const noexcept {
        return &m_memory == &other.m_memory;
    }

    bool operator!=(const handler_allocator& other) const noexcept {
        return &m_memory != &other.m_memory;
    }

    T* allocate(std::size_t n) const {
        return static_cast<T*>(m_memory.allocate(sizeof(T) * n));
    }

    void deallocate(T *p, std::size_t /*n*/) const {
        return m_memory.deallocate(p);
    }

private:
    template <typename> friend class handler_allocator; //for  other.m_memory

    handler_memory &m_memory;
};


template <typename Handler>
class custom_alloc_handler {
public:
    using allocator_type = handler_allocator<Handler>;

    custom_alloc_handler(handler_memory &mem, Handler h)
            : m_memory(mem), m_hander(h) {

    }

    allocator_type get_allocator() const noexcept {
        return allocator_type(m_memory);
    }


    template <typename ...Args>
    void operator()(Args&&... args) {
        m_hander(std::forward<Args>(args)...);
    }
private:
    handler_memory &m_memory;
    Handler m_hander;
};

template <typename Handler>
inline custom_alloc_handler<Handler> make_custom_alloc_handler(
        handler_memory &m, Handler h)
{
    return custom_alloc_handler<Handler>(m, h);
}


class session : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket) : m_socket(std::move(socket)) {

    }

    void start() {
        do_read();
    }
private:
    void do_read() {
        auto self(shared_from_this());
        m_socket.async_read_some(boost::asio::buffer(m_data),
                                 make_custom_alloc_handler(m_memory,
                                                           [this, self](boost::system::error_code ec, std::size_t length)
                                                           {
                                                                std::cout << "recv: " << length << " bytes data!" << std::endl;
                                                               if (!ec) {
                                                                   do_write(length);
                                                               } else {
                                                                   std::cerr << ec.message() << std::endl;
                                                               }
                                                           }));
    }

    void do_write(std::size_t length)
    {
        auto self(shared_from_this());



        boost::asio::async_write(m_socket, boost::asio::buffer(buf, strlen(buf)),
                                 make_custom_alloc_handler(m_memory,
                                                           [this, self](boost::system::error_code ec, std::size_t length)
                                                           {
                                                                std::cout << "send: " << length << " bytes data!" << std::endl;
                                                               if(!ec) {
                                                                   do_read();
                                                               }
                                                           }));
    }
    tcp::socket m_socket;
    std::array<char, 1024> m_data;
    handler_memory m_memory;
};


class server{
public:
    explicit server(boost::asio::io_context &io_context, short port)
            : m_acceptor(io_context, tcp::endpoint(tcp::v4(), port))
    {
        do_acceptor();
    }
private:
    void do_acceptor() {
        m_acceptor.async_accept([this](auto ec, auto socket){
            if (!ec) {
                std::make_shared<session>(std::move(socket))->start();
            }
        });
    }

    tcp::acceptor m_acceptor;
};
#endif //BOOST_ASIO_EXAMPLE_ALLOCTION_H
