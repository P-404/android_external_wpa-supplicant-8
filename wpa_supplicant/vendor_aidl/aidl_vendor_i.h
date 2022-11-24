/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef AIDL_VENDOR_I_H
#define AIDL_VENDOR_I_H

#ifdef _cplusplus
extern "C"
{
#endif  // _cplusplus

	struct wpas_aidl_vendor_priv
	{
		struct wpa_global *global;
		void *aidl_vendor_manager;
	};

#ifdef _cplusplus
}
#endif  // _cplusplus

#endif  // AIDL_VENDOR_I_H
