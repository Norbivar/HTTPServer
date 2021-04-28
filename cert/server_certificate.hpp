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
	/*
		The certificate was generated from CMD.EXE on Windows 10 using:

		winpty openssl dhparam -out dh.pem 2048
		winpty openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 10000 -out cert.pem -subj "//C=US\ST=CA\L=Los Angeles\O=Beast\CN=www.example.com"
	*/

	ctx.set_password_callback([](std::size_t, boost::asio::ssl::context_base::password_purpose)
	{
		return "test";
	});

	ctx.set_options(
		boost::asio::ssl::context::default_workarounds |
		boost::asio::ssl::context::no_sslv2 |
		boost::asio::ssl::context::single_dh_use);

	ctx.use_certificate_chain_file("cert/certificate.pem");

	ctx.use_private_key_file("cert/key.pem", boost::asio::ssl::context::file_format::pem);

	ctx.use_tmp_dh_file("cert/dhparam.pem");
}