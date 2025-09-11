#pragma once
#include "../configurations/Settings.hpp"
#include "../drivers/DisplayDriver.hpp"

#include <httplib.h>

class WebServer {
 public:
  // Builds a WebServer object that will display configs from 'settings' and
  // apply a selected config using 'driver'.
  // 'settings' and 'driver' can't be nullptr and must outlive the WebServer.
  WebServer(Settings* settings, DisplayDriver* driver);
  ~WebServer();

  WebServer(const WebServer&) = delete;
  WebServer& operator=(const WebServer&) = delete;

  bool error() const { return error_; }

 private:
  Settings& settings_;
  DisplayDriver& driver_;
  std::unique_ptr<httplib::Server> server_;
  std::thread th_;
  std::atomic<bool> error_ = false;
};
