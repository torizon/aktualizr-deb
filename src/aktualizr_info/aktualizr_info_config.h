#ifndef AKTUALIZR_INFO_CONFIG_H_
#define AKTUALIZR_INFO_CONFIG_H_

#include <boost/filesystem/path.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <iosfwd>

#include "libaktualizr/config.h"
#include "utilities/config_utils.h"

// Try to keep the order of config options the same as in
// AktualizrInfoConfig::writeToStream() and
// AktualizrInfoConfig::updateFromPropertyTree().

class AktualizrInfoConfig : public BaseConfig {
 public:
  AktualizrInfoConfig() = default;
  explicit AktualizrInfoConfig(const boost::program_options::variables_map& cmd);
  explicit AktualizrInfoConfig(const boost::filesystem::path& filename);

  void postUpdateValues();
  void writeToStream(std::ostream& sink) const;

  // from Primary config
  LoggerConfig logger;
  BootloaderConfig bootloader;
  PackageConfig pacman;
  StorageConfig storage;
  UptaneConfig uptane;

 private:
  void updateFromCommandLine(const boost::program_options::variables_map& cmd);
  void updateFromPropertyTree(const boost::property_tree::ptree& pt) override;

  bool loglevel_from_cmdline{false};
};
std::ostream& operator<<(std::ostream& os, const AktualizrInfoConfig& cfg);

#endif  // AKTUALIZR_INFO_CONFIG_H_
