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

#include "distributed_data_storage.h"

#include <set>
#include <unistd.h>

#include "account_manager_helper.h"
#include "app_log_wrapper.h"
#include "distributed_bms.h"
#include "parameter.h"

using namespace OHOS::DistributedKv;

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string BMS_KV_BASE_DIR = "/data/service/el1/public/database/";
const int32_t EL1 = 1;
const int32_t START_INDEX = 0;
const int32_t MINIMUM_WAITING_TIME = 180;   //3 mins
const int32_t MAX_TIMES = 600;              // 1min
const int32_t PRINTF_LENGTH = 8;              // print length of udid
const int32_t SLEEP_INTERVAL = 100 * 1000;  // 100ms
const int32_t FLAGS = BundleFlag::GET_BUNDLE_WITH_ABILITIES |
                      ApplicationFlag::GET_APPLICATION_INFO_WITH_DISABLE |
                      AbilityInfoFlag::GET_ABILITY_INFO_WITH_DISABLE;
const uint32_t DEVICE_UDID_LENGTH = 65;
const std::string EMPTY_DEVICE_ID = "";
}  // namespace

std::shared_ptr<DistributedDataStorage> DistributedDataStorage::instance_ = nullptr;
std::mutex DistributedDataStorage::mutex_;

std::shared_ptr<DistributedDataStorage> DistributedDataStorage::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<DistributedDataStorage>();
        }
    }
    return instance_;
}

DistributedDataStorage::DistributedDataStorage()
{
    APP_LOGI("instance is created");
    TryTwice([this] { return GetKvStore(); });
}

DistributedDataStorage::~DistributedDataStorage()
{
    APP_LOGI("instance is destroyed");
    dataManager_.CloseKvStore(appId_, storeId_);
}

void DistributedDataStorage::SaveStorageDistributeInfo(const std::string &bundleName, int32_t userId)
{
    APP_LOGI("save DistributedBundleInfo data");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return;
        }
    }
    int32_t currentUserId = AccountManagerHelper::GetCurrentActiveUserId();
    if (currentUserId == Constants::INVALID_USERID) {
        currentUserId = Constants::START_USERID;
    }
    if (userId != currentUserId) {
        APP_LOGW("install userid:%{public}d is not currentUserId:%{public}d", userId, currentUserId);
        return;
    }
    auto bundleMgr = DelayedSingleton<DistributedBms>::GetInstance()->GetBundleMgr();
    if (bundleMgr == nullptr) {
        APP_LOGE("Get bundleMgr shared_ptr nullptr");
        return;
    }
    BundleInfo bundleInfo;
    bool ret = bundleMgr->GetBundleInfo(bundleName, FLAGS, bundleInfo, currentUserId);
    if (!ret) {
        APP_LOGW("GetBundleInfo:%{public}s  userid:%{public}d failed", bundleName.c_str(), currentUserId);
        DeleteStorageDistributeInfo(bundleName, currentUserId);
        return;
    }
    ret = InnerSaveStorageDistributeInfo(ConvertToDistributedBundleInfo(bundleInfo));
    if (!ret) {
        APP_LOGW("InnerSaveStorageDistributeInfo:%{public}s  failed", bundleName.c_str());
    }
}

bool DistributedDataStorage::InnerSaveStorageDistributeInfo(const DistributedBundleInfo &distributedBundleInfo)
{
    std::string udid;
    bool ret = GetLocalUdid(udid);
    if (!ret) {
        APP_LOGE("GetLocalUdid error");
        return false;
    }
    std::string keyOfData = DeviceAndNameToKey(udid, distributedBundleInfo.bundleName);
    Key key(keyOfData);
    Value value(distributedBundleInfo.ToString());
    Status status = kvStorePtr_->Put(key, value);
    if (status == Status::IPC_ERROR) {
        status = kvStorePtr_->Put(key, value);
        APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
    }
    if (status != Status::SUCCESS) {
        APP_LOGE("put to kvStore error: %{public}d", status);
        return false;
    }
    APP_LOGI("put value to kvStore success");
    return true;
}

