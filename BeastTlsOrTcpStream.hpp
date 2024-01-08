/**
* @file BeastTlsOrTcpStream.hpp
* SPDX-License-Identifier: BSL-1.0 OR MIT-0 OR CC0-1.0
*/

#pragma once
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

/**
* @brief Wrapper of  boost::beast::ssl_stream<boost::beast::tcp_stream>
*
* Class BeastTlsStream can be used both as TLS/SSL stream or as plain TCP stream.
*
" BeastTlsStream provides methods with signatures allowing it to be passed to
* boost::beast::http::async_xxxx functions. And also be wrapped to WebSocket
* stream, boost::beast::websocket::stream<BeastTlsOrTcpStream> for async
* operations.
*
* BeastTlsStream start in TCP mode. In TCP mode it bypasses the SSL layer,
* async_write_some() / async_read_some() go to the TCP layer.
* Call setIsTls() at the start of the session to set to TLS mode, and
* async_write_some() / async_read_some() will go through the SSL layer.
*/

using BeastTlsStream = boost::beast::ssl_stream<boost::beast::tcp_stream>;
class BeastTlsOrTcpStream : public BeastTlsStream
{
public:
	// used as client
	BeastTlsOrTcpStream(boost::asio::io_context &ioc, boost::asio::ssl::context &sslCtx)
		: BeastTlsStream(ioc, sslCtx)
	{
	}
	// used as server
	BeastTlsOrTcpStream(boost::asio::ip::tcp::socket tcpSocket, boost::asio::ssl::context &sslCtx)
		: BeastTlsStream(std::move(tcpSocket), sslCtx)
	{
	}

	/**
	* Override async_write_some() to satisfy AsyncWriteStream concept for both TLS and TCP
	* https://www.boost.org/doc/libs/1_74_0/doc/html/boost_asio/reference/AsyncWriteStream.html
	* Adapted from boost/beast/ssl/ssl_stream.hpp
	*/
	template<class ConstBufferSequence, BOOST_BEAST_ASYNC_TPARAM2 WriteHandler>
	BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler, void(boost::system::error_code, std::size_t))
	async_write_some(ConstBufferSequence const &buffers, BOOST_ASIO_MOVE_ARG(WriteHandler) handler)
	{
		if (m_isTls)
		{
			return tls().async_write_some(buffers, BOOST_ASIO_MOVE_CAST(WriteHandler)(handler));
		}
		else
		{
			return tcp().async_write_some(buffers, BOOST_ASIO_MOVE_CAST(WriteHandler)(handler));
		}
	}

	/**
	* override async_read_some() to satisfy AsyncReadStream concept for both TLS and TCP
	* https://www.boost.org/doc/libs/1_74_0/doc/html/boost_asio/reference/AsyncReadStream.html
	* Adapted from boost/beast/ssl/ssl_stream.hpp
	*/
	template<class MutableBufferSequence, BOOST_BEAST_ASYNC_TPARAM2 ReadHandler>
	BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler, void(boost::system::error_code, std::size_t))
	async_read_some(MutableBufferSequence const &buffers, BOOST_ASIO_MOVE_ARG(ReadHandler) handler)
	{
		if (m_isTls)
		{
			return tls().async_read_some(buffers, BOOST_ASIO_MOVE_CAST(ReadHandler)(handler));
		}
		else
		{
			return tcp().async_read_some(buffers, BOOST_ASIO_MOVE_CAST(ReadHandler)(handler));
		}
	}

	/**
	* async_teardown handling is required when wrapped into boost::beast::websocket::stream
	* Override boost::beast::websocket::async_teardown() for vs::BeastTlsOrTcpStream
	* https://www.boost.org/doc/libs/1_74_0/libs/beast/doc/html/beast/ref/boost__beast__websocket__async_teardown.html
	*/
	template<class TeardownHandler>
	friend void async_teardown(boost::beast::role_type role, vs::BeastTlsOrTcpStream &stream, TeardownHandler &&handler)
	{
		if (stream.isTls())
		{
			stream.tls().async_shutdown(std::forward<TeardownHandler>(handler));
		}
		else
		{
			stream.tcp().close();
		}
	}

	void setIsTls(bool isTls = true) { m_isTls = isTls; }

	bool isTls() { return m_isTls; }

	// more readable version of next_layer()
	boost::beast::tcp_stream &tcp() { return next_layer(); }

private:
	bool m_isTls{false};

	// more readable version of BeastTlsStream::...
	BeastTlsStream &tls() { return *this; }
};

