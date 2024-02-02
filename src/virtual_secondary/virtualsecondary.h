#ifndef PRIMARY_VIRTUALSECONDARY_H_
#define PRIMARY_VIRTUALSECONDARY_H_

#include <string>

#include "libaktualizr/types.h"
#include "managedsecondary.h"

namespace Primary {

class VirtualSecondaryConfig : public ManagedSecondaryConfig {
 public:
  static constexpr const char* const Type{"virtual"};

  VirtualSecondaryConfig() : ManagedSecondaryConfig(Type) {}
  explicit VirtualSecondaryConfig(const Json::Value& json_config);

  static std::vector<VirtualSecondaryConfig> create_from_file(const boost::filesystem::path& file_full_path);
  void dump(const boost::filesystem::path& file_full_path) const;
};

class VirtualSecondary : public ManagedSecondary {
 public:
  explicit VirtualSecondary(Primary::VirtualSecondaryConfig sconfig_in);

  std::string Type() const override { return VirtualSecondaryConfig::Type; }
  data::InstallationResult putMetadata(const Uptane::Target& target) override;
  data::InstallationResult putRoot(const std::string& root, bool director) override;
  data::InstallationResult sendFirmware(const Uptane::Target& target, const InstallInfo& install_info,
                                        const api::FlowControlToken* flow_control) override;
  data::InstallationResult install(const Uptane::Target& target, const InstallInfo& info,
                                   const api::FlowControlToken* flow_control) override;

  bool ping() const override { return true; }
};

}  // namespace Primary

#endif  // PRIMARY_VIRTUALSECONDARY_H_