void DistributedDataStorage::DeleteStorageDistributeInfo(const std::string &bundleName, int32_t userId)
{
    APP_LOGI("delete DistributedBundleInfo");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return;
        }
    }
    int32_t currentUserId = AccountManagerHelper::GetCurrentActiveUserId();
    if (userId != currentUserId) {
        APP_LOGW("install userid:%{public}d is not currentUserId:%{public}d", userId, currentUserId);
        return;
    }
    std::string udid;
    bool ret = GetLocalUdid(udid);
    if (!ret) {
        APP_LOGE("GetLocalUdid error");
        return;
    }
    std::string keyOfData = DeviceAndNameToKey(udid, bundleName);
    Key key(keyOfData);
    Status status = kvStorePtr_->Delete(key);
    if (status == Status::IPC_ERROR) {
        status = kvStorePtr_->Delete(key);
        APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
    }
    if (status != Status::SUCCESS) {
        APP_LOGE("delete key error: %{public}d", status);
        return;
    }
    APP_LOGI("delete value to kvStore success");
}

bool DistributedDataStorage::GetStorageDistributeInfo(const std::string &networkId,
    const std::string &bundleName, DistributedBundleInfo &info)
{
    APP_LOGI("get DistributedBundleInfo");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }
    std::string udid;
    int32_t ret = GetUdidByNetworkId(networkId, udid);
    if (ret != 0) {
        APP_LOGI("can not get udid by networkId error:%{public}d", ret);
        return false;
    }
    bool resBool =  SyncAndCompleted(udid, networkId);
    if (!resBool) {
        APP_LOGE("SyncAndCompleted failed");
        return false;
    }
    std::string keyOfData = DeviceAndNameToKey(udid, bundleName);
    APP_LOGI("keyOfData: [%{public}s]", AnonymizeUdid(keyOfData).c_str());
    Key key(keyOfData);
    Value value;
    Status status = kvStorePtr_->Get(key, value);
    if (status == Status::IPC_ERROR) {
        status = kvStorePtr_->Get(key, value);
        APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
    }
    if (status == Status::SUCCESS) {
        if (!info.FromJsonString(value.ToString())) {
            APP_LOGE("it's an error value");
            kvStorePtr_->Delete(key);
            return false;
        }
        return true;
    }
    APP_LOGE("get value status: %{public}d", status);
    return false;
}

int32_t DistributedDataStorage::GetDistributedBundleName(const std::string &networkId, uint32_t accessTokenId,
    std::string &bundleName)
{
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return ERR_BUNDLE_MANAGER_INTERNAL_ERROR;
        }
    }
    std::string udid;
    int32_t ret = GetUdidByNetworkId(networkId, udid);
    if (ret != 0) {
        APP_LOGE("can not get udid by networkId error:%{public}d", ret);
        return ret;
    }
    if (udid.size() == 0) {
        APP_LOGE("get udid is Empty");
        return ERR_BUNDLE_MANAGER_INTERNAL_ERROR;
    }
    bool resBool =  SyncAndCompleted(udid, networkId);
    if (!resBool) {
        APP_LOGE("SyncAndCompleted failed");
        return ERR_BUNDLE_MANAGER_INTERNAL_ERROR;
    }
    Key allEntryKeyPrefix("");
    std::vector<Entry> allEntries;
    Status status = kvStorePtr_->GetEntries(allEntryKeyPrefix, allEntries);
    if (status != Status::SUCCESS) {
        APP_LOGE("dataManager_ GetEntries error: %{public}d", status);
        return ERR_BUNDLE_MANAGER_INTERNAL_ERROR;
    }
    for (auto entry : allEntries) {
        std::string key = entry.key.ToString();
        std::string value =  entry.value.ToString();
        if (key.find(udid) == std::string::npos) {
            continue;
        }
        DistributedBundleInfo distributedBundleInfo;
        if (distributedBundleInfo.FromJsonString(value) && distributedBundleInfo.accessTokenId == accessTokenId) {
            bundleName = distributedBundleInfo.bundleName;
            return OHOS::NO_ERROR;
        }
    }
    APP_LOGE("get distributed bundleName no matching data: %{private}s %{private}s %{private}d",
        networkId.c_str(), udid.c_str(), accessTokenId);
    return ERR_BUNDLE_MANAGER_BUNDLE_NOT_EXIST;
}

