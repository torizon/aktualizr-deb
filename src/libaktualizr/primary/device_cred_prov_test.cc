/**
 * \file
 */

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include "httpfake.h"
#include "logging/logging.h"
#include "primary/provisioner.h"
#include "primary/provisioner_test_utils.h"
#include "primary/sotauptaneclient.h"
#include "storage/invstorage.h"
#include "uptane/uptanerepository.h"
#include "utilities/utils.h"

/**
 * Verify that when provisioning with device credentials, aktualizr fails if
 * credentials are not available.
 */
TEST(DeviceCredProv, DeviceIdFailure) {
  RecordProperty("zephyr_key", "OTA-1209,TST-185");
  TemporaryDirectory temp_dir;
  Config config;
  config.storage.path = temp_dir.Path();
  EXPECT_EQ(config.provision.mode, ProvisionMode::kDeviceCred);

  auto storage = INvStorage::newStorage(config.storage);
  auto http = std::make_shared<HttpFake>(temp_dir.Path());
  auto keys = std::make_shared<KeyManager>(storage, config.keymanagerConfig());

  // Expect failure when trying to read the certificate to get the device ID.
  ExpectProvisionError(Provisioner(config.provision, storage, http, keys, {}));
}

/**
 * Verify that when provisioning with device credentials, aktualizr fails if
 * device ID is provided but credentials are not available.
 */
TEST(DeviceCredProv, TlsFailure) {
  RecordProperty("zephyr_key", "OTA-1209,TST-185");
  TemporaryDirectory temp_dir;
  Config config;
  // Set device_id to prevent trying to read it from the certificate.
  config.provision.device_id = "device_id";
  config.storage.path = temp_dir.Path();
  EXPECT_EQ(config.provision.mode, ProvisionMode::kDeviceCred);

  auto storage = INvStorage::newStorage(config.storage);
  auto http = std::make_shared<HttpFake>(temp_dir.Path());
  auto keys = std::make_shared<KeyManager>(storage, config.keymanagerConfig());

  // Expect failure when trying to read the TLS credentials.
  ExpectProvisionError(Provisioner(config.provision, storage, http, keys, {}));
}

/**
 * Verify that aktualizr halts when provided incomplete device provisioning
 * credentials.
 */
