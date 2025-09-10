#pragma once
#include <memory>
#include <optional>

#include "../configurations/GlobalConfiguration.hpp"

class DisplayDriver {
 protected:
  enum DriverState { OK, FAIL };
  DriverState state = FAIL;
  virtual GlobalConfiguration _getConfig() = 0;
  virtual bool _applyConfig(const GlobalConfiguration& conf) = 0;

 public:
  std::optional<GlobalConfiguration> getConfig();
  bool applyConfig(const GlobalConfiguration& conf);
  bool isOK();
  // static method to get instance
  static std::unique_ptr<DisplayDriver> getAvailableDriver();
};