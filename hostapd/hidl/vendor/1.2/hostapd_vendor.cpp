/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* vendor hidl interface for hostapd daemon */

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <cutils/properties.h>
#include <net/if.h>

#include <android-base/file.h>
#include <android-base/stringprintf.h>
#include <hidl/HidlSupport.h>

#include "hostapd_vendor.h"
#include "hidl_return_util.h"

extern "C"
{
#include "utils/eloop.h"
}

// The HIDL implementation for hostapd creates a hostapd.conf dynamically for
// each interface. This file can then be used to hook onto the normal config
// file parsing logic in hostapd code.  Helps us to avoid duplication of code
// in the HIDL interface.
namespace {
constexpr char kConfFileNameFmt[] = "/data/vendor/wifi/hostapd/hostapd_%s.conf";

using android::base::RemoveFileIfExists;
using android::base::StringPrintf;
using android::base::WriteStringToFile;
using android::hardware::wifi::hostapd::V1_0::IHostapd;
using vendor::qti::hardware::wifi::hostapd::V1_2::IHostapdVendor;
typedef IHostapdVendor::BandMask BandMask;

std::string WriteHostapdConfig(
    const std::string& interface_name, const std::string& config)
{
	const std::string file_path =
	    StringPrintf(kConfFileNameFmt, interface_name.c_str());
	if (WriteStringToFile(
		config, file_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
		getuid(), getgid())) {
		return file_path;
	}
	// Diagnose failure
	int error = errno;
	wpa_printf(
	    MSG_ERROR, "Cannot write hostapd config to %s, error: %s",
	    file_path.c_str(), strerror(error));
	struct stat st;
	int result = stat(file_path.c_str(), &st);
	if (result == 0) {
		wpa_printf(
		    MSG_ERROR, "hostapd config file uid: %d, gid: %d, mode: %d",
		    st.st_uid, st.st_gid, st.st_mode);
	} else {
		wpa_printf(
		    MSG_ERROR,
		    "Error calling stat() on hostapd config file: %s",
		    strerror(errno));
	}
	return "";
}

/*
 * Get the op_class for a channel/band
 * The logic here is based on Table E-4 in the 802.11 Specification
 */
int getOpClassForChannel(int channel, int band, bool support11n, bool support11ac) {
	// 2GHz Band
	if ((band & BandMask::BAND_2_GHZ) != 0) {
		if (channel == 14) {
			return 82;
		}
		if (channel >= 1 && channel <= 13) {
			if (!support11n) {
				//20MHz channel
				return 81;
			}
			if (channel <= 9) {
				// HT40 with secondary channel above primary
				return 83;
			}
			// HT40 with secondary channel below primary
			return 84;
		}
		// Error
		return 0;
	}

	// 5GHz Band
	if ((band & BandMask::BAND_5_GHZ) != 0) {
		if (support11ac) {
			switch (channel) {
				case 42:
				case 58:
				case 106:
				case 122:
				case 138:
				case 155:
					// 80MHz channel
					return 128;
				case 50:
				case 114:
					// 160MHz channel
					return 129;
			}
		}

		if (!support11n) {
			if (channel >= 36 && channel <= 48) {
				return 115;
			}
			if (channel >= 52 && channel <= 64) {
				return 118;
			}
			if (channel >= 100 && channel <= 144) {
				return 121;
			}
			if (channel >= 149 && channel <= 161) {
				return 124;
			}
			if (channel >= 165 && channel <= 169) {
				return 125;
			}
		} else {
			switch (channel) {
				case 36:
				case 44:
					// HT40 with secondary channel above primary
					return 116;
				case 40:
				case 48:
					// HT40 with secondary channel below primary
					return 117;
				case 52:
				case 60:
					// HT40 with secondary channel above primary
					return  119;
				case 56:
				case 64:
					// HT40 with secondary channel below primary
					return 120;
				case 100:
				case 108:
				case 116:
				case 124:
				case 132:
				case 140:
					// HT40 with secondary channel above primary
					return 122;
				case 104:
				case 112:
				case 120:
				case 128:
				case 136:
				case 144:
					// HT40 with secondary channel below primary
					return 123;
				case 149:
				case 157:
					// HT40 with secondary channel above primary
					return 126;
				case 153:
				case 161:
					// HT40 with secondary channel below primary
					return 127;
			}
		}
		// Error
		return 0;
	}

	// 6GHz Band
	if ((band & BandMask::BAND_6_GHZ) != 0) {
		// Channels 1, 5. 9, 13, ...
		if ((channel & 0x03) == 0x01) {
			// 20MHz channel
			return 131;
		}
		// Channels 3, 11, 19, 27, ...
		if ((channel & 0x07) == 0x03) {
			// 40MHz channel
			return 132;
		}
		// Channels 7, 23, 39, 55, ...
		if ((channel & 0x0F) == 0x07) {
			// 80MHz channel
			return 133;
		}
		// Channels 15, 47, 69, ...
		if ((channel & 0x1F) == 0x0F) {
			// 160MHz channel
			return 134;
		}
		// Error
		return 0;
	}

	return 0;
}

bool validatePassphrase(int passphrase_len, int min_len, int max_len)
{
	if (min_len != -1 && passphrase_len < min_len) return false;
	if (max_len != -1 && passphrase_len > max_len) return false;
	return true;
}

std::string CreateHostapdConfig(
    const IHostapdVendor::VendorIfaceParams& v_iface_params,
    const IHostapdVendor::VendorNetworkParams& v_nw_params)
{
	IHostapd::IfaceParams iface_params = v_iface_params.VendorV1_1.VendorV1_0.ifaceParams;
	IHostapd::ChannelParams channelParams = v_iface_params.VendorV1_1.vendorChannelParams.channelParams;
	IHostapd::NetworkParams nw_params = v_nw_params.V1_0;
	if (nw_params.ssid.size() >
	    static_cast<uint32_t>(
		IHostapd::ParamSizeLimits::SSID_MAX_LEN_IN_BYTES)) {
		wpa_printf(
		    MSG_ERROR, "Invalid SSID size: %zu", nw_params.ssid.size());
		return "";
	}

	// SSID string
	std::stringstream ss;
	ss << std::hex;
	ss << std::setfill('0');
	for (uint8_t b : nw_params.ssid) {
		ss << std::setw(2) << static_cast<unsigned int>(b);
	}
	const std::string ssid_as_string = ss.str();

	const int wigigOpClass = (180 << 16);
	bool isWigig = ((channelParams.channel & 0xFF0000) == wigigOpClass);
	channelParams.channel &= 0xFFFF;

	unsigned int band = 0;
	band |= v_iface_params.channelParams.bandMask;
	bool is_6Ghz_band_only = (band  == static_cast<uint32_t>(IHostapdVendor::BandMask::BAND_6_GHZ));
	// Encryption config string
	std::string encryption_config_as_string;
	switch (v_nw_params.vendorEncryptionType) {
	case IHostapdVendor::VendorEncryptionType::NONE:
		// no security params
#ifdef CONFIG_OWE
		if (!v_iface_params.VendorV1_1.oweTransIfaceName.empty()) {
			encryption_config_as_string = StringPrintf(
				"owe_transition_ifname=%s",
				v_iface_params.VendorV1_1.oweTransIfaceName.c_str());
		}
#endif
		break;
	case IHostapdVendor::VendorEncryptionType::WPA:
		if (!validatePassphrase(
		    v_nw_params.passphrase.size(),
		    static_cast<uint32_t>(IHostapd::ParamSizeLimits::
				WPA2_PSK_PASSPHRASE_MIN_LEN_IN_BYTES),
		    static_cast<uint32_t>(IHostapd::ParamSizeLimits::
				WPA2_PSK_PASSPHRASE_MAX_LEN_IN_BYTES))) {
			return "";
		}
		encryption_config_as_string = StringPrintf(
		    "wpa=3\n"
		    "wpa_pairwise=%s\n"
		    "wpa_passphrase=%s",
		    isWigig ? "GCMP" : "TKIP CCMP", v_nw_params.passphrase.c_str());
		break;
	case IHostapdVendor::VendorEncryptionType::WPA2:
		if (!validatePassphrase(
		    v_nw_params.passphrase.size(),
		    static_cast<uint32_t>(IHostapd::ParamSizeLimits::
				WPA2_PSK_PASSPHRASE_MIN_LEN_IN_BYTES),
		    static_cast<uint32_t>(IHostapd::ParamSizeLimits::
				WPA2_PSK_PASSPHRASE_MAX_LEN_IN_BYTES))) {
			return "";
		}
		encryption_config_as_string = StringPrintf(
		    "wpa=2\n"
		    "rsn_pairwise=%s\n"
		    "wpa_passphrase=%s\n"
		    "ieee80211w=1",
		    isWigig ? "GCMP" : "CCMP", v_nw_params.passphrase.c_str());
		break;
#ifdef CONFIG_SAE
	case IHostapdVendor::VendorEncryptionType::SAE_TRANSITION:
		if (!validatePassphrase(
		    v_nw_params.passphrase.size(),
		    static_cast<uint32_t>(IHostapd::ParamSizeLimits::
				WPA2_PSK_PASSPHRASE_MIN_LEN_IN_BYTES),
		    static_cast<uint32_t>(IHostapd::ParamSizeLimits::
				WPA2_PSK_PASSPHRASE_MAX_LEN_IN_BYTES))) {
			return "";
		}
		encryption_config_as_string = StringPrintf(
		    "wpa=2\n"
		    "rsn_pairwise=%s\n"
		    "wpa_key_mgmt=WPA-PSK SAE\n"
		    "ieee80211w=1\n"
		    "sae_require_mfp=1\n"
		    "wpa_passphrase=%s\n"
		    "sae_password=%s\n"
		    "sae_pwe=%d",
		    isWigig ? "GCMP" : "CCMP",
		    v_nw_params.passphrase.c_str(),
		    v_nw_params.passphrase.c_str(),
		    is_6Ghz_band_only ? 1 : 2);
		break;
	case IHostapdVendor::VendorEncryptionType::SAE:
		if (!validatePassphrase(v_nw_params.passphrase.size(), 1, -1)) {
			return "";
		}
		encryption_config_as_string = StringPrintf(
		    "wpa=2\n"
		    "rsn_pairwise=%s\n"
		    "wpa_key_mgmt=SAE\n"
		    "ieee80211w=2\n"
		    "sae_require_mfp=2\n"
		    "sae_password=%s\n"
		    "sae_pwe=%d",
		    isWigig ? "GCMP" : "CCMP",
		    v_nw_params.passphrase.c_str(),
		    is_6Ghz_band_only ? 1 : 2);
		break;
#endif /* CONFIG_SAE */
#ifdef CONFIG_OWE
	case IHostapdVendor::VendorEncryptionType::OWE: {
		std::string owe_transition_ifname_as_string;
		if (!v_iface_params.VendorV1_1.oweTransIfaceName.empty()) {
			owe_transition_ifname_as_string = StringPrintf(
			    "owe_transition_ifname=%s\n",
			    v_iface_params.VendorV1_1.oweTransIfaceName.c_str());
		}
		encryption_config_as_string = StringPrintf(
		    "wpa=2\n"
		    "rsn_pairwise=%s\n"
		    "wpa_key_mgmt=OWE\n"
		    "ieee80211w=2\n%s",
		    isWigig ? "GCMP" : "CCMP",
		    owe_transition_ifname_as_string.c_str());
		break;
		}
#endif /* CONFIG_OWE */
	default:
		wpa_printf(MSG_ERROR, "Unknown encryption type");
		return "";
	}

	std::string channel_config_as_string;
	bool isFirst = true;
	if (channelParams.enableAcs) {
		std::string freqList_as_string;
		for (const auto &range :
		    v_iface_params.channelParams.acsChannelFreqRangesMhz) {
			if (!isFirst) {
				freqList_as_string += ",";
			}
			isFirst = false;

			if (range.start != range.end) {
				freqList_as_string +=
				    StringPrintf("%d-%d", range.start, range.end);
			} else {
				freqList_as_string += StringPrintf("%d", range.start);
			}
		}
		channel_config_as_string = StringPrintf(
		    "channel=0\n"
		    "acs_exclude_dfs=%d\n"
		    "ht_capab=[HT40+]\n"
		    "vht_oper_chwidth=1\n"
		    "he_oper_chwidth=1\n"
		    "freqlist=%s",
		    channelParams.acsShouldExcludeDfs,
		    freqList_as_string.c_str());

		if (((band & BandMask::BAND_6_GHZ) != 0)) {
			int op_class = 134;
			channel_config_as_string = StringPrintf(
			    "channel=0\n"
			    "acs_exclude_dfs=%d\n"
			    "ht_capab=[HT40+]\n"
			    "vht_oper_chwidth=1\n"
			    "he_oper_chwidth=1\n"
			    "freqlist=%s\n"
			    "op_class=%d",
			    channelParams.acsShouldExcludeDfs,
			    freqList_as_string.c_str(),
			    op_class);
		}

	} else {
		int op_class = getOpClassForChannel(channelParams.channel, band,
				  iface_params.hwModeParams.enable80211N,
				  iface_params.hwModeParams.enable80211AC);
		channel_config_as_string = StringPrintf(
		    "channel=%d\n"
		    "op_class=%d",
		    channelParams.channel, op_class);
	}

	std::string hw_mode_as_string;
	if ((band & BandMask::BAND_2_GHZ) != 0) {
		if (((band & BandMask::BAND_5_GHZ) != 0)
		    || ((band & BandMask::BAND_6_GHZ) != 0)) {
			hw_mode_as_string = "hw_mode=any";
		} else {
			hw_mode_as_string = "hw_mode=g";
		}
	} else {
		if (((band & BandMask::BAND_5_GHZ) != 0)
		    || ((band & BandMask::BAND_6_GHZ) != 0)) {
			hw_mode_as_string = "hw_mode=a";
		} else {
			wpa_printf(MSG_ERROR, "Invalid band");
			return "";
		}
	}

	std::string he_params_as_string;
#ifdef CONFIG_IEEE80211AX
	if (v_iface_params.hwModeParams.enable80211AX) {
		int he_bss_color = os_random() % 63 + 1;
		he_params_as_string = StringPrintf(
		    "ieee80211ax=1\n"
		    "he_bss_color=%d\n"
		    "he_su_beamformer=%d\n"
		    "he_su_beamformee=%d\n"
		    "he_mu_beamformer=%d\n"
		    "he_twt_required=%d\n",
		    he_bss_color,
		    v_iface_params.hwModeParams.enableHeSingleUserBeamformer ? 1 : 0,
		    v_iface_params.hwModeParams.enableHeSingleUserBeamformee ? 1 : 0,
		    v_iface_params.hwModeParams.enableHeMultiUserBeamformer ? 1 : 0,
		    v_iface_params.hwModeParams.enableHeTargetWakeTime ? 1 : 0);
	} else {
		he_params_as_string = "ieee80211ax=0";
	}
#endif /* CONFIG_IEEE80211AX */
	std::string bridge_as_string;
	if (!v_iface_params.VendorV1_1.VendorV1_0.bridgeIfaceName.empty())
		bridge_as_string = StringPrintf(
		    "bridge=%s",
		    v_iface_params.VendorV1_1.VendorV1_0.bridgeIfaceName.c_str());

	std::string country_as_string;
	if (!v_iface_params.VendorV1_1.VendorV1_0.countryCode.empty())
		country_as_string = StringPrintf(
		    "country_code=%s",
		    v_iface_params.VendorV1_1.VendorV1_0.countryCode.c_str());

	return StringPrintf(
	    "interface=%s\n"
	    "driver=nl80211\n"
	    "ctrl_interface=/data/vendor/wifi/hostapd/ctrl\n"
	    // ssid2 signals to hostapd that the value is not a literal value
	    // for use as a SSID.  In this case, we're giving it a hex
	    // std::string and hostapd needs to expect that.
	    "ssid2=%s\n"
	    "%s\n"
	    "ieee80211n=%d\n"
	    "ieee80211ac=%d\n"
	    "%s\n"
	    "%s\n"
	    "%s\n"
	    "%s\n"
	    "ignore_broadcast_ssid=%d\n"
	    "wowlan_triggers=any\n"
	    "%s\n"
	    "ocv=2\n"
	    "beacon_prot=1\n",
	    iface_params.ifaceName.c_str(), ssid_as_string.c_str(),
	    channel_config_as_string.c_str(),
	    iface_params.hwModeParams.enable80211N ? 1 : 0,
	    iface_params.hwModeParams.enable80211AC ? 1 : 0,
	    he_params_as_string.c_str(), hw_mode_as_string.c_str(),
	    bridge_as_string.c_str(), country_as_string.c_str(),
	    nw_params.isHidden ? 1 : 0, encryption_config_as_string.c_str());
}

template <class CallbackType>
int addIfaceCallbackHidlObjectToMap(
    const std::string &ifname, const android::sp<CallbackType> &callback,
    std::map<const std::string, std::vector<android::sp<CallbackType>>>
	&callbacks_map)
{
	if (ifname.empty())
		return 1;

	auto iface_callback_map_iter = callbacks_map.find(ifname);
	if (iface_callback_map_iter == callbacks_map.end())
		return 1;

	auto &iface_callback_list = iface_callback_map_iter->second;

        std::vector<android::sp<CallbackType>> &callback_list = iface_callback_list;
	callback_list.push_back(callback);
	return 0;
}

template <class CallbackType>
void callWithEachIfaceCallback(
    const std::string &ifname,
    const std::function<
	android::hardware::Return<void>(android::sp<CallbackType>)> &method,
    const std::map<const std::string, std::vector<android::sp<CallbackType>>>
	&callbacks_map)
{
	if (ifname.empty())
		return;

	auto iface_callback_map_iter = callbacks_map.find(ifname);
	if (iface_callback_map_iter == callbacks_map.end())
		return;

	const auto &iface_callback_list = iface_callback_map_iter->second;
	for (const auto &callback : iface_callback_list) {
		if (!method(callback).isOk()) {
			wpa_printf(
			    MSG_ERROR, "Failed to invoke Hostapd HIDL iface callback");
		}
	}
}

// hostapd core functions accept "C" style function pointers, so use global
// functions to pass to the hostapd core function and store the corresponding
// std::function methods to be invoked.
//
// NOTE: Using the pattern from the vendor HAL (wifi_legacy_hal.cpp).
//
// Callback to be invoked once setup is complete
std::function<void(struct hostapd_data*)> on_setup_complete_internal_callback;
void onAsyncSetupCompleteCb(void* ctx)
{
	struct hostapd_data* iface_hapd = (struct hostapd_data*)ctx;
	if (on_setup_complete_internal_callback) {
		on_setup_complete_internal_callback(iface_hapd);
		// Invalidate this callback since we don't want this firing
		// again.
		on_setup_complete_internal_callback = nullptr;
	}
}

std::function<void(struct hostapd_data*, const u8 *mac_addr, int authorized,
		   const u8 *p2p_dev_addr)> on_sta_authorized_internal_callback;
void onAsyncStaAuthorizedCb(void* ctx, const u8 *mac_addr, int authorized,
			    const u8 *p2p_dev_addr)
{
	struct hostapd_data* iface_hapd = (struct hostapd_data*)ctx;
	if (on_sta_authorized_internal_callback) {
		on_sta_authorized_internal_callback(iface_hapd, mac_addr,
			authorized, p2p_dev_addr);
	}
}

}  // namespace