TEST(DeviceCredProv, Incomplete) {
  RecordProperty("zephyr_key", "OTA-1209,TST-187");
  TemporaryDirectory temp_dir;
  Config config;
  // Set device_id to prevent trying to read it from the certificate.
  config.provision.device_id = "device_id";
  config.storage.path = temp_dir.Path();
  config.import.base_path = temp_dir / "import";
  EXPECT_EQ(config.provision.mode, ProvisionMode::kDeviceCred);

  auto http = std::make_shared<HttpFake>(temp_dir.Path());

  {
    config.import.tls_cacert_path = utils::BasedPath("ca.pem");
    config.import.tls_clientcert_path = utils::BasedPath("");
    config.import.tls_pkey_path = utils::BasedPath("");
    Utils::createDirectories(temp_dir / "import", S_IRWXU);
    boost::filesystem::copy_file("tests/test_data/device_cred_prov/ca.pem", temp_dir / "import/ca.pem");
    auto storage = INvStorage::newStorage(config.storage);
    storage->importData(config.import);
    auto keys = std::make_shared<KeyManager>(storage, config.keymanagerConfig());

    ExpectProvisionError(Provisioner(config.provision, storage, http, keys, {}));
  }

  {
    config.import.tls_cacert_path = utils::BasedPath("");
    config.import.tls_clientcert_path = utils::BasedPath("client.pem");
    config.import.tls_pkey_path = utils::BasedPath("");
    boost::filesystem::remove_all(temp_dir.Path());
    Utils::createDirectories(temp_dir / "import", S_IRWXU);
    boost::filesystem::copy_file("tests/test_data/device_cred_prov/client.pem", temp_dir / "import/client.pem");
    auto storage = INvStorage::newStorage(config.storage);
    storage->importData(config.import);
    auto keys = std::make_shared<KeyManager>(storage, config.keymanagerConfig());

    ExpectProvisionError(Provisioner(config.provision, storage, http, keys, {}));
  }

  {
    config.import.tls_cacert_path = utils::BasedPath("");
    config.import.tls_clientcert_path = utils::BasedPath("");
    config.import.tls_pkey_path = utils::BasedPath("pkey.pem");
    boost::filesystem::remove_all(temp_dir.Path());
    Utils::createDirectories(temp_dir / "import", S_IRWXU);
    boost::filesystem::copy_file("tests/test_data/device_cred_prov/pkey.pem", temp_dir / "import/pkey.pem");
    auto storage = INvStorage::newStorage(config.storage);
    storage->importData(config.import);
    auto keys = std::make_shared<KeyManager>(storage, config.keymanagerConfig());

    ExpectProvisionError(Provisioner(config.provision, storage, http, keys, {}));
  }

  {
    config.import.tls_cacert_path = utils::BasedPath("ca.pem");
    config.import.tls_clientcert_path = utils::BasedPath("client.pem");
    config.import.tls_pkey_path = utils::BasedPath("");
    boost::filesystem::remove_all(temp_dir.Path());
    Utils::createDirectories(temp_dir / "import", S_IRWXU);
    boost::filesystem::copy_file("tests/test_data/device_cred_prov/ca.pem", temp_dir / "import/ca.pem");
    boost::filesystem::copy_file("tests/test_data/device_cred_prov/client.pem", temp_dir / "import/client.pem");
    auto storage = INvStorage::newStorage(config.storage);
    storage->importData(config.import);
    auto keys = std::make_shared<KeyManager>(storage, config.keymanagerConfig());

    ExpectProvisionError(Provisioner(config.provision, storage, http, keys, {}));
  }

  {
    config.import.tls_cacert_path = utils::BasedPath("ca.pem");
    config.import.tls_clientcert_path = utils::BasedPath("");
    config.import.tls_pkey_path = utils::BasedPath("pkey.pem");
    boost::filesystem::remove_all(temp_dir.Path());
    Utils::createDirectories(temp_dir / "import", S_IRWXU);
    boost::filesystem::copy_file("tests/test_data/device_cred_prov/ca.pem", temp_dir / "import/ca.pem");
    boost::filesystem::copy_file("tests/test_data/device_cred_prov/pkey.pem", temp_dir / "import/pkey.pem");
    auto storage = INvStorage::newStorage(config.storage);
    storage->importData(config.import);
    auto keys = std::make_shared<KeyManager>(storage, config.keymanagerConfig());

    ExpectProvisionError(Provisioner(config.provision, storage, http, keys, {}));
  }

  {
    config.import.tls_cacert_path = utils::BasedPath("");
    config.import.tls_clientcert_path = utils::BasedPath("client.pem");
    config.import.tls_pkey_path = utils::BasedPath("pkey.pem");
    boost::filesystem::remove_all(temp_dir.Path());
    Utils::createDirectories(temp_dir / "import", S_IRWXU);
    boost::filesystem::copy_file("tests/test_data/device_cred_prov/client.pem", temp_dir / "import/client.pem");
    boost::filesystem::copy_file("tests/test_data/device_cred_prov/pkey.pem", temp_dir / "import/pkey.pem");
    auto storage = INvStorage::newStorage(config.storage);
    storage->importData(config.import);
    auto keys = std::make_shared<KeyManager>(storage, config.keymanagerConfig());

    ExpectProvisionError(Provisioner(config.provision, storage, http, keys, {}));
  }

  // Do one last round with all three files to make sure it actually works as
  // expected.
  config.import.tls_cacert_path = utils::BasedPath("ca.pem");
  config.import.tls_clientcert_path = utils::BasedPath("client.pem");
  config.import.tls_pkey_path = utils::BasedPath("pkey.pem");
  boost::filesystem::remove_all(temp_dir.Path());
  Utils::createDirectories(temp_dir / "import", S_IRWXU);
  boost::filesystem::copy_file("tests/test_data/device_cred_prov/ca.pem", temp_dir / "import/ca.pem");
  boost::filesystem::copy_file("tests/test_data/device_cred_prov/client.pem", temp_dir / "import/client.pem");
  boost::filesystem::copy_file("tests/test_data/device_cred_prov/pkey.pem", temp_dir / "import/pkey.pem");
  auto storage = INvStorage::newStorage(config.storage);
  storage->importData(config.import);
  auto keys = std::make_shared<KeyManager>(storage, config.keymanagerConfig());

  ExpectProvisionOK(Provisioner(config.provision, storage, http, keys, {}));
}

