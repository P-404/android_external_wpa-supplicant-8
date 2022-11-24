/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef WPA_SUPPLICANT_VENDOR_AIDL_MANAGER_H
#define WPA_SUPPLICANT_VENDOR_AIDL_MANAGER_H

#include <map>
#include <string>

#include "vendorsta_iface.h"
#include "supplicantvendor.h"

extern "C"
{
#include "utils/common.h"
#include "utils/includes.h"
#include "wpa_supplicant_i.h"
#include "driver_i.h"
}

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace wifi {
namespace supplicant {

/**
 * AidlVendorManager is responsible for managing the lifetime of all
 * aidl objects created by wpa_supplicant. This is a singleton
 * class which is created by the supplicant core and can be used
 * to get references to the aidl objects.
 */
class AidlVendorManager
{
public:
	static AidlVendorManager *getInstance();
	static void destroyInstance();
	// Methods called from wpa_supplicant core.
	int registerVendorAidlService(struct wpa_global *global);
	int registerVendorInterface(struct wpa_supplicant *wpa_s);
	int unregisterVendorInterface(struct wpa_supplicant *wpa_s);
	int getVendorStaIfaceAidlObjectByIfname(
		const std::string &ifname,
		std::shared_ptr<aidl::vendor::qti::hardware::wifi::supplicant::ISupplicantVendorStaIface> *iface_object);
private:
	AidlVendorManager() = default;
	~AidlVendorManager() = default;
	AidlVendorManager(const AidlVendorManager &) = default;
	AidlVendorManager &operator=(const AidlVendorManager &) = default;

	// Singleton instance of this class.
	static AidlVendorManager *instance_;
	// Death notifier.
	AIBinder_DeathRecipient* death_notifier_;
	// The main vendor aidl service object.
	std::shared_ptr<SupplicantVendor> supplicant_vendor_object_;
	// Map of all the STA interface specific aidl objects controlled by
	// wpa_supplicant. This map is keyed in by the corresponding
	// |ifname|.
	std::map<const std::string, std::shared_ptr<VendorStaIface>>
		vendor_sta_iface_object_map_;
};
}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
}  // namespace aidl
#endif  // WPA_SUPPLICANT_VENDOR_AIDL_MANAGER_H
