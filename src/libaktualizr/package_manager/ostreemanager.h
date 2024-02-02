#ifndef OSTREE_H_
#define OSTREE_H_

#include <boost/optional/optional.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include <glib/gi18n.h>
#include <ostree.h>

#include "libaktualizr/packagemanagerinterface.h"

#include "crypto/keymanager.h"
#include "utilities/apiqueue.h"

constexpr const char *remote = "aktualizr-remote";

template <typename T>
struct GObjectFinalizer {
  void operator()(T *e) const { g_object_unref(reinterpret_cast<gpointer>(e)); }
};

template <typename T>
using GObjectUniquePtr = std::unique_ptr<T, GObjectFinalizer<T>>;
using OstreeProgressCb = std::function<void(const Uptane::Target &, const std::string &, unsigned int)>;

struct PullMetaStruct {
  PullMetaStruct(Uptane::Target target_in, const api::FlowControlToken *token_in, GCancellable *cancellable_in,
                 OstreeProgressCb progress_cb_in)
      : target{std::move(target_in)},
        percent_complete{0},
        token{token_in},
        cancellable{cancellable_in},
        progress_cb{std::move(progress_cb_in)} {}
  Uptane::Target target;
  unsigned int percent_complete;
  const api::FlowControlToken *token;
  GObjectUniquePtr<GCancellable> cancellable;
  OstreeProgressCb progress_cb;
};

class OstreeManager : public PackageManagerInterface {
 public:
  OstreeManager(const PackageConfig &pconfig, const BootloaderConfig &bconfig,
                const std::shared_ptr<INvStorage> &storage, const std::shared_ptr<HttpInterface> &http,
                Bootloader *bootloader = nullptr);
  ~OstreeManager() override;
  OstreeManager(const OstreeManager &) = delete;
  OstreeManager(OstreeManager &&) = delete;
  OstreeManager &operator=(const OstreeManager &) = delete;
  OstreeManager &operator=(OstreeManager &&) = delete;
  std::string name() const override { return "ostree"; }
  Json::Value getInstalledPackages() const override;
  virtual std::string getCurrentHash() const;
  Uptane::Target getCurrent() const override;
  bool imageUpdated();
  data::InstallationResult install(const Uptane::Target &target) const override;
  void completeInstall() const override;
  data::InstallationResult finalizeInstall(const Uptane::Target &target) override;
  void updateNotify() override;
  bool fetchTarget(const Uptane::Target &target, Uptane::Fetcher &fetcher, const KeyManager &keys,
                   const FetcherProgressCb &progress_cb, const api::FlowControlToken *token) override;
#ifdef BUILD_OFFLINE_UPDATES
  bool fetchTargetOffUpd(const Uptane::Target &target, const Uptane::OfflineUpdateFetcher &fetcher,
                         const KeyManager &keys, const FetcherProgressCb &progress_cb,
                         const api::FlowControlToken *token) override;
#endif
  TargetStatus verifyTarget(const Uptane::Target &target) const override;

  GObjectUniquePtr<OstreeDeployment> getStagedDeployment() const;
  static GObjectUniquePtr<OstreeSysroot> LoadSysroot(const boost::filesystem::path &path);
  static GObjectUniquePtr<OstreeRepo> LoadRepo(OstreeSysroot *sysroot, GError **error);
  static bool addRemote(OstreeRepo *repo, const std::string &url, const KeyManager *keys = nullptr);
  static data::InstallationResult pull(
      const boost::filesystem::path &sysroot_path, const std::string &ostree_server, const KeyManager &keys,
      const Uptane::Target &target, const api::FlowControlToken *token = nullptr,
      OstreeProgressCb progress_cb = nullptr, const char *alt_remote = nullptr,
      boost::optional<std::unordered_map<std::string, std::string>> headers = boost::none);

#ifdef BUILD_OFFLINE_UPDATES
  static data::InstallationResult pullLocal(const boost::filesystem::path &sysroot_path,
                                            const boost::filesystem::path &srcrepo_path, const Uptane::Target &target,
                                            OstreeProgressCb progress_cb = nullptr);
#endif

 private:
  TargetStatus verifyTargetInternal(const Uptane::Target &target) const;

  std::unique_ptr<Bootloader> bootloader_;
};

#endif  // OSTREE_H_
