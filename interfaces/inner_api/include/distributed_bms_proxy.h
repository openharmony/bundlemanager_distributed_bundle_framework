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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_BMS_PROXY_H
#define FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_BMS_PROXY_H

#include <string>

#include "distributed_bms_interface.h"
#include "distributed_bundle_ipc_interface_code.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
class DistributedBmsProxy : public IRemoteProxy<IDistributedBms> {
public:
    explicit DistributedBmsProxy(const sptr<IRemoteObject> &object);
    virtual ~DistributedBmsProxy() override;

    /**
     * @brief get remote ability info
     * @param elementName Indicates the elementName.
     * @param remoteAbilityInfo Indicates the remote ability info.
     * @return Returns result code when get ability info.
     */
    int32_t GetRemoteAbilityInfo(
        const OHOS::AppExecFwk::ElementName &elementName, RemoteAbilityInfo &remoteAbilityInfo) override;

    /**
     * @brief get remote ability info
     * @param elementName Indicates the elementName.
     * @param localeInfo Indicates the localeInfo.
     * @param remoteAbilityInfo Indicates the remote ability info.
     * @return Returns result code when get remote ability info.
     */
    int32_t GetRemoteAbilityInfo(const OHOS::AppExecFwk::ElementName &elementName, const std::string &localeInfo,
        RemoteAbilityInfo &remoteAbilityInfo) override;

    /**
     * @brief get remote ability infos
     * @param elementNames Indicates the elementNames.
     * @param remoteAbilityInfos Indicates the remote ability infos.
     * @return Returns result code when get remote ability infos.
     */
    int32_t GetRemoteAbilityInfos(
        const std::vector<ElementName> &elementNames, std::vector<RemoteAbilityInfo> &remoteAbilityInfos) override;

    /**
     * @brief get remote ability infos
     * @param elementNames Indicates the elementNames.
     * @param localeInfo Indicates the localeInfo.
     * @param remoteAbilityInfos Indicates the remote ability infos.
     * @return Returns result code when get remote ability infos.
     */
    int32_t GetRemoteAbilityInfos(const std::vector<ElementName> &elementNames, const std::string &localeInfo,
        std::vector<RemoteAbilityInfo> &remoteAbilityInfos) override;

    /**
     * @brief get ability info
     * @param elementName Indicates the elementName.
     * @param remoteAbilityInfo Indicates the remote ability info.
     * @return Returns result code when get ability info.
     */
    int32_t GetAbilityInfo(
        const OHOS::AppExecFwk::ElementName &elementName, RemoteAbilityInfo &remoteAbilityInfo) override;

    /**
     * @brief get ability info
     * @param elementName Indicates the elementName.
     * @param localeInfo Indicates the localeInfo.
     * @param remoteAbilityInfo Indicates the remote ability info.
     * @param info Indicates the acl info.
     * @return Returns result code when get ability info.
     */
    int32_t GetAbilityInfo(const OHOS::AppExecFwk::ElementName &elementName, const std::string &localeInfo,
        RemoteAbilityInfo &remoteAbilityInfo, DistributedBmsAclInfo *info = nullptr) override;

    /**
     * @brief get ability infos
     * @param elementNames Indicates the elementNames.
     * @param remoteAbilityInfos Indicates the remote ability infos.
     * @return Returns result code when get ability infos.
     */
    int32_t GetAbilityInfos(
        const std::vector<ElementName> &elementNames, std::vector<RemoteAbilityInfo> &remoteAbilityInfos) override;

    /**
     * @brief get ability infos
     * @param elementNames Indicates the elementNames.
     * @param localeInfo Indicates the localeInfo.
     * @param remoteAbilityInfos Indicates the remote ability infos.
     * @param info Indicates the acl info.
     * @return Returns result code when get ability infos.
     */
    int32_t GetAbilityInfos(const std::vector<ElementName> &elementNames, const std::string &localeInfo,
        std::vector<RemoteAbilityInfo> &remoteAbilityInfos, DistributedBmsAclInfo *info = nullptr) override;
    
    bool GetDistributedBundleInfo(const std::string &networkId, const std::string &bundleName,
        DistributedBundleInfo &distributedBundleInfo) override;

    /**
     * @brief get distributedBundleName based on a given accessTokenId and networkId.
     * @param networkId Indicates the networkId of remote device.
     * @param accessTokenId AccessTokenId of the application
     * @param bundleNames distributed bundle name.
     * @return Returns ERR_OK on success, others on failure when get distributed bundle name.
     */
    int32_t GetDistributedBundleName(const std::string &networkId, uint32_t accessTokenId,
        std::string &bundleName) override;
private:
    int32_t SendRequest(DistributedInterfaceCode code, MessageParcel &data, MessageParcel &reply);
    template<typename T>
    bool WriteParcelableVector(const std::vector<T> &parcelableVector, Parcel &data);
    template <typename T>
    int32_t GetParcelableInfo(DistributedInterfaceCode code, MessageParcel &data, T &parcelableInfo);
    template <typename T>
    int32_t GetParcelableInfos(DistributedInterfaceCode code, MessageParcel &data, std::vector<T> &parcelableInfos);
    int32_t CheckElementName(const ElementName &elementName);
    static inline BrokerDelegator<DistributedBmsProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_BMS_PROXY_H