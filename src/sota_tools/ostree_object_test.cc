#include <gtest/gtest.h>

#include <curl/curl.h>
#include <boost/process.hpp>

#include "authenticate.h"
#include "garage_common.h"
#include "ostree_dir_repo.h"
#include "ostree_object.h"
#include "request_pool.h"
#include "server_credentials.h"
#include "test_utils.h"

std::string port;
std::string repo_path;

/* Verify that constructor does not accept a nonexistent repo. */
TEST(OstreeObject, ConstructorBad) {
  OSTreeDirRepo good_repo(repo_path);
  OSTreeHash hash = good_repo.GetRef("master").GetHash();
  OSTreeDirRepo bad_repo("nonexistentrepo");
  EXPECT_THROW(OSTreeObject(bad_repo, hash, OSTREE_OBJECT_TYPE_COMMIT), std::runtime_error);
}

/* Verify that constructor accepts a valid repo and commit hash. */
TEST(OstreeObject, ConstructorGood) {
  OSTreeDirRepo good_repo(repo_path);
  OSTreeHash hash = good_repo.GetRef("master").GetHash();
  OSTreeObject(good_repo, hash, OSTREE_OBJECT_TYPE_COMMIT);
}

/** Check the << formatting hasn't changed */
TEST(OstreeObject, OStreamFormat) {
  OSTreeDirRepo good_repo(repo_path);
  auto hash = good_repo.GetRef("master").GetHash();
  auto obj = good_repo.GetObject(hash, OstreeObjectType::OSTREE_OBJECT_TYPE_COMMIT);
  std::stringstream ss;
  ss << obj;
  EXPECT_EQ(ss.str(), "b9/ac1e45f9227df8ee191b6e51e09417bd36c6ebbeff999431e3073ac50f0563.commit");
}

/** Check the OstreeObject::GetSize() is sane */
TEST(OstreeObject, GetSize) {
  OSTreeDirRepo good_repo(repo_path);
  auto hash = good_repo.GetRef("master").GetHash();
  auto obj = good_repo.GetObject(hash, OstreeObjectType::OSTREE_OBJECT_TYPE_COMMIT);
  EXPECT_EQ(obj->GetSize(), 118);
}

TEST(OstreeObject, Fsck) {
  OSTreeDirRepo repo("tests/sota_tools/corrupt-repo");
  auto good_object =
      repo.GetObject(OSTreeHash::Parse("2ee758031340b51db1c0229bddd8f64bca4b131728d2bfb20c0c8671b1259a38"),
                     OstreeObjectType::OSTREE_OBJECT_TYPE_FILE);

  EXPECT_TRUE(good_object->Fsck());
  auto corrupt_object =
      repo.GetObject(OSTreeHash::Parse("4145b1a9bade30efb28ff921f7a555ff82ba7d3b7b83b968084436167912fa83"),
                     OstreeObjectType::OSTREE_OBJECT_TYPE_FILE);

  EXPECT_FALSE(corrupt_object->Fsck());
}

// This is a class solely for the purpose of being a FRIEND_TEST to
// OSTreeObject. The name is carefully constructed for this purpose.
class OstreeObject_Request_Test {
 public:
  static void MakeTestRequest(const OSTreeRepo::ptr src_repo, const OSTreeHash& hash, const long expected) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURLM* multi = curl_multi_init();
    curl_multi_setopt(multi, CURLMOPT_PIPELINING, CURLPIPE_HTTP1 | CURLPIPE_MULTIPLEX);

    TreehubServer push_server;
    push_server.root_url("http://localhost:" + port);
    OSTreeObject::ptr object = src_repo->GetObject(hash, OstreeObjectType::OSTREE_OBJECT_TYPE_COMMIT);

    object->MakeTestRequest(push_server, multi);

