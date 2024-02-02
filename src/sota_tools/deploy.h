#ifndef SOTA_CLIENT_TOOLS_DEPLOY_H_
#define SOTA_CLIENT_TOOLS_DEPLOY_H_

#include <string>

#include "garage_common.h"
#include "ostree_ref.h"
#include "ostree_repo.h"
#include "server_credentials.h"

/*
 * Check current state of the request pool depending on the run mode.
 * @return true if active, false if finished/inactive
 */
bool CheckPoolState(const OSTreeObject::ptr& root_object, const RequestPool& request_pool);

/**
 * Upload a OSTree repository to Treehub.
 * This will not push the root reference to /refs/heads/qemux86-64 or update
 * the Uptane targets using garage-sign.
 * \param src_repo Maybe either a OSTreeDirRepo (in which case the objects
 *                 are fetched from disk), or OSTreeHttpRepo (in which case
 *                 the objects will be pulled over https).
 * \param push_server
 * \param ostree_commit
 * \param mode
 * \param max_curl_requests
 * \param fsck_on_upload Validate objects on disk before uploading them
 */
bool UploadToTreehub(const OSTreeRepo::ptr& src_repo, TreehubServer& push_server, const OSTreeHash& ostree_commit,
                     RunMode mode, int max_curl_requests, bool fsck_on_upload);

/**
 * Use the garage-sign tool and the Image repo targets.json keys in credentials.zip
 * to add an entry to the Image repo's targets.json
 */
bool OfflineSignRepo(const ServerCredentials& push_credentials, const std::string& name, const OSTreeHash& hash,
                     const std::string& hardwareids);

/**
 * Update the ref on Treehub to the new commit.
 */
bool PushRootRef(const TreehubServer& push_server, const OSTreeRef& ref);

#endif
