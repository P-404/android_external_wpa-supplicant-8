/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <android/binder_process.h>
#include <android/binder_manager.h>

#include "aidl_vendor_manager.h"

extern "C"
{
#include "aidl_vendor.h"
#include "aidl_vendor_i.h"
#include "utils/common.h"
#include "utils/eloop.h"
#include "utils/includes.h"
}

using aidl::vendor::qti::hardware::wifi::supplicant::AidlVendorManager;

struct wpas_aidl_vendor_priv *wpas_aidl_vendor_init(struct wpa_global *global)
{
	struct wpas_aidl_vendor_priv *priv;
	AidlVendorManager *aidl_vendor_manager;

	priv = (wpas_aidl_vendor_priv *)os_zalloc(sizeof(*priv));
	if (!priv)
		return NULL;
	priv->global = global;

	wpa_printf(MSG_DEBUG, "Initing vendor aidl control");
	aidl_vendor_manager = AidlVendorManager::getInstance();
	if (!aidl_vendor_manager)
		goto err;
	if (aidl_vendor_manager->registerVendorAidlService(global)) {
		wpa_printf(MSG_ERROR, "error in registerVendorAidlService");
		goto err;
	}
	priv->aidl_vendor_manager = (void *)aidl_vendor_manager;

	return priv;
err:
	wpas_aidl_vendor_deinit(priv);
	return NULL;
}

void wpas_aidl_vendor_deinit(struct wpas_aidl_vendor_priv *priv)
{
	if (!priv)
		return;

	wpa_printf(MSG_DEBUG, "Deiniting vendor aidl control");

	AidlVendorManager::destroyInstance();
	os_free(priv);
	priv = NULL;
}

int wpas_aidl_vendor_register_interface(struct wpa_supplicant *wpa_s)
{
	if (!wpa_s || !wpa_s->global->vendor_aidl)
		return 1;

	wpa_printf(
		MSG_DEBUG, "Registering interface to vendor aidl control: %s",
		wpa_s->ifname);

	AidlVendorManager *aidl_vendor_manager = AidlVendorManager::getInstance();
	if (!aidl_vendor_manager)
		return 1;

	return aidl_vendor_manager->registerVendorInterface(wpa_s);
}

int wpas_aidl_vendor_unregister_interface(struct wpa_supplicant *wpa_s)
{
	if (!wpa_s || !wpa_s->global->vendor_aidl)
		return 1;

	wpa_printf(
		MSG_DEBUG, "Deregistering interface from vendor aidl control: %s",
		wpa_s->ifname);

	AidlVendorManager *aidl_vendor_manager = AidlVendorManager::getInstance();
	if (!aidl_vendor_manager)
		return 1;

	return aidl_vendor_manager->unregisterVendorInterface(wpa_s);
}

