/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef WPA_SUPPLICANT_VENDOR_AIDL_SUPPLICANT_H
#define WPA_SUPPLICANT_VENDOR_AIDL_SUPPLICANT_H

#include <aidl/vendor/qti/hardware/wifi/supplicant/IVendorIfaceInfo.h>
#include <aidl/vendor/qti/hardware/wifi/supplicant/ISupplicantVendorStaIface.h>
#include <aidl/vendor/qti/hardware/wifi/supplicant/ISupplicantVendor.h>
#include <aidl/vendor/qti/hardware/wifi/supplicant/BnSupplicantVendor.h>

#include <android-base/macros.h>

extern "C"
{
#include "utils/common.h"
#include "utils/includes.h"
#include "utils/wpa_debug.h"
#include "wpa_supplicant_i.h"
#include "scan.h"
}

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace wifi {
namespace supplicant {

/**
 * Implementation of the supplicant aidl object. This aidl
 * object is used core for global control operations on
 * wpa_supplicant.
 */
class SupplicantVendor : public BnSupplicantVendor
{
public:
	SupplicantVendor(struct wpa_global* global);
	~SupplicantVendor() override = default;
	bool isValid();

	// Vendor Aidl methods exposed.
	::ndk::ScopedAStatus getVendorInterface(
		const IVendorIfaceInfo& vendor_iface_info, std::shared_ptr<ISupplicantVendorStaIface>* _aidl_return) override;
	::ndk::ScopedAStatus listVendorInterfaces(
		std::vector<IVendorIfaceInfo>* _aidl_return) override;
private:
	std::pair<std::shared_ptr<ISupplicantVendorStaIface>, ndk::ScopedAStatus>
		getVendorInterfaceInternal(const IVendorIfaceInfo& vendor_iface_info);
	std::pair<std::vector<IVendorIfaceInfo>, ndk::ScopedAStatus>
		listVendorInterfacesInternal();
	// Raw pointer to the global structure maintained by the core.
	struct wpa_global* wpa_global_;
	struct wpa_supplicant* retrieveIfacePtr();
	DISALLOW_COPY_AND_ASSIGN(SupplicantVendor);
};

}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
}  // namespace aidl

#endif  // WPA_SUPPLICANT_VENDOR_AIDL_SUPPLICANT_H
