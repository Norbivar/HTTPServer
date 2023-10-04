#include "network_component.h"

#include <boost/asio/ip/address.hpp>

#include <Logging>
#include "base/listener.hpp"

https_network_component::https_network_component(const boost::asio::ip::address& address, std::uint16_t port, std::uint8_t desired_thread_number) :
	address{ address },
	port{ port },
	desired_thread_number{ desired_thread_number },
	ioc{ desired_thread_number },
	ctx{ boost::asio::ssl::context::tlsv13 }
{
	assert(desired_thread_number != 0);
	threads.reserve(desired_thread_number);

	interrupt_signal = std::make_unique<boost::asio::signal_set>(ioc, SIGINT, SIGTERM);
}

void https_network_component::setup_run()
{
	std::make_shared<listener>(ioc, ctx, boost::asio::ip::tcp::endpoint{ address, port })->run();

	// Capture SIGINT and SIGTERM to perform a clean shutdown
	interrupt_signal->async_wait([&](const boost::beast::error_code&, int) {
		theLog->warn("Encountered SIGINT/SIGTERM!");
		this->stop();
	});

	for (auto i = 0; i < desired_thread_number; ++i)
		threads.emplace_back([this] { ioc.run(); });

	theLog->info("-> HTTPS request handling is runnning on port: {}, threads running: {} ✓", port, threads.size());
}

void https_network_component::await_finish()
{
	for (auto& t : threads)
		t.join();
}

bool https_network_component::load_certificate()
{
	const auto& cert_dir = theConfig->cert_dir;
	theLog->info("Loading cert from '{}'", cert_dir);

	try {
		ctx.set_password_callback([](std::size_t, boost::asio::ssl::context_base::password_purpose) {
			return "asdfghjk";
		});

		ctx.set_options(
			boost::asio::ssl::context::default_workarounds |
			boost::asio::ssl::context::no_sslv2 |
			boost::asio::ssl::context::single_dh_use);

		ctx.use_certificate_chain_file(cert_dir + "certificate.crt");
		ctx.use_private_key_file(cert_dir + "key.key", boost::asio::ssl::context::file_format::pem);
		ctx.use_tmp_dh_file(cert_dir + "dhparam.pem");
	}
	catch (const std::exception& ex) {
		theLog->error(ex);
		return false;
	}

	return true;
}

void https_network_component::stop()
{
	ioc.stop();
	await_finish();
}