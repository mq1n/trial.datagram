#ifndef TRIAL_DATAGRAM_DETAIL_MULTIPLEXER_HPP
#define TRIAL_DATAGRAM_DETAIL_MULTIPLEXER_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <atomic>
#include <memory>
#include <functional>
#include <deque>
#include <tuple>
#include <map>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <trial/datagram/detail/buffer.hpp>

namespace trial
{
namespace datagram
{
namespace detail
{

class socket_base;

// The multiplexer mediates between trial.datagram sockets and the underlying
// UDP socket.

class multiplexer
    : public std::enable_shared_from_this<multiplexer>
{
public:
    // Must be the underlying UDP types
    using protocol_type = boost::asio::ip::udp;
    using next_layer_type = protocol_type::socket;
    using endpoint_type = protocol_type::endpoint;
    using buffer_type = detail::buffer;

    // Use factory method to ensure that multiplexer is created as shared_ptr
    template <typename... Types>
    static std::unique_ptr<multiplexer> create(Types&&...);

    ~multiplexer();

    void add(socket_base *);
    void remove(socket_base *);

    template <typename SocketType,
              typename AcceptHandler>
    void async_accept(SocketType&,
                      AcceptHandler&& handler);

    template <typename ConstBufferSequence,
              typename CompletionToken>
    typename boost::asio::async_result<
        typename boost::asio::handler_type<CompletionToken,
                                           void(boost::system::error_code, std::size_t)>::type
        >::type
    async_send_to(const ConstBufferSequence& buffers,
                       const endpoint_type& endpoint,
                       CompletionToken&& token);

    void start_receive();

    const next_layer_type& next_layer() const;
    next_layer_type& next_layer();

private:
    multiplexer(boost::asio::io_service& io,
                endpoint_type local_endpoint);

    void do_start_receive();

    void process_receive(const boost::system::error_code&,
                         std::unique_ptr<buffer_type>,
                         const endpoint_type&);

private:
    next_layer_type real_socket;

    using socket_map = std::map<endpoint_type, socket_base *>;
    socket_map sockets;

    std::atomic<int> pending_receive_count;

    using accept_handler_type = std::function<void (const boost::system::error_code&)>;
    using accept_input_type = std::tuple<socket_base *, accept_handler_type>;
    std::deque<std::unique_ptr<accept_input_type>> acceptor_queue;

    // FIXME: Bounded queue? (like listen() backlog)
    using accept_output_type = std::tuple<boost::system::error_code, std::unique_ptr<buffer_type>, endpoint_type>;
    std::deque<std::unique_ptr<accept_output_type>> listen_queue;
};

} // namespace detail
} // namespace datagram
} // namespace trial

#include <trial/datagram/detail/multiplexer.ipp>

#endif // TRIAL_DATAGRAM_DETAIL_MULTIPLEXER_HPP