    // This bit is basically copied from RequestPool::LoopListen().
    int running_requests;
    do {
      CURLMcode mc = curl_multi_perform(multi, &running_requests);
      EXPECT_EQ(mc, CURLM_OK);
    } while (running_requests > 0);

    int msgs_in_queue;
    do {
      CURLMsg* msg = curl_multi_info_read(multi, &msgs_in_queue);
      if ((msg != nullptr) && msg->msg == CURLMSG_DONE) {
        OSTreeObject::ptr h = ostree_object_from_curl(msg->easy_handle);
        EXPECT_EQ(object, h);
        EXPECT_EQ(h->current_operation_, CurrentOp::kOstreeObjectPresenceCheck);

        // This bit is basically copied from OSTreeObject::CurlDone().
        h->refcount_--;
        EXPECT_GE(h->refcount_, 1);
        long rescode = 0;  // NOLINT(google-runtime-int)
        curl_easy_getinfo(h->curl_handle_, CURLINFO_RESPONSE_CODE, &rescode);
        EXPECT_EQ(rescode, expected);
        curl_multi_remove_handle(multi, h->curl_handle_);
        curl_easy_cleanup(h->curl_handle_);
        h->curl_handle_ = nullptr;
      }
    } while (msgs_in_queue > 0);

    curl_multi_cleanup(multi);
    curl_global_cleanup();
  }
};

/* Query destination repository for OSTree commit object.
 * Expect HTTP 200 for a hash that we expect the server to know about. */
TEST(OstreeObject, MakeTestRequestPresent) {
  OSTreeRepo::ptr src_repo = std::make_shared<OSTreeDirRepo>(repo_path);
  OSTreeHash hash = src_repo->GetRef("master").GetHash();
  OstreeObject_Request_Test::MakeTestRequest(src_repo, hash, 200);
}

/* Query destination repository for OSTree commit object.
 * Expect HTTP 404 for a hash that we expect the server not to know about.
 *
 * In this case, the hash is from a different repo. */
TEST(OstreeObject, MakeTestRequestMissing) {
  OSTreeRepo::ptr src_repo = std::make_shared<OSTreeDirRepo>("tests/sota_tools/bigger_repo");
  OSTreeHash hash = src_repo->GetRef("master").GetHash();
  OstreeObject_Request_Test::MakeTestRequest(src_repo, hash, 404);
}

/* Skip upload if dry run was specified. */
TEST(OstreeObject, UploadDryRun) {
  TreehubServer push_server;
  push_server.root_url("http://localhost:" + port);

  OSTreeRepo::ptr src_repo = std::make_shared<OSTreeDirRepo>(repo_path);
  OSTreeHash hash = src_repo->GetRef("master").GetHash();
  OSTreeObject::ptr object = src_repo->GetObject(hash, OstreeObjectType::OSTREE_OBJECT_TYPE_COMMIT);

  object->is_on_server_ = PresenceOnServer::kObjectStateUnknown;
  object->current_operation_ = CurrentOp::kOstreeObjectPresenceCheck;
  object->Upload(push_server, nullptr, RunMode::kDryRun);
  EXPECT_EQ(object->is_on_server_, PresenceOnServer::kObjectPresent);
  // This currently does not get reset.
  EXPECT_EQ(object->current_operation_, CurrentOp::kOstreeObjectPresenceCheck);
}

/* Detect curl misconfiguration.
 *
 * Specifically, verify that a null curl pointer will cause the upload to fail. */
TEST(OstreeObject, UploadFail) {
  TreehubServer push_server;
  push_server.root_url("http://localhost:" + port);

  OSTreeRepo::ptr src_repo = std::make_shared<OSTreeDirRepo>(repo_path);
  OSTreeHash hash = src_repo->GetRef("master").GetHash();
  OSTreeObject::ptr object = src_repo->GetObject(hash, OstreeObjectType::OSTREE_OBJECT_TYPE_COMMIT);

  object->is_on_server_ = PresenceOnServer::kObjectStateUnknown;
  object->current_operation_ = CurrentOp::kOstreeObjectPresenceCheck;
  object->Upload(push_server, nullptr, RunMode::kDefault);
  EXPECT_EQ(object->is_on_server_, PresenceOnServer::kObjectStateUnknown);
  EXPECT_EQ(object->current_operation_, CurrentOp::kOstreeObjectUploading);
}

