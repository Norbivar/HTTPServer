#include "websocket_session.hpp"

#include <Logging>
#include "handle_request.hpp"

ssl_websocket_session::ssl_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream>&& stream) :
	websocket{ std::move(stream) }
{
	theLog->info("VN: DEBUG created websock explicit");
}

ssl_websocket_session::~ssl_websocket_session()
{
	theLog->info("VN: DEBUG removed websock");
}

void ssl_websocket_session::send(const boost::beast::flat_buffer& msg)
{
	ws().text(websocket.got_text());
	websocket.async_write(
		msg.data(),
		boost::beast::bind_front_handler(&ssl_websocket_session::on_write,
			shared_from_this()));
}

void ssl_websocket_session::on_accept(boost::beast::error_code ec)
{
	if (ec)
		return theLog->error("[accept]: {}", ec.message());

	do_read();
}

void ssl_websocket_session::do_read()
{
	websocket.async_read(
		receive_buffer, boost::beast::bind_front_handler(&ssl_websocket_session::on_read, shared_from_this()));
}

void ssl_websocket_session::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);

	// This indicates that the websocket_session was closed
	if (ec == boost::beast::websocket::error::closed)
		return;

	if (ec)
		theLog->error("[read]: {}", ec.message());

	handle_websocket_request();
	receive_buffer.consume(receive_buffer.size());

	// Echo the message
	//websocket.text(websocket.got_text());
	//websocket.async_write(
	//	buffer_.data(),
	//	boost::beast::bind_front_handler(&websocket_session::on_write,
	//		derived().shared_from_this()));
}

void ssl_websocket_session::on_write(boost::beast::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);

	if (ec)
		return theLog->error("[write]: {}", ec.message());

	receive_buffer.consume(receive_buffer.size());

	do_read();
}