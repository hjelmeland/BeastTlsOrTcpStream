# BeastTlsOrTcpStream
Wrapper of  boost::beast::ssl_stream&lt;boost::beast::tcp_stream> dynamic TLS or TCP

Class BeastTlsStream can be used both as TLS/SSL stream or as plain TCP stream.

BeastTlsStream provides methods with signatures allowing it to be passed to
boost::beast::http::async_xxxx functions. And also be wrapped to WebSocket
stream, boost::beast::websocket::stream<BeastTlsOrTcpStream> for async
operations.

BeastTlsStream start in TCP mode. In TCP mode it bypasses the SSL layer,
async_write_some() / async_read_some() go to the TCP layer.
Call setIsTls() at the start of the session to set to TLS mode, and
async_write_some() / async_read_some() will go through the SSL layer.

Consider the snippet public domain, added a palette of liberal licenses
for those that need it.
