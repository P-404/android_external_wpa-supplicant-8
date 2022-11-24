/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <algorithm>
#include <functional>
#include <iostream>
#include <regex>

#include "aidl_vendor_manager.h"
#include "misc_vendor_utils.h"
#include <android/binder_process.h>
#include <android/binder_manager.h>

extern "C" {
#include "scan.h"
#include "list.h"
}

namespace {

template <class ObjectType>
int addAidlObjectToMap(
	const std::string &key, const std::shared_ptr<ObjectType> &object,
	std::map<const std::string, std::shared_ptr<ObjectType>> &object_map)
{
	// Return failure if we already have an object for that |key|.
	if (object_map.find(key) != object_map.end())
		return 1;
	object_map[key] = object;
	if (!object_map[key].get())
		return 1;
	return 0;
}

template <class ObjectType>
int removeAidlObjectFromMap(
	const std::string &key,
	std::map<const std::string, std::shared_ptr<ObjectType>> &object_map)
{
	// Return failure if we dont have an object for that |key|.
	const auto &object_iter = object_map.find(key);
	if (object_iter == object_map.end())
		return 1;
	object_iter->second->invalidate();
	object_map.erase(object_iter);
	return 0;
}

void onDeath(void* cookie) {
	wpa_printf(MSG_ERROR, "Vendor Client died.");
}

}  // namespace

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace wifi {
namespace supplicant {

AidlVendorManager *AidlVendorManager::instance_ = NULL;

AidlVendorManager *AidlVendorManager::getInstance()
{
	if (!instance_)
		instance_ = new AidlVendorManager();
	return instance_;
}

void AidlVendorManager::destroyInstance()
{
	if (instance_)
		delete instance_;
	instance_ = NULL;
}

int AidlVendorManager::registerVendorAidlService(struct wpa_global *global)
{
	// Create the main aidl service object and register it.
	wpa_printf(MSG_INFO, "Starting AIDL vendor supplicant");
	supplicant_vendor_object_ = ndk::SharedRefBase::make<SupplicantVendor>(global);
	std::string instance = std::string() + SupplicantVendor::descriptor + "/default";
	if (AServiceManager_addService(supplicant_vendor_object_->asBinder().get(),
			instance.c_str()) != STATUS_OK)
	{
		return 1;
	}

	// Initialize the death notifier.
	death_notifier_ = AIBinder_DeathRecipient_new(onDeath);
	return 0;
}

/**
 * Check if the provided |wpa_supplicant| structure represents a P2P iface or
 * not.
 */
constexpr bool isP2pIface(const struct wpa_supplicant *wpa_s)
{
	return wpa_s->global->p2p_init_wpa_s == wpa_s;
}

/**
 * Register an interface to aidl vendor manager.
 *
 * @param wpa_s |wpa_supplicant| struct corresponding to the interface.
 *
 * @return 0 on success, 1 on failure.
 */
int AidlVendorManager::registerVendorInterface(struct wpa_supplicant *wpa_s)
{
	if (!wpa_s)
		return 1;

	if (isP2pIface(wpa_s)) {
		wpa_printf(MSG_INFO, "iface type is p2p0, don't add to map");
	} else {
		if (addAidlObjectToMap<VendorStaIface>(
			wpa_s->ifname,
			ndk::SharedRefBase::make<VendorStaIface>(wpa_s->global, wpa_s->ifname),
			vendor_sta_iface_object_map_)) {
			wpa_printf(
				MSG_ERROR,
				"Failed to register STA interface with AIDL "
				"control: %s",
				wpa_s->ifname);
			return 1;
		}
	}
	return 0;
}

/**
 * Unregister an interface from aidl vendor manager.
 *
 * @param wpa_s |wpa_supplicant| struct corresponding to the interface.
 *
 * @return 0 on success, 1 on failure.
 */
int AidlVendorManager::unregisterVendorInterface(struct wpa_supplicant *wpa_s)
{
	if (!wpa_s)
		return 1;

	bool success = !removeAidlObjectFromMap(
		wpa_s->ifname, vendor_sta_iface_object_map_);
	if (!success) {
		wpa_printf(
			MSG_ERROR,
			"Failed to unregister vendor interface with AIDL "
			"control: %s",
			wpa_s->ifname);
		return 1;
	}

	return 0;
}
/**
 * Retrieve the |ISupplicantVendorStaIface| aidl object reference using the provided
 * ifname.
 *
 * @param ifname Name of the corresponding interface.
 * @param vendor_iface_object Aidl reference corresponding to the iface.
 *
 * @return 0 on success, 1 on failure.
 */
int AidlVendorManager::getVendorStaIfaceAidlObjectByIfname(
	const std::string &ifname, std::shared_ptr<ISupplicantVendorStaIface> *vendor_iface_object)
{
	if (ifname.empty() || !vendor_iface_object)
		return 1;

	auto iface_object_iter = vendor_sta_iface_object_map_.find(ifname);
	if (iface_object_iter == vendor_sta_iface_object_map_.end())
		return 1;

	*vendor_iface_object = iface_object_iter->second;
	return 0;
}

}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
}  // namespace aidl
