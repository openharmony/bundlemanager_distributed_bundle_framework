/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "distributed_bms_host.h"

#include "accesstoken_kit.h"
#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "bundle_constants.h"
#include "bundle_memory_guard.h"
#include "remote_ability_info.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t GET_REMOTE_ABILITY_INFO_MAX_SIZE = 10;
}

DistributedBmsHost::DistributedBmsHost()
{
    APP_LOGI("DistributedBmsHost instance is created");
}

DistributedBmsHost::~DistributedBmsHost()
{
    APP_LOGI("DistributedBmsHost instance is destroyed");
}

int DistributedBmsHost::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    BundleMemoryGuard memoryGuard;
    APP_LOGI("DistributedBmsHost receives message from client, code = %{public}u, flags = %{public}d", code,
        option.GetFlags());
    std::u16string descripter = DistributedBmsHost::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        APP_LOGE("verify interface token failed");
        return ERR_INVALID_STATE;
    }
    switch (code) {
        case static_cast<uint32_t>(IDistributedBms::Message::GET_REMOTE_ABILITY_INFO):
        case static_cast<uint32_t>(IDistributedBms::Message::GET_REMOTE_ABILITY_INFO_WITH_LOCALE):
            return HandleGetRemoteAbilityInfo(data, reply);
        case static_cast<uint32_t>(IDistributedBms::Message::GET_REMOTE_ABILITY_INFOS):
        case static_cast<uint32_t>(IDistributedBms::Message::GET_REMOTE_ABILITY_INFOS_WITH_LOCALE):
            return HandleGetRemoteAbilityInfos(data, reply);
        case static_cast<uint32_t>(IDistributedBms::Message::GET_ABILITY_INFO):
        case static_cast<uint32_t>(IDistributedBms::Message::GET_ABILITY_INFO_WITH_LOCALE):
            return HandleGetAbilityInfo(data, reply);
        case static_cast<uint32_t>(IDistributedBms::Message::GET_ABILITY_INFOS):
        case static_cast<uint32_t>(IDistributedBms::Message::GET_ABILITY_INFOS_WITH_LOCALE):
            return HandleGetAbilityInfos(data, reply);
        case static_cast<uint32_t>(IDistributedBms::Message::GET_DISTRIBUTED_BUNDLE_INFO):
            return HandleGetDistributedBundleInfo(data, reply);
        case static_cast<uint32_t>(IDistributedBms::Message::GET_DISTRIBUTED_BUNDLE_NAME):
            return HandleGetDistributedBundleName(data, reply);
        default:
            APP_LOGW("DistributedBmsHost receives unknown code, code = %{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return NO_ERROR;
}

int DistributedBmsHost::HandleGetRemoteAbilityInfo(Parcel &data, Parcel &reply)
{
    APP_LOGI("DistributedBmsHost handle get remote ability info");
    if (!VerifySystemApp()) {
        APP_LOGE("verify system app failed");
        return ERR_BUNDLE_MANAGER_SYSTEM_API_DENIED;
    }
    if (!VerifyCallingPermission(Constants::PERMISSION_GET_BUNDLE_INFO_PRIVILEGED)) {
        APP_LOGE("verify GET_BUNDLE_INFO_PRIVILEGED failed");
        return ERR_BUNDLE_MANAGER_PERMISSION_DENIED;
    }
    std::unique_ptr<ElementName> elementName(data.ReadParcelable<ElementName>());
    if (!elementName) {
        APP_LOGE("ReadParcelable<elementName> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    std::string localeInfo = data.ReadString();
    RemoteAbilityInfo remoteAbilityInfo;
    int ret = GetRemoteAbilityInfo(*elementName, localeInfo, remoteAbilityInfo);
    if (ret != NO_ERROR) {
        APP_LOGE("GetRemoteAbilityInfo result:%{public}d", ret);
        return ret;
    }
    if (!reply.WriteBool(true)) {
        APP_LOGE("GetRemoteAbilityInfo write failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!reply.WriteParcelable(&remoteAbilityInfo)) {
        APP_LOGE("GetRemoteAbilityInfo write failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return NO_ERROR;
}

int DistributedBmsHost::HandleGetRemoteAbilityInfos(Parcel &data, Parcel &reply)
{
    APP_LOGI("DistributedBmsHost handle get remote ability infos");
    if (!VerifySystemApp()) {
        APP_LOGE("verify system app failed");
        return ERR_BUNDLE_MANAGER_SYSTEM_API_DENIED;
    }
    if (!VerifyCallingPermission(Constants::PERMISSION_GET_BUNDLE_INFO_PRIVILEGED)) {
        APP_LOGE("verify GET_BUNDLE_INFO_PRIVILEGED failed");
        return ERR_BUNDLE_MANAGER_PERMISSION_DENIED;
    }
    std::vector<ElementName> elementNames;
    if (!GetParcelableInfos<ElementName>(data, elementNames)) {
        APP_LOGE("GetRemoteAbilityInfos get parcelable infos failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    std::string localeInfo = data.ReadString();
    std::vector<RemoteAbilityInfo> remoteAbilityInfos;
    int ret = GetRemoteAbilityInfos(elementNames, localeInfo, remoteAbilityInfos);
    if (ret != NO_ERROR) {
        APP_LOGE("GetRemoteAbilityInfos result:%{public}d", ret);
        return ret;
    }
    if (!reply.WriteBool(true)) {
        APP_LOGE("GetRemoteAbilityInfos write failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!WriteParcelableVector<RemoteAbilityInfo>(remoteAbilityInfos, reply)) {
        APP_LOGE("GetRemoteAbilityInfos write failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return NO_ERROR;
}


int DistributedBmsHost::HandleGetAbilityInfo(Parcel &data, Parcel &reply)
{
    APP_LOGI("DistributedBmsHost handle get ability info");
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    if (!VerifySystemAppForTokenNative(callerToken) && !VerifySystemAppForTokenShell(callerToken)) {
        APP_LOGE("caller is not native");
        return ERR_BUNDLE_MANAGER_SYSTEM_API_DENIED;
    }
    std::unique_ptr<ElementName> elementName(data.ReadParcelable<ElementName>());
    if (!elementName) {
        APP_LOGE("ReadParcelable<elementName> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    std::string localeInfo = data.ReadString();
    RemoteAbilityInfo remoteAbilityInfo;
    int ret = GetAbilityInfo(*elementName, localeInfo, remoteAbilityInfo);
    if (ret != NO_ERROR) {
        APP_LOGE("GetAbilityInfo result:%{public}d", ret);
        return ret;
    }
    if (!reply.WriteBool(true)) {
        APP_LOGE("GetRemoteAbilityInfo write failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!reply.WriteParcelable(&remoteAbilityInfo)) {
        APP_LOGE("GetRemoteAbilityInfo write failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return NO_ERROR;
}

int DistributedBmsHost::HandleGetAbilityInfos(Parcel &data, Parcel &reply)
{
    APP_LOGI("DistributedBmsHost handle get ability infos");
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    if (!VerifySystemAppForTokenNative(callerToken) && !VerifySystemAppForTokenShell(callerToken)) {
        APP_LOGE("caller is not native");
        return ERR_BUNDLE_MANAGER_SYSTEM_API_DENIED;
    }
    std::vector<ElementName> elementNames;
    if (!GetParcelableInfos<ElementName>(data, elementNames)) {
        APP_LOGE("GetRemoteAbilityInfos get parcelable infos failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    std::string localeInfo = data.ReadString();
    std::vector<RemoteAbilityInfo> remoteAbilityInfos;
    int ret = GetAbilityInfos(elementNames, localeInfo, remoteAbilityInfos);
    if (ret != NO_ERROR) {
        APP_LOGE("GetAbilityInfos result:%{public}d", ret);
        return ret;
    }
    if (!reply.WriteBool(true)) {
        APP_LOGE("GetAbilityInfos write failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!WriteParcelableVector<RemoteAbilityInfo>(remoteAbilityInfos, reply)) {
        APP_LOGE("GetAbilityInfos write failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return NO_ERROR;
}

int DistributedBmsHost::HandleGetDistributedBundleInfo(Parcel &data, Parcel &reply)
{
    APP_LOGI("DistributedBmsHost handle get distributedBundleInfo");
    std::string networkId = data.ReadString();
    std::string bundleName = data.ReadString();
    DistributedBundleInfo distributedBundleInfo;
    bool ret = GetDistributedBundleInfo(networkId, bundleName, distributedBundleInfo);
    if (!ret) {
        APP_LOGE("GetDistributedBundleInfo failed");
        return INVALID_OPERATION;
    }
    if (!reply.WriteBool(true)) {
        APP_LOGE("GetDistributedBundleInfo write failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!reply.WriteParcelable(&distributedBundleInfo)) {
        APP_LOGE("GetDistributedBundleInfo write failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return NO_ERROR;
}

int32_t DistributedBmsHost::HandleGetDistributedBundleName(Parcel &data, Parcel &reply)
{
    APP_LOGD("DistributedBmsHost handle get distributedBundleName");
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    if (!VerifySystemAppForTokenNative(callerToken) && !VerifySystemAppForTokenShell(callerToken)) {
        APP_LOGE("caller tokenType not native or shell, verify failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    std::string networkId = data.ReadString();
    uint32_t accessTokenId = data.ReadUint32();
    std::string bundleName;
    int32_t ret = GetDistributedBundleName(networkId, accessTokenId, bundleName);
    if (ret == NO_ERROR && !reply.WriteString(bundleName)) {
        APP_LOGE("write failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return ret;
}

bool DistributedBmsHost::VerifyCallingPermission(const std::string &permissionName)
{
    APP_LOGD("VerifyCallingPermission permission %{public}s", permissionName.c_str());
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    if (VerifySystemAppForTokenNative(callerToken)) {
        APP_LOGD("caller tokenType is native, verify success");
        return true;
    }
    int32_t ret = OHOS::Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (ret == OHOS::Security::AccessToken::PermissionState::PERMISSION_DENIED) {
        APP_LOGE("permission %{public}s: PERMISSION_DENIED", permissionName.c_str());
        return false;
    }
    APP_LOGD("verify permission success");
    return true;
}

bool DistributedBmsHost::VerifySystemApp()
{
    APP_LOGI("verifying systemApp");
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    if (VerifySystemAppForTokenNative(callerToken) || VerifySystemAppForTokenShell(callerToken)
        || callingUid == Constants::ROOT_UID) {
        APP_LOGI("caller tokenType is native or shell, verify success");
        return true;
    }
    uint64_t accessTokenIdEx = IPCSkeleton::GetCallingFullTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(accessTokenIdEx)) {
        APP_LOGE("non-system app calling system api");
        return false;
    }
    return true;
}

bool DistributedBmsHost::VerifySystemAppForTokenNative(Security::AccessToken::AccessTokenID callerToken)
{
    APP_LOGD("verifying system app for native token");
    Security::AccessToken::ATokenTypeEnum tokenType =
        Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    APP_LOGD("token type is %{public}d", static_cast<int32_t>(tokenType));
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        APP_LOGI("caller tokenType is native, verify success");
        return true;
    }
    APP_LOGE("caller tokenType not native, verify failed");
    return false;
}

bool DistributedBmsHost::VerifySystemAppForTokenShell(Security::AccessToken::AccessTokenID callerToken)
{
    APP_LOGD("verifying system app for shell token");
    Security::AccessToken::ATokenTypeEnum tokenType =
        Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    APP_LOGD("token type is %{public}d", static_cast<int32_t>(tokenType));
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL) {
        APP_LOGI("caller tokenType is shell, verify success");
        return true;
    }
    APP_LOGE("caller tokenType not shell, verify failed");
    return false;
}

template<typename T>
bool DistributedBmsHost::WriteParcelableVector(std::vector<T> &parcelableVector, Parcel &reply)
{
    if (!reply.WriteInt32(parcelableVector.size())) {
        APP_LOGE("write ParcelableVector failed");
        return false;
    }

    for (auto &parcelable : parcelableVector) {
        if (!reply.WriteParcelable(&parcelable)) {
            APP_LOGE("write ParcelableVector failed");
            return false;
        }
    }
    return true;
}

template<typename T>
bool DistributedBmsHost::GetParcelableInfos(Parcel &data, std::vector<T> &parcelableInfos)
{
    int32_t infoSize = data.ReadInt32();
    if (infoSize > GET_REMOTE_ABILITY_INFO_MAX_SIZE) {
        APP_LOGE("GetParcelableInfos elements num exceeds the limit %{public}d", GET_REMOTE_ABILITY_INFO_MAX_SIZE);
        return false;
    }
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(data.ReadParcelable<T>());
        if (!info) {
            APP_LOGE("Read Parcelable infos failed");
            return false;
        }
        parcelableInfos.emplace_back(*info);
    }
    APP_LOGD("get parcelable infos success");
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS