/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "aidl_vendor_manager.h"
#include "aidl_vendor_return_util.h"
#include "misc_vendor_utils.h"
#include "supplicantvendor.h"

#include <android-base/file.h>
#include <fcntl.h>
#include <sys/stat.h>

using aidl::vendor::qti::hardware::wifi::supplicant::AidlVendorManager;
using aidl::vendor::qti::hardware::wifi::supplicant::ISupplicantVendor;
using aidl::vendor::qti::hardware::wifi::supplicant::ISupplicantVendorStaIface;
using aidl::vendor::qti::hardware::wifi::supplicant::SupplicantVendorStatusCode;
using aidl::vendor::qti::hardware::wifi::supplicant::misc_vendor_utils::createStatus;

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace wifi {
namespace supplicant {
using aidl_vendor_return_util::validateAndCall;
using misc_vendor_utils::createStatus;

SupplicantVendor::SupplicantVendor(struct wpa_global* global) : wpa_global_(global) {}
bool SupplicantVendor::isValid()
{
	// This top level object cannot be invalidated.
	return true;
}

::ndk::ScopedAStatus SupplicantVendor::getVendorInterface(
	const IVendorIfaceInfo& vendor_iface_info,
	std::shared_ptr<ISupplicantVendorStaIface>* _aidl_return)
{
	return validateAndCall(
		this, SupplicantVendorStatusCode::FAILURE_IFACE_INVALID,
		&SupplicantVendor::getVendorInterfaceInternal, _aidl_return, vendor_iface_info);
}

::ndk::ScopedAStatus SupplicantVendor::listVendorInterfaces(
	std::vector<IVendorIfaceInfo>* _aidl_return)
{
	return validateAndCall(
		this, SupplicantVendorStatusCode::FAILURE_IFACE_INVALID,
		&SupplicantVendor::listVendorInterfacesInternal, _aidl_return);
}

std::pair<std::shared_ptr<ISupplicantVendorStaIface>, ndk::ScopedAStatus>
SupplicantVendor::getVendorInterfaceInternal(const IVendorIfaceInfo& vendor_iface_info)
{
	struct wpa_supplicant* wpa_s =
		wpa_supplicant_get_iface(wpa_global_, vendor_iface_info.name.c_str());
	if (!wpa_s) {
		return {nullptr, createStatus(SupplicantVendorStatusCode::FAILURE_IFACE_INVALID)};
	}
	AidlVendorManager* aidl_manager = AidlVendorManager::getInstance();
	std::shared_ptr<ISupplicantVendorStaIface> vendor_iface;
	if (!aidl_manager ||
		aidl_manager->getVendorStaIfaceAidlObjectByIfname(
		wpa_s->ifname, &vendor_iface)) {
		return {nullptr, createStatus(SupplicantVendorStatusCode::FAILURE_UNKNOWN)};
	}
	return {vendor_iface, ndk::ScopedAStatus::ok()};
}

std::pair<std::vector<IVendorIfaceInfo>, ndk::ScopedAStatus>
SupplicantVendor::listVendorInterfacesInternal()
{
	std::vector<IVendorIfaceInfo> ifaces;
	for (struct wpa_supplicant* wpa_s = wpa_global_->ifaces; wpa_s;
		 wpa_s = wpa_s->next) {
		if (wpa_s->global->p2p_init_wpa_s == wpa_s)
			continue;
		else {
			ifaces.emplace_back(IVendorIfaceInfo{
				IVendorIfaceType::STA, wpa_s->ifname});
		}
	}
	return {std::move(ifaces), ndk::ScopedAStatus::ok()};
}

}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
}  // namespace aidl
