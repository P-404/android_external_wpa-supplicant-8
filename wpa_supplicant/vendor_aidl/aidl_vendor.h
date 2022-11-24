/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef WPA_SUPPLICANT_VENDOR_AIDL_AIDL_H
#define WPA_SUPPLICANT_VENDOR_AIDL_AIDL_H

#ifdef _cplusplus
extern "C"
{
#endif  // _cplusplus

	/**
	 * This is the aidl RPC interface entry point to the vendor wpa_supplicant
	 * core. This initializes the aidl driver & AidlManager instance and
	 * then forwards all the notifcations from the supplicant core to the
	 * AidlVendorManager.
	 */
	struct wpas_aidl_vendor_priv;
	struct wpa_global;

	struct wpas_aidl_vendor_priv *wpas_aidl_vendor_init(struct wpa_global *global);
	void wpas_aidl_vendor_deinit(struct wpas_aidl_vendor_priv *priv);

#ifdef CONFIG_SUPPLICANT_VENDOR_AIDL
	int wpas_aidl_vendor_register_interface(struct wpa_supplicant *wpa_s);
	int wpas_aidl_vendor_unregister_interface(struct wpa_supplicant *wpa_s);
#else   // CONFIG_SUPPLICANT_VENDOR_AIDL

static inline int wpas_aidl_vendor_register_interface(struct wpa_supplicant *wpa_s)
{
	return 0;
}
static inline int wpas_aidl_vendor_unregister_interface(struct wpa_supplicant *wpa_s)
{
	return 0;
}
#endif  // CONFIG_SUPPLICANT_VENDOR_AIDL

#ifdef _cplusplus
}
#endif  // _cplusplus

#endif  // WPA_SUPPLICANT_VENDOR_AIDL_AIDL_H
