/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef MISC_VENDOR_UTILS_H_
#define MISC_VENDOR_UTILS_H_

#include <iostream>
#include <aidl/vendor/qti/hardware/wifi/supplicant/SupplicantVendorStatusCode.h>

extern "C"
{
#include "wpabuf.h"
}

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace misc_vendor_utils {

// Wrappers to create a ScopedAStatus using a SupplicantVendorStatusCode
inline ndk::ScopedAStatus createStatus(SupplicantVendorStatusCode status_code) {
	return ndk::ScopedAStatus::fromServiceSpecificError(
		static_cast<int32_t>(status_code));
}

inline ndk::ScopedAStatus createStatusWithMsg(
	SupplicantVendorStatusCode status_code, std::string msg)
{
	return ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(
		static_cast<int32_t>(status_code), msg.c_str());
}

// Creates an std::string from a char*, which could be null
inline std::string charBufToString(const char* buf) {
	return buf ? std::string(buf) : "";
}

}  // namespace misc_vendor_utils
}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
}  // namespace aidl
#endif  // MISC_VENDOR_UTILS_H_
