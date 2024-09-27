#pragma once

#include <memory>
#include <vector>
#include <map>

#include "id_types.hpp"
#include "threadsafe.hpp"

#include <boost/container/stable_vector.hpp>
#include <utils/shared_mutex_write_prefered.hpp>

class ssl_websocket_session;
struct session_element;

using websocket_handshake_code = std::string;

struct websocket_session_key
{
	const id::session session_id;

	std::shared_ptr<ssl_websocket_session> websocket;
	std::shared_ptr<threadsafe::element<session_element>> session;
};

bool operator<(const websocket_session_key& lhs, const websocket_session_key& rhs);

class websocket_tracker
{
public:
	websocket_tracker() = default;

	websocket_handshake_code get_new_handshake_code(const id::session& owner_session);

	//bool store_new_socket(const id::session& owner_session, std::shared_ptr<ssl_websocket_session> new_socket);
	void add_unverified_websocket(std::shared_ptr<ssl_websocket_session> socket);
	bool verify_websocket(const websocket_handshake_code& code, std::shared_ptr<ssl_websocket_session> socket);

	void remove_socket(std::shared_ptr<ssl_websocket_session> new_socket);
	void remove_sockets(const id::session& owner_session);

	std::vector<std::shared_ptr<ssl_websocket_session>> find_sockets(const id::session& owner_session) const;

private:
	mutable std::shared_mutex mutex;

	boost::container::stable_vector<websocket_session_key> websockets;
	
	std::vector<std::pair<websocket_handshake_code, id::session>> waiting_handshake_codes;
	std::vector<std::shared_ptr<ssl_websocket_session>> unverified_websockets;

};