/* Upload missing OSTree objects to destination repository. */
TEST(OstreeObject, UploadSuccess) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURLM* multi = curl_multi_init();
  curl_multi_setopt(multi, CURLMOPT_PIPELINING, CURLPIPE_HTTP1 | CURLPIPE_MULTIPLEX);

  TemporaryDirectory temp_dir;
  const std::string dp = TestUtils::getFreePort();
  Json::Value auth;
  auth["ostree"]["server"] = std::string("https://localhost:") + dp;
  Utils::writeFile(temp_dir.Path() / "auth.json", auth);
  boost::process::child deploy_server_process("tests/sota_tools/treehub_server.py", std::string("-p"), dp,
                                              std::string("-d"), temp_dir.Path().string(), std::string("--tls"));
  TestUtils::waitForServer("https://localhost:" + dp + "/");

  TreehubServer push_server;
  push_server.root_url("https://localhost:" + dp);

  boost::filesystem::path filepath = (temp_dir.Path() / "auth.json").string();
  boost::filesystem::path cert_path = "tests/fake_http_server/server.crt";
  EXPECT_EQ(authenticate(cert_path.string(), ServerCredentials(filepath), push_server), EXIT_SUCCESS);

  OSTreeRepo::ptr src_repo = std::make_shared<OSTreeDirRepo>(repo_path);
  OSTreeHash hash = src_repo->GetRef("master").GetHash();
  OSTreeObject::ptr object = src_repo->GetObject(hash, OstreeObjectType::OSTREE_OBJECT_TYPE_COMMIT);

  object->Upload(push_server, multi, RunMode::kDefault);

  // This bit is basically copied from RequestPool::LoopListen().
  int running_requests;
  do {
    CURLMcode mc = curl_multi_perform(multi, &running_requests);
    EXPECT_EQ(mc, CURLM_OK);
  } while (running_requests > 0);

  int msgs_in_queue;
  do {
    CURLMsg* msg = curl_multi_info_read(multi, &msgs_in_queue);
    if ((msg != nullptr) && msg->msg == CURLMSG_DONE) {
      OSTreeObject::ptr h = ostree_object_from_curl(msg->easy_handle);
      EXPECT_EQ(object, h);
      EXPECT_EQ(h->current_operation_, CurrentOp::kOstreeObjectUploading);

      // This bit is basically copied from OSTreeObject::CurlDone().
      h->refcount_--;
      EXPECT_GE(h->refcount_, 1);
      long rescode = 0;  // NOLINT(google-runtime-int)
      curl_easy_getinfo(h->curl_handle_, CURLINFO_RESPONSE_CODE, &rescode);
      EXPECT_EQ(rescode, 204);

      curl_multi_remove_handle(multi, h->curl_handle_);
      curl_easy_cleanup(h->curl_handle_);
      h->curl_handle_ = nullptr;
    }
  } while (msgs_in_queue > 0);

  curl_multi_cleanup(multi);
  curl_global_cleanup();
}

#ifndef __NO_MAIN__
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  std::string server = "tests/sota_tools/treehub_server.py";
  port = TestUtils::getFreePort();
  TemporaryDirectory repo_dir;
  repo_path = repo_dir.PathString();

  boost::process::child server_process(server, std::string("-p"), port, std::string("-d"), repo_path,
                                       std::string("--create"));
  TestUtils::waitForServer("http://localhost:" + port + "/");

  return RUN_ALL_TESTS();
}
#endif

// vim: set tabstop=2 shiftwidth=2 expandtab:
