/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef WPA_SUPPLICANT_VENDOR_AIDL_STA_IFACE_H
#define WPA_SUPPLICANT_VENDOR_AIDL_STA_IFACE_H

#include <array>
#include <vector>

#include <android-base/macros.h>

#include <aidl/vendor/qti/hardware/wifi/supplicant/ISupplicantVendorStaIface.h>
#include <aidl/vendor/qti/hardware/wifi/supplicant/BnSupplicantVendorStaIface.h>
#include <aidl/vendor/qti/hardware/wifi/supplicant/IVendorIfaceInfo.h>
#include <aidl/vendor/qti/hardware/wifi/supplicant/ISupplicantVendor.h>
extern "C"
{
#include "utils/common.h"
#include "utils/includes.h"
#include "wpa_supplicant_i.h"
#include "config.h"
#include "driver_i.h"
#include "wpa.h"
}

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace wifi {
namespace supplicant {

/**
 * Implementation of VendorStaIface aidl object. Each unique aidl
 * object is used for control operations on a specific interface
 * controlled by wpa_supplicant.
 */
class VendorStaIface : public BnSupplicantVendorStaIface
{
public:
	VendorStaIface(struct wpa_global* wpa_global, const char ifname[]);
	~VendorStaIface() override = default;
	void invalidate();
	bool isValid();

	// Vendor Aidl methods exposed.
	::ndk::ScopedAStatus doDriverCmd(
		const std::string& command, std::string* _aidl_return) override;
private:
	// Corresponding worker functions for the Vendor AIDL methods.
	std::pair<std::string, ndk::ScopedAStatus> doDriverCmdInternal(
		const std::string& command);

	// Reference to the global wpa_struct. This is assumed to be valid for
	// the lifetime of the process.
	struct wpa_global* wpa_global_;
	// Name of the iface this aidl object controls
	const std::string ifname_;
	bool is_valid_;
	struct wpa_supplicant* retrieveIfacePtr();
	DISALLOW_COPY_AND_ASSIGN(VendorStaIface);
};

}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
}  // namespace aidl

#endif  // WPA_SUPPLICANT_VENDOR_AIDL_STA_IFACE_H
