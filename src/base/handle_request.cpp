#include "handle_request.hpp"

#include <boost/beast/core/string.hpp>
#include <boost/beast/version.hpp>

#include <Logging>

#include "../webserver.hpp"
#include "../session_tracker.hpp"
#include "../routing_table.hpp"
#include "http_session.hpp"

boost::beast::string_view mime_type(boost::beast::string_view path)
{
	namespace beast = boost::beast;

	using beast::iequals;
	auto const ext = [&path] {
		auto const pos = path.rfind(".");
		if (pos == beast::string_view::npos)
			return beast::string_view{};
		return path.substr(pos);
	}();
	if (iequals(ext, ".htm"))
		return "text/html";
	if (iequals(ext, ".html"))
		return "text/html";
	if (iequals(ext, ".php"))
		return "text/html";
	if (iequals(ext, ".css"))
		return "text/css";
	if (iequals(ext, ".txt"))
		return "text/plain";
	if (iequals(ext, ".js"))
		return "application/javascript";
	if (iequals(ext, ".json"))
		return "application/json";
	if (iequals(ext, ".xml"))
		return "application/xml";
	if (iequals(ext, ".swf"))
		return "application/x-shockwave-flash";
	if (iequals(ext, ".flv"))
		return "video/x-flv";
	if (iequals(ext, ".png"))
		return "image/png";
	if (iequals(ext, ".jpe"))
		return "image/jpeg";
	if (iequals(ext, ".jpeg"))
		return "image/jpeg";
	if (iequals(ext, ".jpg"))
		return "image/jpeg";
	if (iequals(ext, ".gif"))
		return "image/gif";
	if (iequals(ext, ".bmp"))
		return "image/bmp";
	if (iequals(ext, ".ico"))
		return "image/vnd.microsoft.icon";
	if (iequals(ext, ".tiff"))
		return "image/tiff";
	if (iequals(ext, ".tif"))
		return "image/tiff";
	if (iequals(ext, ".svg"))
		return "image/svg+xml";
	if (iequals(ext, ".svgz"))
		return "image/svg+xml";
	return "application/text";
}

std::string path_cat(boost::beast::string_view base, boost::beast::string_view path)
{
	if (base.empty())
		return std::string(path);
	std::string result(base);
#ifdef BOOST_MSVC
	char constexpr path_separator = '\\';
	if (result.back() == path_separator)
		result.resize(result.size() - 1);
	result.append(path.data(), path.size());
	for (auto& c : result)
		if (c == '/')
			c = path_separator;
#else
	char constexpr path_separator = '/';
	if (result.back() == path_separator)
		result.resize(result.size() - 1);
	result.append(path.data(), path.size());
#endif
	return result;
}

beast_response set_bad_request(const beast_request& req, boost::beast::string_view why)
{
	beast_response res{ boost::beast::http::status::bad_request, req.version() };
	res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(boost::beast::http::field::content_type, "text/html");
	res.keep_alive(req.keep_alive());
	res.body() = std::string(why);
	res.prepare_payload();
	return res;
}

beast_response set_not_found(const beast_request& req)
{
	beast_response res{ boost::beast::http::status::not_found, req.version() };
	res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(boost::beast::http::field::content_type, "text/html");
	res.keep_alive(req.keep_alive());
	res.body() = "The resource '" + std::string(req.target()) + "' was not found.";
	res.prepare_payload();
	return res;
}

beast_response set_server_error(const beast_request& req, boost::beast::string_view what)
{
	beast_response res{ boost::beast::http::status::internal_server_error, req.version() };
	res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(boost::beast::http::field::content_type, "text/html");
	res.keep_alive(req.keep_alive());
	res.body() = "An error occurred: '" + std::string(what) + "'";
	res.prepare_payload();
	return res;
}

beast_response set_unauthorized(const beast_request& req, bool kill_session = false)
{
	beast_response res{ boost::beast::http::status::unauthorized, req.version() };
	res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(boost::beast::http::field::content_type, "text/html");
	if (kill_session)
		res.set(boost::beast::http::field::set_cookie, "SID=deleted; expires=Thu, 01 Jan 1970 00:00:00 GMT");

	res.keep_alive(req.keep_alive());
	res.prepare_payload();
	return res;
}

beast_response respond_head_request(const std::string& path, const std::uint64_t size, const beast_request& req)
{
	beast_response res{ boost::beast::http::status::ok, req.version() };
	res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(boost::beast::http::field::content_type, mime_type(path));
	res.content_length(size);
	res.keep_alive(req.keep_alive());
	return res;
}

