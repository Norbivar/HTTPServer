#include "websocket_session.hpp"

#include <boost/asio/buffers_iterator.hpp>

#include <Logging>
#include <WebServer.hpp>
#include "websocket_tracker.hpp"

#include "handle_request.hpp"

void end_websocket_session(std::shared_ptr<ssl_websocket_session> session);

ssl_websocket_session::ssl_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream>&& stream) :
	websocket{ std::move(stream) }
{
}

ssl_websocket_session::~ssl_websocket_session()
{
	theLog->info("VN: DEBUG removed websock");
}

void ssl_websocket_session::do_accept(const beast_request& req)
{
	// Set suggested timeout settings for the websocket
	websocket.set_option(
		boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));

	// Set a decorator to change the Server of the handshake
	websocket.set_option(
		boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::response_type& res) {
			res.set(boost::beast::http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " advanced-server-flex");
			}));

	websocket.async_accept(
		req, boost::beast::bind_front_handler(&ssl_websocket_session::on_accept, shared_from_this()));
}

void ssl_websocket_session::do_refuse()
{
	//websocket.close(boost::beast::websocket::close_reason{ boost::beast::websocket::close_code::policy_error });
	websocket.async_close(
		boost::beast::websocket::close_reason{ boost::beast::websocket::close_code::policy_error },
		boost::beast::bind_front_handler(&ssl_websocket_session::on_close, shared_from_this()));
}


void ssl_websocket_session::send(const std::string& msg)
{
	send(boost::asio::buffer(msg));
}

void ssl_websocket_session::send(const boost::beast::flat_buffer& msg)
{
	send(msg.data());
}

void ssl_websocket_session::send(const boost::asio::const_buffer& msg)
{
	ws().text(websocket.got_text());

	websocket.write(msg);
	/*websocket.async_write(
		boost::asio::const_buffer{ msg },
		boost::beast::bind_front_handler(&ssl_websocket_session::on_write,
			shared_from_this()));*/
}

void ssl_websocket_session::on_accept(boost::beast::error_code ec)
{
	if (ec)
		return theLog->error("[accept]: {}", ec.message());

	do_read();
}

void ssl_websocket_session::on_close(boost::beast::error_code ec)
{
	if (ec)
		return theLog->error("[close]: {}", ec.message());
}


void ssl_websocket_session::do_read()
{
	receive_buffer.consume(receive_buffer.size());

	// Called by the https io_context
	websocket.async_read(
		receive_buffer, boost::beast::bind_front_handler(&ssl_websocket_session::on_read, shared_from_this()));
}

void ssl_websocket_session::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);

	if (ec == boost::beast::websocket::error::closed 
		|| ec == boost::asio::ssl::error::stream_errors::stream_truncated
		|| ec.value() == 10053)
	{
		theLog->info("Websocket closing");

		end_websocket_session(ssl_websocket_session::shared_from_this());

		websocket.async_close(
			boost::beast::websocket::close_reason{ boost::beast::websocket::close_code::normal },
			boost::beast::bind_front_handler(&ssl_websocket_session::on_close, shared_from_this()));

		return;
	}

	if (ec)
		theLog->error("[read]: Code {} : {}", ec.value(), ec.message());

	if (!verified)
	{
		std::string expected_code(boost::asio::buffers_begin(receive_buffer.data()), boost::asio::buffers_end(receive_buffer.data()));

		if (theServer.get_websocket_tracker().verify_websocket(expected_code, shared_from_this()))
		{
			verified = true;

			do_read();
			return;
		}
		else
		{
			do_refuse();
			return;
		}
	}

	std::string result(boost::asio::buffers_begin(receive_buffer.data()), boost::asio::buffers_end(receive_buffer.data()));
	theLog->info("Got Websock comm: {}", result);

	do_read();
}

void ssl_websocket_session::on_write(boost::beast::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);

	if (ec)
		return theLog->error("[write]: {}", ec.message());

	receive_buffer.consume(receive_buffer.size()); // Why?
}

void make_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream> stream, beast_request req)
{
	auto ws_sess = std::make_shared<ssl_websocket_session>(std::move(stream));

	theServer.get_websocket_tracker().add_unverified_websocket(ws_sess);
	ws_sess->do_accept(req);
}

void end_websocket_session(std::shared_ptr<ssl_websocket_session> session)
{
	theServer.get_websocket_tracker().remove_socket(session);
}