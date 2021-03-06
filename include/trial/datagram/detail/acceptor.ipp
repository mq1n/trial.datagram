#ifndef TRIAL_DATAGRAM_ACCEPTOR_IPP
#define TRIAL_DATAGRAM_ACCEPTOR_IPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <utility>
#include <functional>

namespace trial
{
namespace datagram
{

inline acceptor::acceptor(boost::asio::io_service& io,
                          endpoint_type local_endpoint)
    : boost::asio::basic_io_object<detail::service<protocol>>(io),
      multiplexer(get_service().add(local_endpoint))
{
}

template <typename AcceptHandler>
void acceptor::async_accept(socket_type& socket,
                            AcceptHandler&& handler)
{
    assert(multiplexer);

    multiplexer->async_accept
        (socket,
         [this, &socket, handler]
         (const boost::system::error_code& error)
         {
             if (!error)
             {
                 socket.set_multiplexer(multiplexer);
                 multiplexer->add(&socket);
             }
             handler(error);
         });
}

inline acceptor::endpoint_type acceptor::local_endpoint() const
{
    assert(multiplexer);

    return multiplexer->next_layer().local_endpoint();
}

} // namespace datagram
} // namespace trial

#endif // TRIAL_DATAGRAM_ACCEPTOR_IPP
