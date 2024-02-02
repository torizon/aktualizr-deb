#ifndef RESULTS_H_
#define RESULTS_H_
/** \file */

#include <string>
#include <vector>

#include "libaktualizr/campaign.h"
#include "libaktualizr/types.h"

/** Results of libaktualizr API calls. */
namespace result {
/**
 * Container for information about available campaigns.
 */
class CampaignCheck {
 public:
  explicit CampaignCheck(std::vector<campaign::Campaign> campaigns_in) : campaigns(std::move(campaigns_in)) {}
  std::vector<campaign::Campaign> campaigns;
};

/**
 * Status of an update.
 */
enum class UpdateStatus {
  /* Updates are available for ECUs known to aktualizr. */
  kUpdatesAvailable = 0,
  /* No updates are available for ECUs known to aktualizr. */
  kNoUpdatesAvailable,
  /* There was an error checking for updates. */
  kError,
};

std::ostream& operator<<(std::ostream& os, UpdateStatus update_status);

/**
 * Container for information about available updates.
 */
class UpdateCheck {
 public:
  UpdateCheck() = default;
  UpdateCheck(std::vector<Uptane::Target> updates_in, unsigned int ecus_count_in, UpdateStatus status_in,
              std::string message_in)
      : updates(std::move(updates_in)), ecus_count(ecus_count_in), status(status_in), message(std::move(message_in)) {}
  std::vector<Uptane::Target> updates;
  unsigned int ecus_count{0};
  UpdateStatus status{UpdateStatus::kNoUpdatesAvailable};
  std::string message;
};

/**
 * Result of an attempt to pause or resume a download.
 */
enum class PauseStatus {
  /* The system was successfully paused or resumed */
  kSuccess = 0,
  /* The system was already paused */
  kAlreadyPaused,
  /* The system was already running */
  kAlreadyRunning,
  /* General error */
  kError,
};

class Pause {
 public:
  Pause() = default;
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  Pause(PauseStatus status_in) : status(status_in) {}

  PauseStatus status{PauseStatus::kSuccess};
};

/**
 * Status of an update download.
 */
enum class DownloadStatus {
  /* Update was downloaded successfully. */
  kSuccess = 0,
  /* Some targets in the update were downloaded successfully, but some were not. */
  kPartialSuccess,
  /* There are no targets to download for this update. */
  kNothingToDownload,
  /* There was an error downloading targets. */
  kError,
};

std::ostream& operator<<(std::ostream& os, DownloadStatus stat);

/**
 * Container for information about downloading an update.
 */
class Download {
 public:
  Download() = default;
  Download(std::vector<Uptane::Target> updates_in, DownloadStatus status_in, std::string message_in)
      : updates(std::move(updates_in)), status(status_in), message(std::move(message_in)) {}
  std::vector<Uptane::Target> updates;
  DownloadStatus status{DownloadStatus::kNothingToDownload};
  std::string message;
};

/**
 * Container for information about installing an update.
 */
class Install {
 public:
  Install() = default;
  class EcuReport;
  Install(data::InstallationResult dev_report_in, std::vector<EcuReport> ecu_reports_in, std::string raw_report_in = "")
      : dev_report(std::move(dev_report_in)),
        ecu_reports(std::move(ecu_reports_in)),
        raw_report(std::move(raw_report_in)) {}

  data::InstallationResult dev_report{false, data::ResultCode::Numeric::kUnknown, ""};
  std::vector<EcuReport> ecu_reports;
  std::string raw_report;

  class EcuReport {
   public:
    EcuReport(Uptane::Target update_in, Uptane::EcuSerial serial_in, data::InstallationResult install_res_in)
        : update(std::move(update_in)), serial(std::move(serial_in)), install_res(std::move(install_res_in)) {}

    Uptane::Target update;
    Uptane::EcuSerial serial;
    data::InstallationResult install_res;
  };
};

}  // namespace result

#endif  // RESULTS_H_