std::string DistributedDataStorage::AnonymizeUdid(const std::string& udid)
{
    if (udid.length() < PRINTF_LENGTH) {
        return EMPTY_DEVICE_ID;
    }
    std::string anonUdid = udid.substr(START_INDEX, PRINTF_LENGTH);
    anonUdid.append("******");
    return anonUdid;
}

std::string DistributedDataStorage::DeviceAndNameToKey(
    const std::string &udid, const std::string &bundleName) const
{
    std::string key = udid + Constants::FILE_UNDERLINE + bundleName;
    return key;
}

bool DistributedDataStorage::CheckKvStore()
{
    if (kvStorePtr_ != nullptr) {
        return true;
    }
    int32_t tryTimes = MAX_TIMES;
    while (tryTimes > 0) {
        Status status = GetKvStore();
        if (status == Status::SUCCESS && kvStorePtr_ != nullptr) {
            return true;
        }
        APP_LOGI("CheckKvStore, Times: %{public}d", tryTimes);
        usleep(SLEEP_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

bool DistributedDataStorage::SyncAndCompleted(const std::string &udid, const std::string &networkId)
{
    std::string localUdid;
    bool ret = GetLocalUdid(localUdid);
    if (!ret) {
        APP_LOGE("GetLocalUdid error");
        return false;
    }
    if (udid == localUdid) {
        APP_LOGE("query db of local udid");
        return false;
    }
    std::string uuid;
    int32_t result = GetUuidByNetworkId(networkId, uuid);
    if (result != 0) {
        APP_LOGE("can not get uuid by networkId error:%{public}d", ret);
        return false;
    }
    if (udid.size() == 0) {
        APP_LOGE("get uuid is Empty");
        return false;
    }
    DistributedKv::DataQuery dataQuery;
    dataQuery.KeyPrefix(udid);
    std::vector<std::string> networkIdList = {udid};
    std::shared_ptr<DistributedDataStorageCallback> syncCallback = std::make_shared<DistributedDataStorageCallback>();
    syncCallback->setUuid(uuid);
    Status status = kvStorePtr_->Sync(networkIdList, DistributedKv::SyncMode::PUSH_PULL, dataQuery, syncCallback);
    if (status != Status::SUCCESS) {
        APP_LOGE("distribute database start sync data: %{public}d", status);
        return false;
    }
    APP_LOGI("distribute database start sync data success");
    Status statusResult = syncCallback->GetResultCode();
    if (statusResult != Status::SUCCESS) {
        APP_LOGE("distribute database syncCompleted status: %{public}d", statusResult);
        return false;
    }
    APP_LOGI("distribute database start sync data syncCompleted success");
    return true;
}

Status DistributedDataStorage::GetKvStore()
{
    Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .securityLevel = SecurityLevel::S1,
        .area = EL1,
        .kvStoreType = KvStoreType::SINGLE_VERSION,
        .baseDir = BMS_KV_BASE_DIR + appId_.appId
    };
    Status status = dataManager_.GetSingleKvStore(options, appId_, storeId_, kvStorePtr_);
    if (status != Status::SUCCESS) {
        APP_LOGE("return error: %{public}d", status);
    } else {
        APP_LOGI("get kvStore success");
    }
    return status;
}

void DistributedDataStorage::TryTwice(const std::function<Status()> &func) const
{
    Status status = func();
    if (status == Status::IPC_ERROR) {
        status = func();
        APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
    }
}

bool DistributedDataStorage::GetLocalUdid(std::string &udid)
{
    char innerUdid[DEVICE_UDID_LENGTH] = {0};
    int ret = GetDevUdid(innerUdid, DEVICE_UDID_LENGTH);
    if (ret != 0) {
        APP_LOGI("GetDevUdid failed ,ret:%{public}d", ret);
        return false;
    }
    udid = std::string(innerUdid);
    return true;
}

int32_t DistributedDataStorage::GetUdidByNetworkId(const std::string &networkId, std::string &udid)
{
    auto dbms = DelayedSingleton<DistributedBms>::GetInstance();
    if (dbms == nullptr) {
        APP_LOGE("dbms is null");
        return Constants::INVALID_UDID;
    }
    return dbms->GetUdidByNetworkId(networkId, udid);
}

int32_t DistributedDataStorage::GetUuidByNetworkId(const std::string &networkId, std::string &uuid)
{
    auto dbms = DelayedSingleton<DistributedBms>::GetInstance();
    if (dbms == nullptr) {
        APP_LOGE("dbms is null");
        return Constants::INVALID_UDID;
    }
    return dbms->GetUuidByNetworkId(networkId, uuid);
}

DistributedBundleInfo DistributedDataStorage::ConvertToDistributedBundleInfo(const BundleInfo &bundleInfo)
{
    DistributedBundleInfo distributedBundleInfo;
    distributedBundleInfo.bundleName = bundleInfo.name;
    distributedBundleInfo.versionCode = bundleInfo.versionCode;
    distributedBundleInfo.compatibleVersionCode = bundleInfo.compatibleVersion;
    distributedBundleInfo.versionName = bundleInfo.versionName;
    distributedBundleInfo.minCompatibleVersion = bundleInfo.minCompatibleVersionCode;
    distributedBundleInfo.targetVersionCode = bundleInfo.targetVersion;
    distributedBundleInfo.appId = bundleInfo.appId;
    std::map<std::string, std::vector<DistributedAbilityInfo>> moduleAbilityInfos;
    for (const auto &abilityInfo : bundleInfo.abilityInfos) {
        DistributedAbilityInfo distributedAbilityInfo;
        distributedAbilityInfo.abilityName = abilityInfo.name;
        distributedAbilityInfo.permissions = abilityInfo.permissions;
        distributedAbilityInfo.type = abilityInfo.type;
        distributedAbilityInfo.enabled = abilityInfo.enabled;
        auto infoItem = moduleAbilityInfos.find(abilityInfo.moduleName);
        if (infoItem == moduleAbilityInfos.end()) {
            std::vector<DistributedAbilityInfo> distributedAbilityInfos;
            distributedAbilityInfos.emplace_back(distributedAbilityInfo);
            moduleAbilityInfos.emplace(abilityInfo.moduleName, distributedAbilityInfos);
        } else {
            moduleAbilityInfos[abilityInfo.moduleName].emplace_back(distributedAbilityInfo);
        }
    }
    for (const auto& moduleAbilityInfo : moduleAbilityInfos) {
        DistributedModuleInfo distributedModuleInfo;
        distributedModuleInfo.moduleName = moduleAbilityInfo.first;
        distributedModuleInfo.abilities = moduleAbilityInfo.second;
        distributedBundleInfo.moduleInfos.emplace_back(distributedModuleInfo);
    }
    distributedBundleInfo.enabled = bundleInfo.applicationInfo.enabled;
    distributedBundleInfo.accessTokenId = bundleInfo.applicationInfo.accessTokenId;
    distributedBundleInfo.updateTime = bundleInfo.updateTime;
    return distributedBundleInfo;
}

void DistributedDataStorage::UpdateDistributedData(int32_t userId)
{
    APP_LOGI("UpdateDistributedData");
    auto bundleMgr = DelayedSingleton<DistributedBms>::GetInstance()->GetBundleMgr();
    if (bundleMgr == nullptr) {
        APP_LOGE("Get bundleMgr shared_ptr nullptr");
        return;
    }
    std::vector<BundleInfo> bundleInfos;
    if (!bundleMgr->GetBundleInfos(FLAGS, bundleInfos, userId)) {
        APP_LOGE("get bundleInfos failed");
        return;
    }
    std::vector<std::string> bundleNames;
    for (const auto &bundleInfo : bundleInfos) {
        bundleNames.push_back(bundleInfo.name);
    }
    std::map<std::string, DistributedBundleInfo> oldDistributedBundleInfos =
        GetAllOldDistributionBundleInfo(bundleNames);

    for (const auto &bundleInfo : bundleInfos) {
        if (bundleInfo.singleton) {
            continue;
        }
        if (oldDistributedBundleInfos.find(bundleInfo.name) != oldDistributedBundleInfos.end()) {
            if (oldDistributedBundleInfos[bundleInfo.name].updateTime == bundleInfo.updateTime) {
                APP_LOGD("bundleName:%{public}s no need to update", bundleInfo.name.c_str());
                continue;
            }
        }
        if (!InnerSaveStorageDistributeInfo(ConvertToDistributedBundleInfo(bundleInfo))) {
            APP_LOGW("UpdateDistributedData SaveStorageDistributeInfo:%{public}s failed", bundleInfo.name.c_str());
        }
    }
}

std::map<std::string, DistributedBundleInfo> DistributedDataStorage::GetAllOldDistributionBundleInfo(
    const std::vector<std::string> &bundleNames)
{
    APP_LOGD("start");
    std::map<std::string, DistributedBundleInfo> oldDistributedBundleInfos;
    if (kvStorePtr_ == nullptr) {
        APP_LOGE("kvStorePtr_ is null");
        return oldDistributedBundleInfos;
    }
    std::string udid;
    if (!GetLocalUdid(udid)) {
        APP_LOGE("GetLocalUdid failed");
        return oldDistributedBundleInfos;
    }
    Key allEntryKeyPrefix("");
    std::vector<Entry> allEntries;
    Status status = kvStorePtr_->GetEntries(allEntryKeyPrefix, allEntries);
    if (status != Status::SUCCESS) {
        APP_LOGE("dataManager_ GetEntries error: %{public}d", status);
        return oldDistributedBundleInfos;
    }
    for (const auto &entry : allEntries) {
        std::string key = entry.key.ToString();
        if (key.find(udid) == std::string::npos) {
            continue;
        }
        std::string value = entry.value.ToString();
        DistributedBundleInfo distributedBundleInfo;
        if (distributedBundleInfo.FromJsonString(value)) {
            if (std::find(bundleNames.begin(), bundleNames.end(), distributedBundleInfo.bundleName) ==
                bundleNames.end()) {
                APP_LOGW("bundleName:%{public}s need delete", distributedBundleInfo.bundleName.c_str());
                if (kvStorePtr_->Delete(entry.key) != Status::SUCCESS) {
                    APP_LOGE("Delete key:%{public}s failed", key.c_str());
                }
                continue;
            }
            oldDistributedBundleInfos.emplace(distributedBundleInfo.bundleName, distributedBundleInfo);
        } else {
            APP_LOGE("DistributionInfo FromJsonString key:%{public}s failed", key.c_str());
        }
    }
    return oldDistributedBundleInfos;
}

DistributedDataStorageCallback::DistributedDataStorageCallback()
{
    APP_LOGD("create dbms callback instance");
}

DistributedDataStorageCallback::~DistributedDataStorageCallback()
{
    APP_LOGD("destroy dbms callback instance");
}

void DistributedDataStorageCallback::SyncCompleted(const std::map<std::string, DistributedKv::Status> &result)
{
    APP_LOGD("kvstore sync completed.");
    if (result.find(uuid_) == result.end()) {
        APP_LOGW("SyncCompleted uuid %{public}s no result",
            OHOS::AppExecFwk::DistributedDataStorage::AnonymizeUdid(uuid_).c_str());
        return;
    }
    DistributedKv::Status status = result.at(uuid_);
    APP_LOGI("SyncCompleted uuid %{public}s  result %{public}d",
        OHOS::AppExecFwk::DistributedDataStorage::AnonymizeUdid(uuid_).c_str(), status);
    std::lock_guard<std::mutex> lock(setVauleMutex_);
    if (!isSetValue_) {
        isSetValue_ = true;
        resultStatusSignal_.set_value(status);
    } else {
        APP_LOGW("resultStatusSignal_ is set");
    }
}

void DistributedDataStorageCallback::setUuid(const std::string uuid)
{
    uuid_ = uuid;
}

DistributedKv::Status DistributedDataStorageCallback::GetResultCode()
{
    auto future = resultStatusSignal_.get_future();
    if (future.wait_for(std::chrono::seconds(MINIMUM_WAITING_TIME)) == std::future_status::ready) {
        DistributedKv::Status status = future.get();
        APP_LOGI("GetResultCode status %{public}d", status);
        return status;
    }
    APP_LOGW("GetResultCode time out");
    return Status::TIME_OUT;
}
}  // namespace AppExecFwk
}  // namespace OHOS