void handle_request(std::string&& from_addr, beast_request&& req, response_queue& resp_queue)
{
	const auto target = req.target();
	// Request path must be absolute and not contain "..".
	if (target.empty() || target[0] != '/' || target.find("..") != boost::beast::string_view::npos)
		return resp_queue.process(set_bad_request(req, "Illegal request-target"));

	// Build the path to the requested file
	std::string path = path_cat(theServer.get_doc_root(), target);
	if (target.back() == '/')
		path.append("index.html");

	const auto mime = mime_type(path);
	if (mime != "application/text")
	{
		// Attempt to open the file
		boost::beast::error_code ec;
		boost::beast::http::file_body::value_type body;
		body.open(path.c_str(), boost::beast::file_mode::scan, ec); // TODO: this seems like a neat little security leak :)

		// Handle the case where the file doesn't exist
		if (ec == boost::beast::errc::no_such_file_or_directory)
			return resp_queue.process(set_not_found(req));

		// Handle an unknown error
		if (ec)
			return resp_queue.process(set_server_error(req, ec.message()));

		// Cache the size since we need it after the move
		const auto size = body.size();

		// Respond to HEAD request
		if (req.method() == boost::beast::http::verb::head)
			return resp_queue.process(respond_head_request(path, size, req));

		// Respond to GET request
		boost::beast::http::response<boost::beast::http::file_body> res
		{
			std::piecewise_construct,
			std::make_tuple(std::move(body)),
			std::make_tuple(boost::beast::http::status::ok, req.version())
		};
		res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(boost::beast::http::field::content_type, mime_type(path));
		res.content_length(size);
		res.keep_alive(req.keep_alive());
		return resp_queue.process_file(std::move(res));
	}
	else
	{
		auto req_target_stripped = target.to_string();
		auto get_method_question_mark = req_target_stripped.find_first_of('?');
		if (get_method_question_mark != std::string::npos)
			req_target_stripped = req_target_stripped.substr(0, get_method_question_mark);

		const auto [success, it] = theServer.get_routing_table().lookup(req.method(), req_target_stripped);
		if (!success)
			return resp_queue.process(set_not_found(req));

		const auto request_id = theServer.fetch_add_request_count();
		Libs::Logger::Tag _{ fmt::format("req id {}", request_id)};
		Libs::Logger::Tag __{ fmt::format("url {}", target.to_string())};

		try
		{
			http_request request{
				request_id,
				std::move(req), 
				std::move(from_addr) 
			};

			auto [session_found, session_it] = theServer.get_session_tracker().find_by_session_id(request.sid);
			if (session_found)
			{
				request.session = session_it->session;
				const auto read_session = request.session->acquire();

				if (read_session->deactivated)
				{
					theLog->info("Unauthorized: Session deactivated.");
					return resp_queue.process(set_unauthorized(req, true));
				}

				if (!read_session->ip_address.empty() && read_session->ip_address != request.address)
				{
					theLog->info("Unauthorized: Request ip address different than saved session ip. Blocking.");
					return resp_queue.process(set_unauthorized(req));
				}
			}
			else 
			{
				if (!request.sid.empty())
				{
					theLog->info("Unauthorized: Unknown SID.");
					return resp_queue.process(set_unauthorized(req, true));
				}

				if (it->second.need_session)
				{
					theLog->info("Unauthorized: Handler requires session.");
					return resp_queue.process(set_unauthorized(req, true));
				}
			}

			if (it->second.access_predicate)
			{
				bool result = it->second.access_predicate(request);
				if (!result)
				{
					theLog->info("Unauthorized: Handler access predicate failed.");
					return resp_queue.process(set_unauthorized(req));
				}
			}

			if (session_found)
				session_it->session->modify([](session_element& sess) { sess.last_request_time = std::chrono::system_clock::now(); });

			http_response response{ request.base().version(), req.keep_alive() };
			it->second.handler(request, response);

			return resp_queue.process(std::move(response.prepare_release()));
		}
		catch (const nlohmann::json::parse_error& e)
		{
			theLog->error("Json parse error: {}", e.what());

			http_response response{ req.version(), req.keep_alive() };
			response.response_code(boost::beast::http::status::bad_request);
			return resp_queue.process(std::move(response.prepare_release()));
		}
		catch (const nlohmann::json::out_of_range& e)
		{
			theLog->error("Json params not found: {}", e.what());

			http_response response{ req.version(), req.keep_alive() };
			response.response_code(boost::beast::http::status::bad_request);
			return resp_queue.process(std::move(response.prepare_release()));
		}
		catch (const std::invalid_argument& e)
		{
			theLog->error("Invalid argument error: {}", e.what());

			http_response response{ req.version(), req.keep_alive() };
			response.response_code(boost::beast::http::status::bad_request);
			response["error_message"] = e.what();
			return resp_queue.process(std::move(response.prepare_release()));
		}
		catch (const std::runtime_error& e)
		{
			theLog->error("Runtime error: {}", e.what());

			http_response response{ req.version(), req.keep_alive() };
			response.response_code(boost::beast::http::status::internal_server_error);
			response["error_message"] = e.what();
			return resp_queue.process(std::move(response.prepare_release()));
		}
		catch (const std::logic_error& e)
		{
			theLog->error("Logic error: {}", e.what());

			http_response response{ req.version(), req.keep_alive() };
			response.response_code(boost::beast::http::status::internal_server_error);
			return resp_queue.process(std::move(response.prepare_release()));
		}
		catch (const std::exception& e)
		{
			theLog->critical("Root catched exception: {}", e.what());

			http_response response{ req.version(), req.keep_alive() };
			response.response_code(boost::beast::http::status::internal_server_error);
			return resp_queue.process(std::move(response.prepare_release()));
		}
	}
}