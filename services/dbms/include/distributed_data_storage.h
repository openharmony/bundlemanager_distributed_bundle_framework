/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_DATA_STORAGE_H
#define FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_DATA_STORAGE_H

#include <future>
#include <map>

#include "distributed_bundle_info.h"
#include "bundle_constants.h"
#include "bundle_info.h"
#include "distributed_kv_data_manager.h"
#include <future>

namespace OHOS {
namespace AppExecFwk {
namespace {
// distributed database
static const char* APP_ID = "bundle_manager_service";
static const char* DISTRIBUTE_DATA_STORE_ID = "distribute_bundle_datas";
}

class DistributedDataStorageCallback : public OHOS::DistributedKv::KvStoreSyncCallback {
public:
    DistributedDataStorageCallback();
    virtual ~DistributedDataStorageCallback();
    void SyncCompleted(const std::map<std::string, DistributedKv::Status> &result) override;
    void setUuid(const std::string udid);
    DistributedKv::Status GetResultCode();
private:
    std::promise<OHOS::DistributedKv::Status> resultStatusSignal_;
    bool isSetValue_ = false;
    std::mutex setVauleMutex_;
    std::string uuid_;
};

class DistributedDataStorage {
public:
    DistributedDataStorage();
    ~DistributedDataStorage();
    static std::shared_ptr<DistributedDataStorage> GetInstance();

    void SaveStorageDistributeInfo(const std::string &bundleName, int32_t userId);
    void DeleteStorageDistributeInfo(const std::string &bundleName, int32_t userId);
    bool GetStorageDistributeInfo(const std::string &networkId, const std::string &bundleName,
        DistributedBundleInfo &info);
    int32_t GetDistributedBundleName(const std::string &networkId,  uint32_t accessTokenId,
        std::string &bundleName);
    void UpdateDistributedData(int32_t userId);
    static std::string AnonymizeUdid(const std::string& udid);

private:
    std::string DeviceAndNameToKey(const std::string &udid, const std::string &bundleName) const;
    void TryTwice(const std::function<DistributedKv::Status()> &func) const;
    bool CheckKvStore();
    DistributedKv::Status GetKvStore();
    bool GetLocalUdid(std::string &udid);
    DistributedBundleInfo ConvertToDistributedBundleInfo(const BundleInfo &bundleInfo);
    int32_t GetUdidByNetworkId(const std::string &networkId, std::string &udid);
    int32_t GetUuidByNetworkId(const std::string &netWorkId, std::string &uuid);
    bool InnerSaveStorageDistributeInfo(const DistributedBundleInfo &distributedBundleInfo);
    std::map<std::string, DistributedBundleInfo> GetAllOldDistributionBundleInfo(
        const std::vector<std::string> &bundleNames);
    bool SyncAndCompleted(const std::string &udid, const std::string &networkId);
private:
    static std::mutex mutex_;
    static std::shared_ptr<DistributedDataStorage> instance_;

    const DistributedKv::AppId appId_ {APP_ID};
    const DistributedKv::StoreId storeId_ {DISTRIBUTE_DATA_STORE_ID};
    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_DATA_STORAGE_H
