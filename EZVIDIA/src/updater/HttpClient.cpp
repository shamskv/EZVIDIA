#include "HttpClient.hpp"
// boost needs to know the target version
#include "../targetver.h"
// boost network stuff
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
// windows access to certificates
#include <wincrypt.h>
// json lib
#include <json.hpp>
// internal stuff
#include "../logging/Logger.hpp"

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
namespace ssl = net::ssl;			// from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>
using nlohmann::json;

namespace {
	template<class T>
	std::optional <T> convertResponse(const std::vector<char>& resp);

	template<class T>
	std::optional <T> convertResponse(const std::vector<char>& resp) = delete;

	template<>
	std::optional<json> convertResponse<json>(const std::vector<char>& resp) {
		LOG(DEBUG) << "Extracting as json object";
		try {
			std::string bodyString(resp.data(), resp.size());
			return nlohmann::json::parse(bodyString);
		}
		catch (std::exception& e) {
			LOG(ERR) << "Problem extracting json object from response body" << e.what();
			return std::nullopt;
		}
	}

	template<>
	std::optional<std::vector<char>> convertResponse<std::vector<char>>(const std::vector<char>& resp) {
		LOG(DEBUG) << "Extracting as byte vector";
		return resp;
	}

	void add_windows_root_certs(boost::asio::ssl::context& ctx) {
		HCERTSTORE hStore = CertOpenSystemStore(0, L"ROOT");
		if (hStore == NULL) {
			return;
		}

		X509_STORE* store = X509_STORE_new();
		PCCERT_CONTEXT pContext = NULL;
		while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) != NULL) {
			X509* x509 = d2i_X509(NULL,
				(const unsigned char**)&pContext->pbCertEncoded,
				pContext->cbCertEncoded);
			if (x509 != NULL) {
				X509_STORE_add_cert(store, x509);
				X509_free(x509);
			}
		}

		CertFreeCertificateContext(pContext);
		CertCloseStore(hStore, 0);

		SSL_CTX_set_cert_store(ctx.native_handle(), store);
	}
}

template<class T>
std::optional<T> HttpClient::makeGetRequest(const std::string& host, const std::string& target, const std::string& contentType) {
	try {
		LOG(DEBUG) << "Making request to " << host.c_str();

		// The io_context is required for all I/O
		net::io_context ioc;
		// The SSL context is required, and holds certificates
		ssl::context ctx(ssl::context::tlsv12_client);

		// This holds the root certificate used for verification
		add_windows_root_certs(ctx);

		// Verify the remote server's certificate
		ctx.set_verify_mode(ssl::verify_peer);

		// These objects perform our I/O
		tcp::resolver resolver(ioc);
		beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

		// Set SNI Hostname (many hosts need this to handshake successfully)
		if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
			beast::error_code ec{ static_cast<int>(::ERR_get_error()), net::error::get_ssl_category() };
			throw beast::system_error{ ec };
		}

		// Look up the domain name
		auto const results = resolver.resolve(host, "https");

		// Make the connection on the IP address we get from a lookup
		beast::get_lowest_layer(stream).connect(results);
		// Perform the SSL handshake
		stream.handshake(ssl::stream_base::client);

		// Set up an HTTP GET request message
		http::request<http::string_body> req{ http::verb::get, target, 11 };
		req.set(http::field::host, host);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		req.set(http::field::accept, contentType);

		// Send the HTTP request to the remote host
		http::write(stream, req);

		// This buffer is used for reading and must be persisted
		beast::flat_buffer buffer;

		// Declare a container to hold the response
		http::response<http::vector_body<char>> res;

		// Receive the HTTP response
		http::read(stream, buffer, res);

		if (res.result_int() != 200) {
			LOG(ERR) << "Request with host " << host.c_str() << " and target " << target.c_str() << " failed with code " << res.result_int();
			return std::nullopt;
		}

		auto& vectorBody = res.body();

		// Gracefully close the socket
		beast::error_code ec;
		stream.shutdown(ec);

		// not_connected happens sometimes
		// so don't bother reporting it.
		//
		if (ec && ec != net::error::eof && ec != ssl::error::stream_truncated)
			throw beast::system_error{ ec };

		// If we get here then the connection is closed gracefully

		return convertResponse<T>(vectorBody);
	}
	catch (std::exception const& e) {
		LOG(ERR) << "Error: " << e.what();
		return std::nullopt;
	}
}

// Instantiate the two implementations we will need
template std::optional<std::vector<char>> HttpClient::makeGetRequest<std::vector<char>>(const std::string&, const std::string&, const std::string&);
template std::optional<json> HttpClient::makeGetRequest<json>(const std::string&, const std::string&, const std::string&);