#include "WebServer.hpp"

#include <boost/asio/ip/address.hpp>
#include <boost/asio/signal_set.hpp>

#include <Logging>
#include "base/listener.hpp"
#include "base/websocket_session.hpp"
#include "base/http_session.hpp"

#include "routing_table.hpp"
#include "session_tracker.hpp"
#include "database/sql/sql_manager.hpp"

webserver::webserver() : webserver(
	theConfig->get<Configs::doc_root>("./web/"),
	boost::asio::ip::make_address(theConfig->get<Configs::bind_ip>("0.0.0.0")),
	theConfig->get<Configs::port>(443),
	theConfig->get<Configs::threads>(3))
{ }

webserver::webserver(std::string&& doc_root, const boost::asio::ip::address& address, const uint16_t port, std::uint8_t numthreads) :
	m_address{address},
	m_port{port},
	m_doc_root{std::move(doc_root)},
	m_desired_thread_number{numthreads},
	m_ioc{numthreads},
	m_ctx{boost::asio::ssl::context::tlsv13},
	m_routing_table{std::make_unique<routing_table>()},
	m_session_tracker{std::make_unique<session_tracker>()},
	m_sql_manager{std::make_unique<sql_manager>()}
{
	assert(numthreads != 0);
	m_threads.reserve(numthreads - 1);
}

webserver::~webserver() { }

void webserver::bootstrap()
{
	theLog->info("Bootstrapping WebServer...");
	// This holds the self-signed certificate used by the server
	load_server_certificate();
	theLog->info("->	Certificates loaded.");
	m_routing_table->register_all();
	theLog->info("->	Path mapping set up.");
	m_routing_table->print_stats();
}

int webserver::run()
{
	boost::beast::error_code ec;

	theLog->info("Preparing WebServer...");

	std::make_shared<listener>(m_ioc, m_ctx, boost::asio::ip::tcp::endpoint{ m_address, m_port })->run();

	// Capture SIGINT and SIGTERM to perform a clean shutdown
	boost::asio::signal_set signals(m_ioc, SIGINT, SIGTERM);
	signals.async_wait([&](const boost::beast::error_code&, int) {
		m_ioc.stop();
	});

	for (auto i = 1; i < m_desired_thread_number; ++i)
		m_threads.emplace_back([this] { m_ioc.run(); });

	theLog->info("WebServer runnning on port {}", m_port);

	m_ioc.run();

	// Block until all the threads exit
	for (auto& t : m_threads)
		t.join();

	theLog->info("WebServer stopped.");

	return EXIT_SUCCESS;
}

void webserver::load_server_certificate()
{
	const auto cert_dir = theConfig->get<Configs::cert_dir>("./cert");

	m_ctx.set_password_callback([](std::size_t, boost::asio::ssl::context_base::password_purpose) {
		return "asdfghjk";
	});

	m_ctx.set_options(
		boost::asio::ssl::context::default_workarounds |
		boost::asio::ssl::context::no_sslv2 |
		boost::asio::ssl::context::single_dh_use);

	m_ctx.use_certificate_chain_file(cert_dir + "/certificate.crt");
	m_ctx.use_private_key_file(cert_dir + "/key.key", boost::asio::ssl::context::file_format::pem);
	m_ctx.use_tmp_dh_file(cert_dir + "/dhparam.pem");
}