/**
 * Verify that aktualizr can provision with provided device credentials.
 */
TEST(DeviceCredProv, Success) {
  RecordProperty("zephyr_key", "OTA-996,OTA-1210,TST-186");
  TemporaryDirectory temp_dir;
  Config config;
  Utils::createDirectories(temp_dir / "import", S_IRWXU);
  boost::filesystem::copy_file("tests/test_data/device_cred_prov/ca.pem", temp_dir / "import/ca.pem");
  boost::filesystem::copy_file("tests/test_data/device_cred_prov/client.pem", temp_dir / "import/client.pem");
  boost::filesystem::copy_file("tests/test_data/device_cred_prov/pkey.pem", temp_dir / "import/pkey.pem");
  config.storage.path = temp_dir.Path();
  config.import.base_path = temp_dir / "import";
  config.import.tls_cacert_path = utils::BasedPath("ca.pem");
  config.import.tls_clientcert_path = utils::BasedPath("client.pem");
  config.import.tls_pkey_path = utils::BasedPath("pkey.pem");
  EXPECT_EQ(config.provision.mode, ProvisionMode::kDeviceCred);

  auto storage = INvStorage::newStorage(config.storage);
  storage->importData(config.import);
  auto http = std::make_shared<HttpFake>(temp_dir.Path());
  auto keys = std::make_shared<KeyManager>(storage, config.keymanagerConfig());

  ExpectProvisionOK(Provisioner(config.provision, storage, http, keys, {}));
}

/**
 * Verify that aktualizr can reimport cert and keeps device name.
 */
TEST(DeviceCredProv, ReImportCert) {
  RecordProperty("zephyr_key", "OLPSUP-12477");
  TemporaryDirectory temp_dir;
  Config config;
  Utils::createDirectories(temp_dir / "import", S_IRWXU);
  boost::filesystem::copy_file("tests/test_data/device_cred_prov/ca.pem", temp_dir / "import/ca.pem");
  boost::filesystem::copy_file("tests/test_data/device_cred_prov/client.pem", temp_dir / "import/client.pem");
  boost::filesystem::copy_file("tests/test_data/device_cred_prov/pkey.pem", temp_dir / "import/pkey.pem");
  /*use any cert with non empty CN*/
  boost::filesystem::copy_file("tests/test_data/prov/client.pem", temp_dir / "import/newcert.pem");
  config.storage.path = temp_dir.Path();
  config.import.base_path = temp_dir / "import";
  config.import.tls_cacert_path = utils::BasedPath("ca.pem");
  config.import.tls_clientcert_path = utils::BasedPath("client.pem");
  config.import.tls_pkey_path = utils::BasedPath("pkey.pem");
  EXPECT_EQ(config.provision.mode, ProvisionMode::kDeviceCred);
  config.provision.device_id = "AnYsTrInG";
  auto http = std::make_shared<HttpFake>(temp_dir.Path());

  {
    /* prepare storage initialized with device_id from config where cert CN and device id are different */
    auto storage = INvStorage::newStorage(config.storage);
    storage->importData(config.import);
    auto keys = std::make_shared<KeyManager>(storage, config.keymanagerConfig());
    ExpectProvisionOK(Provisioner(config.provision, storage, http, keys, {}));
    std::string device_id;
    EXPECT_TRUE(storage->loadDeviceId(&device_id));
    EXPECT_EQ(device_id, "AnYsTrInG");
  }

  {
    config.import.tls_clientcert_path = utils::BasedPath("newcert.pem");
    auto storage = INvStorage::newStorage(config.storage);
    EXPECT_NO_THROW(storage->importData(config.import));
    auto keys = std::make_shared<KeyManager>(storage, config.keymanagerConfig());
    ExpectProvisionOK(Provisioner(config.provision, storage, http, keys, {}));
    std::string device_id;
    EXPECT_TRUE(storage->loadDeviceId(&device_id));
    EXPECT_EQ(device_id, "AnYsTrInG");
  }
}

#ifndef __NO_MAIN__
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  logger_set_threshold(boost::log::trivial::trace);
  return RUN_ALL_TESTS();
}
#endif
