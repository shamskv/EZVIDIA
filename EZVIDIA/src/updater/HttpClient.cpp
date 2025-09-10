#include "HttpClient.hpp"
// boost needs to know the target version
#include "../targetver.h"
// boost network stuff
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
// string trim
#include <boost/algorithm/string.hpp>
// windows access to certificates
#include <wincrypt.h>
// json lib
#include <nlohmann/json.hpp>
// internal stuff
#include "../logging/Logger.hpp"

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;    // from <boost/beast/http.hpp>
namespace net = boost::asio;     // from <boost/asio.hpp>
namespace ssl = net::ssl;        // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;        // from <boost/asio/ip/tcp.hpp>
using nlohmann::json;

namespace {
template <class T>
std::optional<T> convertResponse(const std::vector<char>& resp);

template <class T>
std::optional<T> convertResponse(const std::vector<char>& resp) = delete;

template <>
std::optional<json> convertResponse<json>(const std::vector<char>& resp) {
  LOG(DEBUG) << "Extracting as json object";
  try {
    std::string bodyString(resp.data(), resp.size());
    return nlohmann::json::parse(bodyString);
  } catch (std::exception& e) {
    LOG(ERR) << "Problem extracting json object from response body" << e.what();
    return std::nullopt;
  }
}

template <>
std::optional<std::vector<char>> convertResponse<std::vector<char>>(
    const std::vector<char>& resp) {
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
    X509* x509 = d2i_X509(NULL, (const unsigned char**)&pContext->pbCertEncoded,
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
}  // namespace

template <class T>
std::optional<T> HttpClient::makeGetRequestSslNoRedirect(
    const std::string& url, const std::string& contentType) {
  try {
    LOG(DEBUG) << "Making request to " << url.c_str();

    // Tranform url into protocol/host/target
    const unsigned int ARRAY_SIZE = 500;
    char protocol[ARRAY_SIZE] = {0}, host[ARRAY_SIZE] = {0},
         target[ARRAY_SIZE] = {0};
    if (sscanf_s(url.c_str(), "%[^:]://%[^/]/%s", protocol, ARRAY_SIZE, host,
                 ARRAY_SIZE, target + 1, ARRAY_SIZE - 1) != 3) {
      LOG(ERR)
          << "Failed to extract protocol host or target from the provide url";
      return std::nullopt;
    }
    target[0] = '/';

    // Only use this function with https, not sure what would happen with other
    // protocols
    if (strcmp("https", protocol)) {
      LOG(ERR) << "Protocol is not https";
      return std::nullopt;
    }

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
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host)) {
      beast::error_code ec{static_cast<int>(::ERR_get_error()),
                           net::error::get_ssl_category()};
      throw beast::system_error{ec};
    }

    // Look up the domain name
    auto const results = resolver.resolve(host, "https");

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(stream).connect(results);
    // Perform the SSL handshake
    stream.handshake(ssl::stream_base::client);

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, target, 11};
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
      LOG(DEBUG) << "Request with host " << host << " and target " << target
                 << " failed with code " << res.result_int();
      if (res.result_int() / 100 == 3) {
        std::string redirectUrl = res[http::field::location].data();
        if (!redirectUrl.empty()) {
          redirectUrl = redirectUrl.substr(0, redirectUrl.find("\n"));
          HttpClient::redirectUrl = redirectUrl;
        }
      }
      return std::nullopt;
    }

    auto& vectorBody = res.body();

    // So many problem closing sockets, just trying not to leak anything, not
    // like webservers care about it.
    stream.next_layer().close();

    return convertResponse<T>(vectorBody);
  } catch (std::exception const& e) {
    LOG(ERR) << "Error: " << e.what();
    return std::nullopt;
  }
}

// This is so scuffed lol
template <class T>
std::optional<T> HttpClient::makeGetRequestSsl(const std::string& url,
                                               const std::string& contentType) {
  std::optional<T> ret =
      HttpClient::makeGetRequestSslNoRedirect<T>(url, contentType);
  if (!ret && !HttpClient::redirectUrl.empty()) {
    std::string redirectUrl = HttpClient::redirectUrl;
    HttpClient::redirectUrl.clear();
    ret = HttpClient::makeGetRequestSslNoRedirect<T>(redirectUrl, contentType);
  }
  return ret;
}

// Instantiate the two implementations we will need
template std::optional<std::vector<char>>
HttpClient::makeGetRequestSsl<std::vector<char>>(const std::string&,
                                                 const std::string&);
template std::optional<json> HttpClient::makeGetRequestSsl<json>(
    const std::string&, const std::string&);