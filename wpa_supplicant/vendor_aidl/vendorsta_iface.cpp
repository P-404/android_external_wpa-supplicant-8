/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "aidl_vendor_manager.h"
#include "aidl_vendor_return_util.h"
#include "misc_vendor_utils.h"
#include "vendorsta_iface.h"

extern "C"
{
#include "utils/eloop.h"
#include "wps_supplicant.h"
}

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

VendorStaIface::VendorStaIface(struct wpa_global *wpa_global, const char ifname[])
	: wpa_global_(wpa_global), ifname_(ifname), is_valid_(true)
{}

void VendorStaIface::invalidate() { is_valid_ = false; }
bool VendorStaIface::isValid()
{
	return (is_valid_ && (retrieveIfacePtr() != nullptr));
}

std::pair<std::string, ndk::ScopedAStatus> VendorStaIface::doDriverCmdInternal(
	const std::string& command)
{
	const char * cmd = command.c_str();
	std::vector<char> cmd_vec(cmd, cmd + strlen(cmd) + 1);
	struct wpa_supplicant *wpa_s = retrieveIfacePtr();
	char driver_cmd_reply_buf[4096] = {};
	if (wpa_drv_driver_cmd(
		wpa_s, cmd_vec.data(), driver_cmd_reply_buf,
		sizeof(driver_cmd_reply_buf)) < 0) {
		return {"", createStatus(SupplicantVendorStatusCode::FAILURE_UNKNOWN)};
	}
	return {driver_cmd_reply_buf, ndk::ScopedAStatus::ok()};
}

ndk::ScopedAStatus VendorStaIface::doDriverCmd(
	const std::string& command, std::string* _aidl_return)
{
	return validateAndCall(
		this, SupplicantVendorStatusCode::FAILURE_IFACE_INVALID,
		&VendorStaIface::doDriverCmdInternal, _aidl_return, command);
}

/**
 * Retrieve the underlying |wpa_supplicant| struct
 * pointer for this iface.
 * If the underlying iface is removed, then all RPC method calls on this object
 * will return failure.
 */
wpa_supplicant *VendorStaIface::retrieveIfacePtr()
{
	return wpa_supplicant_get_iface(wpa_global_, ifname_.c_str());
}
}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
}  // namespace aidl
