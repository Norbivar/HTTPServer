#pragma once

#include <memory>
#include <vector>

#include <boost/optional/optional.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/write.hpp>

#include <Logging>

// Handles an SSL HTTP connection
class ssl_http_session : public std::enable_shared_from_this<ssl_http_session> 
{
public:
	// Create the http_session
	ssl_http_session(boost::beast::tcp_stream&& stream, boost::asio::ssl::context& ctx, boost::beast::flat_buffer&& buffer);

	boost::beast::ssl_stream<boost::beast::tcp_stream>& stream() { return stream_; }

	// Start the session
	void run();

private:
	// This queue is used for HTTP pipelining.
	class queue
	{
		// Maximum number of responses we will queue
		enum {
			limit = 8
		};

		// The type-erased, saved work item
		struct work {
			virtual ~work() = default;
			virtual void operator()() = 0;
		};

		ssl_http_session& self_;
		std::vector<std::unique_ptr<work>> items_;

	public:
		explicit queue(ssl_http_session& self) :
			self_(self)
		{
			static_assert(limit > 0, "queue limit must be positive");
			items_.reserve(limit);
		}

		// Returns true if we have reached the queue limit
		bool is_full() const { return items_.size() >= limit; }

		// Called when a message finishes sending
		// Returns `true` if the caller should initiate a read
		bool on_write()
		{
			BOOST_ASSERT(!items_.empty());
			auto const was_full = is_full();
			items_.erase(items_.begin());
			if (!items_.empty())
				(*items_.front())();
			return was_full;
		}

		// Called by the HTTP handler to send a response.
		template <bool isRequest, class Body, class Fields>
		void operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg)
		{
			// This holds a work item
			struct work_impl : work {
				ssl_http_session& self_;
				boost::beast::http::message<isRequest, Body, Fields> msg_;

				work_impl(ssl_http_session& self,
					boost::beast::http::message<isRequest, Body, Fields>&& msg) :
					self_(self),
					msg_(std::move(msg))
				{ }

				void operator()()
				{
					::boost::beast::http::async_write(
						self_.stream(), msg_,
						boost::beast::bind_front_handler(&ssl_http_session::on_write,
							self_.shared_from_this(),
							msg_.need_eof()));
				}
			};

			// Allocate and store the work
			items_.push_back(std::make_unique<work_impl>(self_, std::move(msg)));

			// If there was no previous work, start this one
			if (items_.size() == 1)
				(*items_.front())();
		}
	};

	// Called by the base class
	boost::beast::ssl_stream<boost::beast::tcp_stream> release_stream() { return std::move(stream_); }

	// Called by the base class
	void do_eof();

	void do_read();

	void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);

	void on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred);

	void on_handshake(boost::beast::error_code ec, std::size_t bytes_used);

	void on_shutdown(boost::beast::error_code ec);

	queue queue_;

	// The parser is stored in an optional container so we can
	// construct it from scratch it at the beginning of each new message.
	boost::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> parser_;

	boost::beast::flat_buffer buffer_;

	boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;
};
