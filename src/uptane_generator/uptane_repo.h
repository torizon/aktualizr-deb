#ifndef UPTANE_REPO_H_
#define UPTANE_REPO_H_

#include "director_repo.h"
#include "image_repo.h"

class UptaneRepo {
 public:
  UptaneRepo(const boost::filesystem::path &path, const std::string &expires, const std::string &correlation_id);
  void generateRepo(KeyType key_type = KeyType::kRSA2048);
  void addTarget(const std::string &target_name, const std::string &hardware_id, const std::string &ecu_serial,
                 const std::string &url = "", const std::string &expires = "");
  void addImage(const boost::filesystem::path &image_path, const boost::filesystem::path &targetname,
                const std::string &hardware_id, const std::string &url = "", int32_t custom_version = 0,
                const Delegation &delegation = {}, const Json::Value &custom = {});
  void addCustomImage(const std::string &name, const Hash &hash, uint64_t length, const std::string &hardware_id,
                      const std::string &url = "", int32_t custom_version = 0, const Delegation &delegation = {},
                      const Json::Value &custom = {});
  void addDelegation(const Uptane::Role &name, const Uptane::Role &parent_role, const std::string &path,
                     bool terminating, KeyType key_type);
  void revokeDelegation(const Uptane::Role &name);
  void signTargets();
  void emptyTargets();
  void oldTargets();
  void generateCampaigns();
  void refresh(Uptane::RepositoryType repo_type, const Uptane::Role &role);
  void rotate(Uptane::RepositoryType repo_type, const Uptane::Role &role, KeyType key_type = KeyType::kRSA2048);

 private:
  DirectorRepo director_repo_;
  ImageRepo image_repo_;
};

#endif  // UPTANE_REPO_H_
