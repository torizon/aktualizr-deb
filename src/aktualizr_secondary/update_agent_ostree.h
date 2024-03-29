#ifndef AKTUALIZR_SECONDARY_UPDATE_AGENT_OSTREE_H
#define AKTUALIZR_SECONDARY_UPDATE_AGENT_OSTREE_H

#include "update_agent.h"

class OstreeManager;
class KeyManager;

class OstreeUpdateAgent : public UpdateAgent {
 public:
  OstreeUpdateAgent(boost::filesystem::path sysroot_path, std::shared_ptr<KeyManager>& key_mngr,
                    std::shared_ptr<OstreeManager>& ostree_pack_man, std::string targetname_prefix)
      : sysrootPath_(std::move(sysroot_path)),
        keyMngr_(key_mngr),
        ostreePackMan_(ostree_pack_man),
        targetname_prefix_(std::move(targetname_prefix)) {}

  bool isTargetSupported(const Uptane::Target& target) const override;
  bool getInstalledImageInfo(Uptane::InstalledImageInfo& installed_image_info) const override;

  data::InstallationResult downloadTargetRev(const Uptane::Target& target, const std::string& treehub_tls_creds);

  data::InstallationResult install(const Uptane::Target& target) override;

  void completeInstall() override;

  data::InstallationResult applyPendingInstall(const Uptane::Target& target) override;

 private:
  boost::filesystem::path sysrootPath_;
  std::shared_ptr<KeyManager> keyMngr_;
  std::shared_ptr<OstreeManager> ostreePackMan_;
  const ::std::string targetname_prefix_;
};

#endif  // AKTUALIZR_SECONDARY_UPDATE_AGENT_OSTREE_H
