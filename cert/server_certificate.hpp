#pragma once

#include <boost/asio/buffer.hpp>
#include <boost/asio/ssl/context.hpp>
#include <cstddef>
#include <memory>

/*  Load a signed certificate into the ssl context, and configure
	the context for use with a server.

	For this to work with the browser or operating system, it is
	necessary to import the "Beast Test CA" certificate into
	the local certificate store, browser, or operating system
	depending on your environment Please see the documentation
	accompanying the Beast certificate for more details.
*/
inline void load_server_certificate(boost::asio::ssl::context& ctx)
{
	const auto cert_dir = theConfig->get<Configs::cert_dir>("./cert");

	ctx.set_password_callback([](std::size_t, boost::asio::ssl::context_base::password_purpose)
	{
		return "asdfghjk";
	});

	ctx.set_options(
		boost::asio::ssl::context::default_workarounds |
		boost::asio::ssl::context::no_sslv2 |
		boost::asio::ssl::context::single_dh_use);

	ctx.use_certificate_chain_file(cert_dir + "/certificate.crt");
	ctx.use_private_key_file(cert_dir + "/key.key", boost::asio::ssl::context::file_format::pem);
	ctx.use_tmp_dh_file(cert_dir + "/dhparam.pem");
}