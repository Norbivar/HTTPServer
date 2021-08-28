#pragma once

#include <memory>
#include <vector>

#include <boost/optional/optional.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/string_body.hpp>

#include <boost/lockfree/queue.hpp>

#include <Logging>
#include "../http_response.hpp"

class ssl_http_session;

struct work {
	virtual ~work() = default;
	virtual void operator()() = 0;
};

class response_queue
{
	// Maximum number of responses we will queue
	static constexpr auto limit = 16;

	ssl_http_session& self_;
	std::vector<std::unique_ptr<work>> items_;

public:
	explicit response_queue(ssl_http_session& self);

	// Returns true if we have reached the queue limit
	bool is_full() const;

	// Called when a message finishes sending
	// Returns `true` if the caller should initiate a read
	bool on_write();
	void process(beast_response&& msg);
	void process_file(beast_response_file&& msg);
private:
	void add(std::unique_ptr<work>&& item);
};

// Handles an SSL HTTP connection
class ssl_http_session : public std::enable_shared_from_this<ssl_http_session> 
{
	friend response_queue;
	template<typename T> friend struct work_impl;

public:
	// Create the http_session
	ssl_http_session(boost::beast::tcp_stream&& stream, boost::asio::ssl::context& ctx, boost::beast::flat_buffer&& buffer);

	boost::beast::ssl_stream<boost::beast::tcp_stream>& stream() { return stream_; }

	// Start the session
	void run();

private:
	// Called by the base class
	boost::beast::ssl_stream<boost::beast::tcp_stream> release_stream() { return std::move(stream_); }

	// Called by the base class
	void do_eof();

	void do_read();

	void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);

	void on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred);

	void on_handshake(boost::beast::error_code ec, std::size_t bytes_used);

	void on_shutdown(boost::beast::error_code ec);

	response_queue queue_;

	// The parser is stored in an optional container so we can
	// construct it from scratch it at the beginning of each new message.
	boost::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> parser_;

	boost::beast::flat_buffer buffer_;

	boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;
};
