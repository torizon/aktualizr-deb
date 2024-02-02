#ifndef PRIMARY_MANAGEDSECONDARY_H_
#define PRIMARY_MANAGEDSECONDARY_H_

#include <future>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>
#include "json/json.h"

#include "libaktualizr/secondaryinterface.h"
#include "libaktualizr/types.h"
#include "primary/secondary_config.h"
#include "uptane/secondary_metadata.h"

namespace Uptane {
class DirectorRepository;
class ImageRepository;
}  // namespace Uptane

class INvStorage;

namespace Primary {

class ManagedSecondaryConfig : public SecondaryConfig {
 public:
  explicit ManagedSecondaryConfig(const std::string& type = "managed") : SecondaryConfig(type) {}

  bool partial_verifying{false};
  std::string ecu_serial;
  std::string ecu_hardware_id;
  boost::filesystem::path full_client_dir;
  std::string ecu_private_key;
  std::string ecu_public_key;
  boost::filesystem::path firmware_path;
  boost::filesystem::path target_name_path;
  boost::filesystem::path metadata_path;
  KeyType key_type{KeyType::kRSA2048};
};

// ManagedSecondary is an abstraction over virtual and other types of legacy
// (non-Uptane) Secondaries. They require any the Uptane-related functionality
// to be implemented in aktualizr itself.
class ManagedSecondary : public SecondaryInterface {
 public:
  explicit ManagedSecondary(Primary::ManagedSecondaryConfig sconfig_in);
  // Prevent inlining to enable forward declarations.
  ~ManagedSecondary() override;
  ManagedSecondary(const ManagedSecondary&) = delete;
  ManagedSecondary& operator=(const ManagedSecondary&) = delete;

  void init(std::shared_ptr<SecondaryProvider> secondary_provider_in) override {
    secondary_provider_ = std::move(secondary_provider_in);
  }

  Uptane::EcuSerial getSerial() const override {
    if (!sconfig.ecu_serial.empty()) {
      return Uptane::EcuSerial(sconfig.ecu_serial);
    }
    return Uptane::EcuSerial(public_key_.KeyId());
  }
  Uptane::HardwareIdentifier getHwId() const override { return Uptane::HardwareIdentifier(sconfig.ecu_hardware_id); }
  PublicKey getPublicKey() const override { return public_key_; }
  data::InstallationResult putMetadata(const Uptane::Target& target) override;
  int getRootVersion(bool director) const override;
  data::InstallationResult putRoot(const std::string& root, bool director) override;

  data::InstallationResult sendFirmware(const Uptane::Target& target, const InstallInfo& install_info,
                                        const api::FlowControlToken* flow_control) override;
  data::InstallationResult install(const Uptane::Target& target, const InstallInfo& info,
                                   const api::FlowControlToken* flow_control) override;

  Uptane::Manifest getManifest() const override;

  // Public for testing only
  bool loadKeys(std::string* pub_key, std::string* priv_key);
  int storeKeysCount() const { return did_store_keys; }

#ifdef BUILD_OFFLINE_UPDATES
  data::InstallationResult putMetadataOffUpd(const Uptane::Target& target,
                                             const Uptane::OfflineUpdateFetcher& fetcher) override;
#endif

 protected:
  ManagedSecondary(ManagedSecondary&&) = default;
  ManagedSecondary& operator=(ManagedSecondary&&) = default;

  virtual bool getFirmwareInfo(Uptane::InstalledImageInfo& firmware_info) const;

  std::shared_ptr<SecondaryProvider> secondary_provider_;
  Primary::ManagedSecondaryConfig sconfig;
  std::string detected_attack;

 private:
  void storeKeys(const std::string& pub_key, const std::string& priv_key);

  int did_store_keys{0};  // For testing
  std::unique_ptr<Uptane::DirectorRepository> director_repo_;
  std::unique_ptr<Uptane::ImageRepository> image_repo_;
  PublicKey public_key_;
  std::string private_key;
  StorageConfig storage_config_;
  std::shared_ptr<INvStorage> storage_;
};

}  // namespace Primary

#endif  // PRIMARY_MANAGEDSECONDARY_H_
