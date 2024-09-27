#include "websocket_tracker.hpp"

#include <WebServer.hpp>
#include <Logging>
#include "session_tracker.hpp"
#include "base/websocket_session.hpp"

#include <boost/range/adaptor/filtered.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

bool operator<(const websocket_session_key& lhs, const websocket_session_key& rhs)
{
	return std::lexicographical_compare(lhs.session_id.begin(), lhs.session_id.end(), rhs.session_id.begin(), rhs.session_id.end());
}

websocket_handshake_code websocket_tracker::get_new_handshake_code(const id::session& owner_session)
{
	constexpr auto code_length = 32;
	constexpr char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

	websocket_handshake_code code;
	code.reserve(code_length);

	boost::random::random_device rng;
	boost::random::uniform_int_distribution<> index_dist(0, sizeof(chars) - 2); // last one is a \0
	for (int i = 0; i < code_length; ++i)
		code.push_back(chars[index_dist(rng)]);

	std::unique_lock lock{ mutex };
	waiting_handshake_codes.emplace_back(code, owner_session);

	return code;
}

void websocket_tracker::add_unverified_websocket(std::shared_ptr<ssl_websocket_session> socket)
{
	std::unique_lock lock{ mutex };
	unverified_websockets.push_back(socket);
}

bool websocket_tracker::verify_websocket(
	const websocket_handshake_code& code,
	std::shared_ptr<ssl_websocket_session> socket)
{
	std::unique_lock lock{ mutex };
	const auto sock_it = std::find(unverified_websockets.begin(), unverified_websockets.end(), socket);

	auto result = false;
	if (sock_it != unverified_websockets.end())
	{
		const auto code_it = std::find_if(waiting_handshake_codes.begin(), waiting_handshake_codes.end(), [&code](const auto& item) { return item.first == code; });
		if (code_it != waiting_handshake_codes.end())
		{
			auto [exists, session] = theServer.get_session_tracker().find_by_session_id(code_it->second);
			if (exists)
			{
				websocket_session_key new_key{ code_it->second, *sock_it, session->session };

				unverified_websockets.erase(sock_it);
				waiting_handshake_codes.erase(code_it);

				websockets.emplace_back(std::move(new_key));
				
				result = true;
			}
		}
	}

	theLog->info("Verify websocket {} for code '{}'", 
		result ? "SUCCESFUL" : "FAILED",
		code);

	return result;
}

std::vector<std::shared_ptr<ssl_websocket_session>> websocket_tracker::find_sockets(const id::session& owner_session) const
{
	std::vector<std::shared_ptr<ssl_websocket_session>> sockets_found;

	std::shared_lock lock{ mutex };

	const auto suitable_sockets = websockets | boost::adaptors::filtered([&](const auto& item) { return item.session_id == owner_session; });

	for (const auto& socket : suitable_sockets) {
		sockets_found.emplace_back(socket.websocket);
	}

	return sockets_found;
}

void websocket_tracker::remove_socket(std::shared_ptr<ssl_websocket_session> socket)
{
	std::unique_lock lock{ mutex };

	const auto it = std::find_if(websockets.begin(), websockets.end(), [&socket](const auto& elem) { return elem.websocket == socket; });
	if (it != websockets.end())
		websockets.erase(it);

	boost::range::remove_erase(unverified_websockets, socket);
}