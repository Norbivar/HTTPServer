#include "websocket_tracker.hpp"

#include <WebServer.hpp>
#include <Logging>
#include "session_tracker.hpp"
#include "base/websocket_session.hpp"

#include <boost/range/adaptor/filtered.hpp>

bool operator<(const websocket_session_key& lhs, const websocket_session_key& rhs)
{
	return std::lexicographical_compare(lhs.session_id.begin(), lhs.session_id.end(), rhs.session_id.begin(), rhs.session_id.end());
}

bool websocket_tracker::store_new_socket(const id::session owner_session, std::shared_ptr<ssl_websocket_session> new_socket)
{
	auto [exists, session] = theServer.get_session_tracker().find_by_session_id(owner_session);
	if (!exists)
	{
		theLog->error("Tried upgrading to websocket with invalid session id!");
		return false;
	}

	std::unique_lock lock{ mutex };

	websocket_session_key new_key{ owner_session, new_socket, session->session };
	websockets.emplace_back(std::move(new_key));

	return true;
}

std::vector<std::shared_ptr<ssl_websocket_session>> websocket_tracker::find_sockets(const id::session owner_session) const
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
	else
		theLog->warn("Tried to remove websocket session that is not in the tracker!!");
}