#ifndef AKTUALIZR_SECONDARYINSTALLATIONJOB_H
#define AKTUALIZR_SECONDARYINSTALLATIONJOB_H

#include "libaktualizr/results.h"
#include "libaktualizr/secondaryinterface.h"
#include "libaktualizr/types.h"

#include <future>

class SotaUptaneClient;

/**
 * This represents the job of installing firmware on a secondary.
 * It is possible to kick off background tasks to separately send and install
 * the firmware, wait for their completion and fetch the results.
 */
class SecondaryEcuInstallationJob {
 public:
  SecondaryEcuInstallationJob(SotaUptaneClient& uptane_client, SecondaryInterface& secondary,
                              const Uptane::EcuSerial& ecu_serial, const Uptane::Target& target,
                              const std::string& correlation_id, UpdateType update_type);

  /**
   * Start sending the firmware to the secondary
   */
  void SendFirmwareAsync();

  /**
   * Wait for the firmware to finish being sent, and fetch the result
   */
  void WaitForFirmwareSent();

  /**
   * Start installing the firmware on the secondary
   */
  void InstallAsync();

  /**
   * Wait for the installation to complete, and fetch the result
   */
  void WaitForInstall();

  /**
   * Are things OK so far?
   * @return
   */
  bool Ok() const;

  result::Install::EcuReport InstallationReport() const;

  Uptane::EcuSerial ecu_serial() const { return ecu_serial_; }

  Uptane::Target target() const { return target_; }

 private:
  void SendFirmware();
  void Install();
  SotaUptaneClient& uptane_client_;
  SecondaryInterface& secondary_;
  Uptane::Target target_;
  Uptane::EcuSerial ecu_serial_;
  std::string correlation_id_;
  InstallInfo install_info_;
  data::InstallationResult installation_result_{};  // default ctor => success
  std::future<void> firmware_send_;
  std::future<void> install_;
  bool have_installed_{false};
};

#endif  // AKTUALIZR_SECONDARYINSTALLATIONJOB_H
