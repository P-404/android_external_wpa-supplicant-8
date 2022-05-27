/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef AIDL_VENDOR_RETURN_UTIL_H_
#define AIDL_VENDOR_RETURN_UTIL_H_

#include <aidl/vendor/qti/hardware/wifi/supplicant/SupplicantVendorStatusCode.h>

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace aidl_vendor_return_util {

/**
 * These utility functions are used to invoke a method on the provided
 * AIDL interface object.
 * These functions check if the provided AIDL interface object is valid.
 * a) If valid, invokes the corresponding internal implementation function of
 * the AIDL method.
 * b) If invalid, return without calling the internal implementation function.
 */

// Use for AIDL methods which only return an AIDL status
template <typename ObjT, typename WorkFuncT, typename... Args>
::ndk::ScopedAStatus validateAndCall(
	ObjT* obj, SupplicantVendorStatusCode status_code_if_invalid, WorkFuncT&& work,
	Args&&... args)
{
	if (obj->isValid()) {
		return (obj->*work)(std::forward<Args>(args)...);
	} else {
		return ndk::ScopedAStatus::fromServiceSpecificError(
			static_cast<int32_t>(status_code_if_invalid));
	}
}

// Use for AIDL methods which have a return value along with the AIDL status
template <typename ObjT, typename WorkFuncT, typename ReturnT, typename... Args>
::ndk::ScopedAStatus validateAndCall(
	ObjT* obj, SupplicantVendorStatusCode status_code_if_invalid, WorkFuncT&& work,
	ReturnT* ret_val, Args&&... args)
{
	if (obj->isValid()) {
		auto call_pair = (obj->*work)(std::forward<Args>(args)...);
		*ret_val = call_pair.first;
		return std::forward<::ndk::ScopedAStatus>(call_pair.second);
	} else {
		return ndk::ScopedAStatus::fromServiceSpecificError(
			static_cast<int32_t>(status_code_if_invalid));
	}
}

}  // namespace aidl_return_util
}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
}  // namespace aidl
#endif  // AIDL_VENDOR_RETURN_UTIL_H_