namespace vendor {
namespace qti {
namespace hardware {
namespace wifi {
namespace hostapd {
namespace V1_2 {
namespace implementation {

using namespace android::hardware;
using namespace android::hardware::wifi::hostapd::V1_0;
using namespace android::hardware::wifi::hostapd::V1_0::implementation::hidl_return_util;

using android::base::StringPrintf;

HostapdVendor::HostapdVendor(struct hapd_interfaces* interfaces)
    : interfaces_(interfaces)
{
}

Return<void> HostapdVendor::addVendorAccessPoint(
    const V1_0::IHostapdVendor::VendorIfaceParams& iface_params, const NetworkParamsV1_0& nw_params,
    addVendorAccessPoint_cb _hidl_cb)
{
	return call(
	    this, &HostapdVendor::addVendorAccessPointInternal, _hidl_cb, iface_params,
	    nw_params);
}

Return<void> HostapdVendor::addVendorAccessPoint_1_1(
    const V1_1::IHostapdVendor::VendorIfaceParams& iface_params, const NetworkParamsV1_0& nw_params,
    addVendorAccessPoint_cb _hidl_cb)
{
	return call(
	    this, &HostapdVendor::addVendorAccessPointInternal_1_1, _hidl_cb, iface_params,
	    nw_params);
}

Return<void> HostapdVendor::addVendorAccessPoint_1_2(
    const VendorIfaceParams& iface_params, const VendorNetworkParams& nw_params,
    addVendorAccessPoint_1_2_cb _hidl_cb)
{
	return call(
	    this, &HostapdVendor::addVendorAccessPointInternal_1_2, _hidl_cb, iface_params,
	    nw_params);
}

Return<void> HostapdVendor::removeVendorAccessPoint(
    const hidl_string& iface_name, removeVendorAccessPoint_cb _hidl_cb)
{
	return call(
	    this, &HostapdVendor::removeVendorAccessPointInternal, _hidl_cb, iface_name);
}

Return<void> HostapdVendor::setHostapdParams(
    const hidl_string& cmd, setHostapdParams_cb _hidl_cb)
{
	return call(
	    this, &HostapdVendor::setHostapdParamsInternal, _hidl_cb, cmd);
}

Return<void> HostapdVendor::setDebugParams(
    IHostapdVendor::DebugLevel level, bool show_timestamp, bool show_keys,
    setDebugParams_cb _hidl_cb)
{
	return call(
	    this, &HostapdVendor::setDebugParamsInternal, _hidl_cb, level,
	    show_timestamp, show_keys);
}

Return<IHostapdVendor::DebugLevel> HostapdVendor::getDebugLevel()
{
	return (IHostapdVendor::DebugLevel)wpa_debug_level;
}
Return<void> HostapdVendor::registerVendorCallback(
    const hidl_string& iface_name,
    const android::sp<V1_0::IHostapdVendorIfaceCallback> &callback,
    registerVendorCallback_cb _hidl_cb)
{
	return call(
	    this, &HostapdVendor::registerCallbackInternal, _hidl_cb, iface_name, callback);
}

Return<void> HostapdVendor::registerVendorCallback_1_1(
    const hidl_string& iface_name,
    const android::sp<V1_1::IHostapdVendorIfaceCallback> &callback,
    registerVendorCallback_cb _hidl_cb)
{
	return call(
	    this, &HostapdVendor::registerCallbackInternal_1_1, _hidl_cb, iface_name, callback);
}

Return<void> HostapdVendor::listInterfaces(listInterfaces_cb _hidl_cb)
{
	_hidl_cb({HostapdStatusCode::FAILURE_UNKNOWN, "Not supported"}, NULL);
	return Void();
}

Return<void> HostapdVendor::hostapdCmd(const hidl_string& ifname, const hidl_string& cmd,
			hostapdCmd_cb _hidl_cb)
{
	_hidl_cb({HostapdStatusCode::FAILURE_UNKNOWN, "Not supported"}, NULL);
	return Void();
}

HostapdStatus HostapdVendor::addVendorAccessPointInternal(
    const V1_0::IHostapdVendor::VendorIfaceParams& v_iface_params, const NetworkParamsV1_0& nw_params)
{
	// Deprecated support for this API
	return {HostapdStatusCode::FAILURE_UNKNOWN, "Not supported"};
}


HostapdStatus HostapdVendor::__addVendorAccessPointInternal_1_2(
    const VendorIfaceParams& v_iface_params, const VendorNetworkParams& v_nw_params)
{
	IfaceParamsV1_0 iface_params = v_iface_params.VendorV1_1.VendorV1_0.ifaceParams;
	const auto conf_params = CreateHostapdConfig(v_iface_params, v_nw_params);
	if (conf_params.empty()) {
		wpa_printf(MSG_ERROR, "Failed to create config params");
		return {HostapdStatusCode::FAILURE_ARGS_INVALID, ""};
	}
	const auto conf_file_path =
	    WriteHostapdConfig(iface_params.ifaceName, conf_params);
	if (conf_file_path.empty()) {
		wpa_printf(MSG_ERROR, "Failed to write config file");
		return {HostapdStatusCode::FAILURE_UNKNOWN, ""};
	}

	std::string add_iface_param_str = StringPrintf(
	    "%s config=%s", iface_params.ifaceName.c_str(),
	    conf_file_path.c_str());
	std::vector<char> add_iface_param_vec(
	    add_iface_param_str.begin(), add_iface_param_str.end() + 1);
	if (hostapd_add_iface(interfaces_, add_iface_param_vec.data()) < 0) {
		wpa_printf(
		    MSG_ERROR, "Adding interface %s failed",
		    add_iface_param_str.c_str());
		return {HostapdStatusCode::FAILURE_UNKNOWN, ""};
	}
	struct hostapd_data* iface_hapd =
	    hostapd_get_iface(interfaces_, iface_params.ifaceName.c_str());
	WPA_ASSERT(iface_hapd != nullptr && iface_hapd->iface != nullptr);
	if (hostapd_enable_iface(iface_hapd->iface) < 0) {
		wpa_printf(
		    MSG_ERROR, "Enabling interface %s failed",
		    iface_params.ifaceName.c_str());
		return {HostapdStatusCode::FAILURE_UNKNOWN, ""};
	}

	// Register the setup complete callbacks
	on_setup_complete_internal_callback =
	    [this](struct hostapd_data* iface_hapd) {
		    wpa_printf(
			MSG_DEBUG, "AP interface setup completed - state %s",
			hostapd_state_text(iface_hapd->iface->state));
		    if (iface_hapd->iface->state == HAPD_IFACE_DISABLED) {
			    callWithEachHostapdIfaceCallback(
				iface_hapd->conf->iface,
				std::bind(
				&V1_1::IHostapdVendorIfaceCallback::onFailure, std::placeholders::_1,
				iface_hapd->conf->iface));
		    }
	    };
	iface_hapd->setup_complete_cb = onAsyncSetupCompleteCb;
	iface_hapd->setup_complete_cb_ctx = iface_hapd;

	// Listen for new Sta connect/disconnect indication.
	on_sta_authorized_internal_callback =
	    [this](struct hostapd_data* iface_hapd, const u8 *mac_addr,
		   int authorized, const u8 *p2p_dev_addr) {
		wpa_printf(MSG_DEBUG, "vendor hidl: [%s] notify sta " MACSTR " %s",
			   iface_hapd->conf->iface, MAC2STR(mac_addr),
			   (authorized) ? "Connected" : "Disconnected");
		if (authorized)
			callWithEachHostapdIfaceCallback(
			    iface_hapd->conf->iface,
			    std::bind(
			    &V1_1::IHostapdVendorIfaceCallback::onStaConnected, std::placeholders::_1,
			    mac_addr));
		else
			callWithEachHostapdIfaceCallback(
			    iface_hapd->conf->iface,
			    std::bind(
			    &V1_1::IHostapdVendorIfaceCallback::onStaDisconnected, std::placeholders::_1,
			    mac_addr));
	    };
	iface_hapd->sta_authorized_cb = onAsyncStaAuthorizedCb;
	iface_hapd->sta_authorized_cb_ctx = iface_hapd;

	return {HostapdStatusCode::SUCCESS, ""};
}


HostapdStatus HostapdVendor::addVendorAccessPointInternal_1_1(
    const V1_1::IHostapdVendor::VendorIfaceParams& v1_1_iface_params, const NetworkParamsV1_0& nw_params)
{
	// Deprecated support for this API
	return {HostapdStatusCode::FAILURE_UNKNOWN, "Not supported"};
}


HostapdStatus HostapdVendor::addVendorAccessPointInternal_1_2(
    const VendorIfaceParams& v1_2_iface_params, const VendorNetworkParams& v_nw_params)
{
	VendorIfaceParams v_iface_params = v1_2_iface_params;
	IfaceParamsV1_0 iface_params = v_iface_params.VendorV1_1.VendorV1_0.ifaceParams;
	if (hostapd_get_iface(interfaces_, iface_params.ifaceName.c_str())) {
		wpa_printf(
		    MSG_ERROR, "Interface %s already present",
		    iface_params.ifaceName.c_str());
		return {HostapdStatusCode::FAILURE_IFACE_EXISTS, ""};
	}

	return __addVendorAccessPointInternal_1_2(v1_2_iface_params, v_nw_params);
}

HostapdStatus HostapdVendor::removeVendorAccessPointInternal(const std::string& iface_name)
{
	// Deprecated support for this API
	return {HostapdStatusCode::FAILURE_UNKNOWN, "Not supported"};
}

HostapdStatus HostapdVendor::setHostapdParamsInternal(const std::string& cmd)
{
	// Deprecated support for this API
	return {HostapdStatusCode::FAILURE_UNKNOWN, "Not supported"};
}

HostapdStatus HostapdVendor::setDebugParamsInternal(
    IHostapdVendor::DebugLevel level, bool show_timestamp, bool show_keys)
{
	// Deprecated support for this API
	return {HostapdStatusCode::FAILURE_UNKNOWN, "Not supported"};
}

HostapdStatus HostapdVendor::registerCallbackInternal(
    const std::string& iface_name,
    const android::sp<V1_0::IHostapdVendorIfaceCallback> &callback)
{
	// Deprecated support for this callback
	return {HostapdStatusCode::FAILURE_UNKNOWN, "Not supported"};
}

HostapdStatus HostapdVendor::registerCallbackInternal_1_1(
    const std::string& iface_name,
    const android::sp<V1_1::IHostapdVendorIfaceCallback> &callback)
{
	if(addVendorIfaceCallbackHidlObject(iface_name, callback)) {
	   wpa_printf(MSG_ERROR, "FAILED to register Hostapd Vendor IfaceCallback");
	   return {HostapdStatusCode::FAILURE_UNKNOWN, ""};
	}

	return {HostapdStatusCode::SUCCESS, ""};
}

/**
 * Add a new Hostapd vendoriface callback hidl object
 * reference to our interface callback list.
 *
 * @param ifname Name of the corresponding interface.
 * @param callback Hidl reference of the vendor ifacecallback object.
 *
 * @return 0 on success, 1 on failure.
 */
int HostapdVendor::addVendorIfaceCallbackHidlObject(
    const std::string &ifname,
    const android::sp<V1_1::IHostapdVendorIfaceCallback> &callback)
{
	vendor_hostapd_callbacks_map_[ifname] =
		    std::vector<android::sp<V1_1::IHostapdVendorIfaceCallback>>();
	return addIfaceCallbackHidlObjectToMap(ifname, callback, vendor_hostapd_callbacks_map_);
}

/**
 * Helper function to invoke the provided callback method on all the
 * registered |IHostapdVendorIfaceCallback| callback hidl objects.
 *
 * @param ifname Name of the corresponding interface.
 * @param method Pointer to the required hidl method from
 * |IHostapdVendorIfaceCallback|.
 */
void HostapdVendor::callWithEachHostapdIfaceCallback(
    const std::string &ifname,
    const std::function<Return<void>(android::sp<V1_1::IHostapdVendorIfaceCallback>)> &method)
{
	callWithEachIfaceCallback(ifname, method, vendor_hostapd_callbacks_map_);
}

void HostapdVendor::invalidate()
{
	wpa_printf(MSG_DEBUG, "invalidate hostapd vendor callbacks");
	on_setup_complete_internal_callback = nullptr;
	on_sta_authorized_internal_callback = nullptr;
}

}  // namespace implementation
}  // namespace V1_2
}  // namespace hostapd
}  // namespace wifi
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
