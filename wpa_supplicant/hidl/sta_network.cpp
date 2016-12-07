/*
 * hidl interface for wpa_supplicant daemon
 * Copyright (c) 2004-2016, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2004-2016, Roshan Pius <rpius@google.com>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "hidl_manager.h"
#include "hidl_return_util.h"
#include "sta_network.h"

namespace {
using android::hardware::wifi::supplicant::V1_0::ISupplicantStaNetwork;
using android::hardware::wifi::supplicant::V1_0::SupplicantStatus;

constexpr uint8_t kZeroBssid[6] = {0, 0, 0, 0, 0, 0};

constexpr uint32_t kAllowedKeyMgmtMask =
    (static_cast<uint32_t>(ISupplicantStaNetwork::KeyMgmtMask::NONE) |
     static_cast<uint32_t>(ISupplicantStaNetwork::KeyMgmtMask::WPA_PSK) |
     static_cast<uint32_t>(ISupplicantStaNetwork::KeyMgmtMask::WPA_EAP) |
     static_cast<uint32_t>(ISupplicantStaNetwork::KeyMgmtMask::IEEE8021X));
constexpr uint32_t kAllowedproto_mask =
    (static_cast<uint32_t>(ISupplicantStaNetwork::ProtoMask::WPA) |
     static_cast<uint32_t>(ISupplicantStaNetwork::ProtoMask::RSN) |
     static_cast<uint32_t>(ISupplicantStaNetwork::ProtoMask::OSEN));
constexpr uint32_t kAllowedauth_alg_mask =
    (static_cast<uint32_t>(ISupplicantStaNetwork::AuthAlgMask::OPEN) |
     static_cast<uint32_t>(ISupplicantStaNetwork::AuthAlgMask::SHARED) |
     static_cast<uint32_t>(ISupplicantStaNetwork::AuthAlgMask::LEAP));
constexpr uint32_t kAllowedgroup_cipher_mask =
    (static_cast<uint32_t>(ISupplicantStaNetwork::GroupCipherMask::WEP40) |
     static_cast<uint32_t>(ISupplicantStaNetwork::GroupCipherMask::WEP104) |
     static_cast<uint32_t>(ISupplicantStaNetwork::GroupCipherMask::TKIP) |
     static_cast<uint32_t>(ISupplicantStaNetwork::GroupCipherMask::CCMP));
constexpr uint32_t kAllowedpairwise_cipher_mask =
    (static_cast<uint32_t>(ISupplicantStaNetwork::PairwiseCipherMask::NONE) |
     static_cast<uint32_t>(ISupplicantStaNetwork::PairwiseCipherMask::TKIP) |
     static_cast<uint32_t>(ISupplicantStaNetwork::PairwiseCipherMask::CCMP));

constexpr uint32_t kEapMethodMax =
    static_cast<uint32_t>(ISupplicantStaNetwork::EapMethod::WFA_UNAUTH_TLS) + 1;
constexpr char const *kEapMethodStrings[kEapMethodMax] = {
    "PEAP", "TLS", "TTLS", "PWD", "SIM", "AKA", "AKA'", "WFA-UNAUTH-TLS"};
constexpr uint32_t kEapPhase2MethodMax =
    static_cast<uint32_t>(ISupplicantStaNetwork::EapPhase2Method::GTC) + 1;
constexpr char const *kEapPhase2MethodStrings[kEapPhase2MethodMax] = {
    "NULL", "PAP", "MSCHAP", "MSCHAPV2", "GTC"};
}  // namespace

namespace android {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V1_0 {
namespace implementation {
using hidl_return_util::validateAndCall;

StaNetwork::StaNetwork(
    struct wpa_global *wpa_global, const char ifname[], int network_id)
    : wpa_global_(wpa_global),
      ifname_(ifname),
      network_id_(network_id),
      is_valid_(true)
{
}

void StaNetwork::invalidate() { is_valid_ = false; }
bool StaNetwork::isValid()
{
	return (is_valid_ && (retrieveNetworkPtr() != nullptr));
}

Return<void> StaNetwork::getId(getId_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getIdInternal, _hidl_cb);
}

Return<void> StaNetwork::getInterfaceName(getInterfaceName_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getInterfaceNameInternal, _hidl_cb);
}

Return<void> StaNetwork::getType(getType_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getTypeInternal, _hidl_cb);
}

Return<void> StaNetwork::registerCallback(
    const sp<ISupplicantStaNetworkCallback> &callback,
    registerCallback_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::registerCallbackInternal, _hidl_cb, callback);
}

Return<void> StaNetwork::setSsid(
    const hidl_vec<uint8_t> &ssid, setSsid_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setSsidInternal, _hidl_cb, ssid);
}

Return<void> StaNetwork::setBssid(
    const hidl_array<uint8_t, 6> &bssid, setBssid_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setBssidInternal, _hidl_cb, bssid);
}

Return<void> StaNetwork::setScanSsid(bool enable, setScanSsid_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setScanSsidInternal, _hidl_cb, enable);
}

Return<void> StaNetwork::setKeyMgmt(
    uint32_t key_mgmt_mask, setKeyMgmt_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setKeyMgmtInternal, _hidl_cb, key_mgmt_mask);
}

Return<void> StaNetwork::setProto(uint32_t proto_mask, setProto_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setProtoInternal, _hidl_cb, proto_mask);
}

Return<void> StaNetwork::setAuthAlg(
    uint32_t auth_alg_mask,
    std::function<void(const SupplicantStatus &status)> _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setAuthAlgInternal, _hidl_cb, auth_alg_mask);
}

Return<void> StaNetwork::setGroupCipher(
    uint32_t group_cipher_mask, setGroupCipher_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setGroupCipherInternal, _hidl_cb, group_cipher_mask);
}

Return<void> StaNetwork::setPairwiseCipher(
    uint32_t pairwise_cipher_mask, setPairwiseCipher_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setPairwiseCipherInternal, _hidl_cb,
	    pairwise_cipher_mask);
}

Return<void> StaNetwork::setPskPassphrase(
    const hidl_string &psk, setPskPassphrase_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setPskPassphraseInternal, _hidl_cb, psk);
}

Return<void> StaNetwork::setWepKey(
    uint32_t key_idx, const hidl_vec<uint8_t> &wep_key, setWepKey_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setWepKeyInternal, _hidl_cb, key_idx, wep_key);
}

Return<void> StaNetwork::setWepTxKeyIdx(
    uint32_t key_idx, setWepTxKeyIdx_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setWepTxKeyIdxInternal, _hidl_cb, key_idx);
}

Return<void> StaNetwork::setRequirePmf(bool enable, setRequirePmf_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setRequirePmfInternal, _hidl_cb, enable);
}

Return<void> StaNetwork::setEapMethod(
    ISupplicantStaNetwork::EapMethod method, setEapMethod_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapMethodInternal, _hidl_cb, method);
}

Return<void> StaNetwork::setEapPhase2Method(
    ISupplicantStaNetwork::EapPhase2Method method,
    setEapPhase2Method_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapPhase2MethodInternal, _hidl_cb, method);
}

Return<void> StaNetwork::setEapIdentity(
    const hidl_vec<uint8_t> &identity, setEapIdentity_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapIdentityInternal, _hidl_cb, identity);
}

Return<void> StaNetwork::setEapAnonymousIdentity(
    const hidl_vec<uint8_t> &identity, setEapAnonymousIdentity_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapAnonymousIdentityInternal, _hidl_cb, identity);
}

Return<void> StaNetwork::setEapPassword(
    const hidl_vec<uint8_t> &password, setEapPassword_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapPasswordInternal, _hidl_cb, password);
}

Return<void> StaNetwork::setEapCACert(
    const hidl_string &path, setEapCACert_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapCACertInternal, _hidl_cb, path);
}

Return<void> StaNetwork::setEapCAPath(
    const hidl_string &path, setEapCAPath_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapCAPathInternal, _hidl_cb, path);
}

Return<void> StaNetwork::setEapClientCert(
    const hidl_string &path, setEapClientCert_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapClientCertInternal, _hidl_cb, path);
}

Return<void> StaNetwork::setEapPrivateKey(
    const hidl_string &path, setEapPrivateKey_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapPrivateKeyInternal, _hidl_cb, path);
}

Return<void> StaNetwork::setEapSubjectMatch(
    const hidl_string &match, setEapSubjectMatch_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapSubjectMatchInternal, _hidl_cb, match);
}

Return<void> StaNetwork::setEapAltSubjectMatch(
    const hidl_string &match, setEapAltSubjectMatch_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapAltSubjectMatchInternal, _hidl_cb, match);
}

Return<void> StaNetwork::setEapEngine(bool enable, setEapEngine_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapEngineInternal, _hidl_cb, enable);
}

Return<void> StaNetwork::setEapEngineID(
    const hidl_string &id, setEapEngineID_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapEngineIDInternal, _hidl_cb, id);
}

Return<void> StaNetwork::setEapDomainSuffixMatch(
    const hidl_string &match, setEapDomainSuffixMatch_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::setEapDomainSuffixMatchInternal, _hidl_cb, match);
}

Return<void> StaNetwork::getSsid(getSsid_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getSsidInternal, _hidl_cb);
}

Return<void> StaNetwork::getBssid(getBssid_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getBssidInternal, _hidl_cb);
}

Return<void> StaNetwork::getScanSsid(getScanSsid_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getScanSsidInternal, _hidl_cb);
}

Return<void> StaNetwork::getKeyMgmt(getKeyMgmt_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getKeyMgmtInternal, _hidl_cb);
}

Return<void> StaNetwork::getProto(getProto_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getProtoInternal, _hidl_cb);
}

Return<void> StaNetwork::getAuthAlg(getAuthAlg_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getAuthAlgInternal, _hidl_cb);
}

Return<void> StaNetwork::getGroupCipher(getGroupCipher_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getGroupCipherInternal, _hidl_cb);
}

Return<void> StaNetwork::getPairwiseCipher(getPairwiseCipher_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getPairwiseCipherInternal, _hidl_cb);
}

Return<void> StaNetwork::getPskPassphrase(getPskPassphrase_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getPskPassphraseInternal, _hidl_cb);
}

Return<void> StaNetwork::getWepKey(uint32_t key_idx, getWepKey_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getWepKeyInternal, _hidl_cb, key_idx);
}

Return<void> StaNetwork::getWepTxKeyIdx(getWepTxKeyIdx_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getWepTxKeyIdxInternal, _hidl_cb);
}

Return<void> StaNetwork::getRequirePmf(getRequirePmf_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::getRequirePmfInternal, _hidl_cb);
}

Return<void> StaNetwork::enable(bool no_connect, enable_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::enableInternal, _hidl_cb, no_connect);
}

Return<void> StaNetwork::disable(disable_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::disableInternal, _hidl_cb);
}

Return<void> StaNetwork::select(select_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::selectInternal, _hidl_cb);
}

Return<void> StaNetwork::sendNetworkEapSimGsmAuthResponse(
    const ISupplicantStaNetwork::NetworkResponseEapSimGsmAuthParams &params,
    sendNetworkEapSimGsmAuthResponse_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::sendNetworkEapSimGsmAuthResponseInternal, _hidl_cb,
	    params);
}

Return<void> StaNetwork::sendNetworkEapSimUmtsAuthResponse(
    const ISupplicantStaNetwork::NetworkResponseEapSimUmtsAuthParams &params,
    sendNetworkEapSimUmtsAuthResponse_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::sendNetworkEapSimUmtsAuthResponseInternal, _hidl_cb,
	    params);
}

Return<void> StaNetwork::sendNetworkEapIdentityResponse(
    const hidl_vec<uint8_t> &identity,
    sendNetworkEapIdentityResponse_cb _hidl_cb)
{
	return validateAndCall(
	    this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
	    &StaNetwork::sendNetworkEapIdentityResponseInternal, _hidl_cb,
	    identity);
}

std::pair<SupplicantStatus, uint32_t> StaNetwork::getIdInternal()
{
	return {{SupplicantStatusCode::SUCCESS, ""}, network_id_};
}

std::pair<SupplicantStatus, std::string> StaNetwork::getInterfaceNameInternal()
{
	return {{SupplicantStatusCode::SUCCESS, ""}, ifname_};
}

std::pair<SupplicantStatus, IfaceType> StaNetwork::getTypeInternal()
{
	return {{SupplicantStatusCode::SUCCESS, ""}, IfaceType::STA};
}

SupplicantStatus StaNetwork::registerCallbackInternal(
    const sp<ISupplicantStaNetworkCallback> &callback)
{
	HidlManager *hidl_manager = HidlManager::getInstance();
	if (!hidl_manager ||
	    hidl_manager->addStaNetworkCallbackHidlObject(
		ifname_, network_id_, callback)) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setSsidInternal(const std::vector<uint8_t> &ssid)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (ssid.size() == 0 ||
	    ssid.size() >
		static_cast<uint32_t>(ISupplicantStaNetwork::ParamSizeLimits::
					  SSID_MAX_LEN_IN_BYTES)) {
		return {SupplicantStatusCode::FAILURE_ARGS_INVALID, ""};
	}
	if (setByteArrayFieldAndResetState(
		ssid.data(), ssid.size(), &(wpa_ssid->ssid),
		&(wpa_ssid->ssid_len), "ssid")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	if (wpa_ssid->passphrase) {
		wpa_config_update_psk(wpa_ssid);
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setBssidInternal(
    const std::array<uint8_t, 6> &bssid)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	int prev_bssid_set = wpa_ssid->bssid_set;
	u8 prev_bssid[ETH_ALEN];
	os_memcpy(prev_bssid, wpa_ssid->bssid, ETH_ALEN);
	// Zero'ed array is used to clear out the BSSID value.
	if (os_memcmp(bssid.data(), kZeroBssid, ETH_ALEN) == 0) {
		wpa_ssid->bssid_set = 0;
		wpa_printf(MSG_MSGDUMP, "BSSID any");
	} else {
		os_memcpy(wpa_ssid->bssid, bssid.data(), ETH_ALEN);
		wpa_ssid->bssid_set = 1;
		wpa_hexdump(MSG_MSGDUMP, "BSSID", wpa_ssid->bssid, ETH_ALEN);
	}
	struct wpa_supplicant *wpa_s = retrieveIfacePtr();
	if ((wpa_ssid->bssid_set != prev_bssid_set ||
	     os_memcmp(wpa_ssid->bssid, prev_bssid, ETH_ALEN) != 0)) {
		wpas_notify_network_bssid_set_changed(wpa_s, wpa_ssid);
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setScanSsidInternal(bool enable)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	wpa_ssid->scan_ssid = enable ? 1 : 0;
	resetInternalStateAfterParamsUpdate();
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setKeyMgmtInternal(uint32_t key_mgmt_mask)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (key_mgmt_mask & ~kAllowedKeyMgmtMask) {
		return {SupplicantStatusCode::FAILURE_ARGS_INVALID, ""};
	}
	wpa_ssid->key_mgmt = key_mgmt_mask;
	wpa_printf(MSG_MSGDUMP, "key_mgmt: 0x%x", wpa_ssid->key_mgmt);
	resetInternalStateAfterParamsUpdate();
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setProtoInternal(uint32_t proto_mask)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (proto_mask & ~kAllowedproto_mask) {
		return {SupplicantStatusCode::FAILURE_ARGS_INVALID, ""};
	}
	wpa_ssid->proto = proto_mask;
	wpa_printf(MSG_MSGDUMP, "proto: 0x%x", wpa_ssid->proto);
	resetInternalStateAfterParamsUpdate();
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setAuthAlgInternal(uint32_t auth_alg_mask)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (auth_alg_mask & ~kAllowedauth_alg_mask) {
		return {SupplicantStatusCode::FAILURE_ARGS_INVALID, ""};
	}
	wpa_ssid->auth_alg = auth_alg_mask;
	wpa_printf(MSG_MSGDUMP, "auth_alg: 0x%x", wpa_ssid->auth_alg);
	resetInternalStateAfterParamsUpdate();
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setGroupCipherInternal(uint32_t group_cipher_mask)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (group_cipher_mask & ~kAllowedgroup_cipher_mask) {
		return {SupplicantStatusCode::FAILURE_ARGS_INVALID, ""};
	}
	wpa_ssid->group_cipher = group_cipher_mask;
	wpa_printf(MSG_MSGDUMP, "group_cipher: 0x%x", wpa_ssid->group_cipher);
	resetInternalStateAfterParamsUpdate();
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setPairwiseCipherInternal(
    uint32_t pairwise_cipher_mask)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (pairwise_cipher_mask & ~kAllowedpairwise_cipher_mask) {
		return {SupplicantStatusCode::FAILURE_ARGS_INVALID, ""};
	}
	wpa_ssid->pairwise_cipher = pairwise_cipher_mask;
	wpa_printf(
	    MSG_MSGDUMP, "pairwise_cipher: 0x%x", wpa_ssid->pairwise_cipher);
	resetInternalStateAfterParamsUpdate();
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setPskPassphraseInternal(const std::string &psk)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (isPskPassphraseValid(psk)) {
		return {SupplicantStatusCode::FAILURE_ARGS_INVALID, ""};
	}
	if (wpa_ssid->passphrase &&
	    os_strlen(wpa_ssid->passphrase) == psk.size() &&
	    os_memcmp(wpa_ssid->passphrase, psk.c_str(), psk.size()) == 0) {
		return {SupplicantStatusCode::SUCCESS, ""};
	}
	// Flag to indicate if raw psk is calculated or not using
	// |wpa_config_update_psk|. Deferred if ssid not already set.
	wpa_ssid->psk_set = 0;
	if (setStringKeyFieldAndResetState(
		psk.c_str(), &(wpa_ssid->passphrase), "psk passphrase")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	if (wpa_ssid->ssid_len) {
		wpa_config_update_psk(wpa_ssid);
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setWepKeyInternal(
    uint32_t key_idx, const std::vector<uint8_t> &wep_key)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (key_idx >=
	    static_cast<uint32_t>(
		ISupplicantStaNetwork::ParamSizeLimits::WEP_KEYS_MAX_NUM)) {
		return {SupplicantStatusCode::FAILURE_ARGS_INVALID, ""};
	}
	if (wep_key.size() !=
		static_cast<uint32_t>(ISupplicantStaNetwork::ParamSizeLimits::
					  WEP40_KEY_LEN_IN_BYTES) &&
	    wep_key.size() !=
		static_cast<uint32_t>(ISupplicantStaNetwork::ParamSizeLimits::
					  WEP104_KEY_LEN_IN_BYTES)) {
		return {SupplicantStatusCode::FAILURE_ARGS_INVALID, ""};
	}
	os_memcpy(wpa_ssid->wep_key[key_idx], wep_key.data(), wep_key.size());
	wpa_ssid->wep_key_len[key_idx] = wep_key.size();
	std::string msg_dump_title("wep_key" + std::to_string(key_idx));
	wpa_hexdump_key(
	    MSG_MSGDUMP, msg_dump_title.c_str(), wpa_ssid->wep_key[key_idx],
	    wpa_ssid->wep_key_len[key_idx]);
	resetInternalStateAfterParamsUpdate();
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setWepTxKeyIdxInternal(uint32_t key_idx)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (key_idx >=
	    static_cast<uint32_t>(
		ISupplicantStaNetwork::ParamSizeLimits::WEP_KEYS_MAX_NUM)) {
		return {SupplicantStatusCode::FAILURE_ARGS_INVALID, ""};
	}
	wpa_ssid->wep_tx_keyidx = key_idx;
	resetInternalStateAfterParamsUpdate();
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setRequirePmfInternal(bool enable)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	wpa_ssid->ieee80211w =
	    enable ? MGMT_FRAME_PROTECTION_REQUIRED : NO_MGMT_FRAME_PROTECTION;
	resetInternalStateAfterParamsUpdate();
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapMethodInternal(
    ISupplicantStaNetwork::EapMethod method)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	int retrieved_vendor, retrieved_method;
	const char *method_str =
	    kEapMethodStrings[static_cast<uint32_t>(method)];
	// This string lookup is needed to check if the device supports the
	// corresponding EAP type.
	retrieved_method = eap_peer_get_type(method_str, &retrieved_vendor);
	if (retrieved_vendor == EAP_VENDOR_IETF &&
	    retrieved_method == EAP_TYPE_NONE) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	if (wpa_ssid->eap.eap_methods) {
		os_free(wpa_ssid->eap.eap_methods);
	}
	// wpa_supplicant can support setting multiple eap methods for each
	// network. But, this is not really used by Android. So, just adding
	// support for setting one EAP method for each network. The additional
	// |eap_method_type| member in the array is used to indicate the end
	// of list.
	wpa_ssid->eap.eap_methods =
	    (eap_method_type *)os_malloc(sizeof(eap_method_type) * 2);
	if (!wpa_ssid->eap.eap_methods) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	wpa_ssid->eap.eap_methods[0].vendor = retrieved_vendor;
	wpa_ssid->eap.eap_methods[0].method = retrieved_method;
	wpa_ssid->eap.eap_methods[1].vendor = EAP_VENDOR_IETF;
	wpa_ssid->eap.eap_methods[1].method = EAP_TYPE_NONE;

	wpa_ssid->leap = 0;
	wpa_ssid->non_leap = 0;
	if (retrieved_vendor == EAP_VENDOR_IETF &&
	    retrieved_method == EAP_TYPE_LEAP) {
		wpa_ssid->leap++;
	} else {
		wpa_ssid->non_leap++;
	}
	wpa_hexdump(
	    MSG_MSGDUMP, "eap methods", (u8 *)wpa_ssid->eap.eap_methods,
	    sizeof(eap_method_type) * 2);
	resetInternalStateAfterParamsUpdate();
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapPhase2MethodInternal(
    ISupplicantStaNetwork::EapPhase2Method method)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (setStringFieldAndResetState(
		kEapPhase2MethodStrings[static_cast<uint32_t>(method)],
		&(wpa_ssid->eap.phase2), "eap phase2")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapIdentityInternal(
    const std::vector<uint8_t> &identity)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (setByteArrayFieldAndResetState(
		identity.data(), identity.size(), &(wpa_ssid->eap.identity),
		&(wpa_ssid->eap.identity_len), "eap identity")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapAnonymousIdentityInternal(
    const std::vector<uint8_t> &identity)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (setByteArrayFieldAndResetState(
		identity.data(), identity.size(),
		&(wpa_ssid->eap.anonymous_identity),
		&(wpa_ssid->eap.anonymous_identity_len),
		"eap anonymous_identity")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapPasswordInternal(
    const std::vector<uint8_t> &password)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (setByteArrayKeyFieldAndResetState(
		password.data(), password.size(), &(wpa_ssid->eap.password),
		&(wpa_ssid->eap.password_len), "eap password")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	wpa_ssid->eap.flags &= ~EAP_CONFIG_FLAGS_PASSWORD_NTHASH;
	wpa_ssid->eap.flags &= ~EAP_CONFIG_FLAGS_EXT_PASSWORD;
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapCACertInternal(const std::string &path)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (setStringFieldAndResetState(
		path.c_str(), &(wpa_ssid->eap.ca_cert), "eap ca_cert")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapCAPathInternal(const std::string &path)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (setStringFieldAndResetState(
		path.c_str(), &(wpa_ssid->eap.ca_path), "eap ca_path")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapClientCertInternal(const std::string &path)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (setStringFieldAndResetState(
		path.c_str(), &(wpa_ssid->eap.client_cert),
		"eap client_cert")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapPrivateKeyInternal(const std::string &path)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (setStringFieldAndResetState(
		path.c_str(), &(wpa_ssid->eap.private_key),
		"eap private_key")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapSubjectMatchInternal(
    const std::string &match)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (setStringFieldAndResetState(
		match.c_str(), &(wpa_ssid->eap.subject_match),
		"eap subject_match")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapAltSubjectMatchInternal(
    const std::string &match)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (setStringFieldAndResetState(
		match.c_str(), &(wpa_ssid->eap.altsubject_match),
		"eap altsubject_match")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapEngineInternal(bool enable)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	wpa_ssid->eap.engine = enable ? 1 : 0;
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapEngineIDInternal(const std::string &id)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (setStringFieldAndResetState(
		id.c_str(), &(wpa_ssid->eap.engine_id), "eap engine_id")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::setEapDomainSuffixMatchInternal(
    const std::string &match)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (setStringFieldAndResetState(
		match.c_str(), &(wpa_ssid->eap.domain_suffix_match),
		"eap domain_suffix_match")) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

std::pair<SupplicantStatus, std::vector<uint8_t>> StaNetwork::getSsidInternal()
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	std::vector<uint8_t> ssid;
	ssid.assign(wpa_ssid->ssid, wpa_ssid->ssid + wpa_ssid->ssid_len);
	return {{SupplicantStatusCode::SUCCESS, ""}, std::move(ssid)};
}

std::pair<SupplicantStatus, std::array<uint8_t, 6>>
StaNetwork::getBssidInternal()
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	std::array<uint8_t, 6> bssid{};
	os_memcpy(bssid.data(), kZeroBssid, ETH_ALEN);
	if (wpa_ssid->bssid_set) {
		os_memcpy(bssid.data(), wpa_ssid->bssid, ETH_ALEN);
	}
	return {{SupplicantStatusCode::SUCCESS, ""}, std::move(bssid)};
}

std::pair<SupplicantStatus, bool> StaNetwork::getScanSsidInternal()
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	return {{SupplicantStatusCode::SUCCESS, ""},
		(wpa_ssid->scan_ssid == 1)};
}

std::pair<SupplicantStatus, uint32_t> StaNetwork::getKeyMgmtInternal()
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	return {{SupplicantStatusCode::SUCCESS, ""}, wpa_ssid->key_mgmt};
}

std::pair<SupplicantStatus, uint32_t> StaNetwork::getProtoInternal()
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	return {{SupplicantStatusCode::SUCCESS, ""}, wpa_ssid->proto};
}

std::pair<SupplicantStatus, uint32_t> StaNetwork::getAuthAlgInternal()
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	return {{SupplicantStatusCode::SUCCESS, ""}, wpa_ssid->auth_alg};
}

std::pair<SupplicantStatus, uint32_t> StaNetwork::getGroupCipherInternal()
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	return {{SupplicantStatusCode::SUCCESS, ""}, wpa_ssid->group_cipher};
}

std::pair<SupplicantStatus, uint32_t> StaNetwork::getPairwiseCipherInternal()
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	return {{SupplicantStatusCode::SUCCESS, ""}, wpa_ssid->pairwise_cipher};
}

std::pair<SupplicantStatus, std::string> StaNetwork::getPskPassphraseInternal()
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	std::string psk;
	if (wpa_ssid->passphrase) {
		psk = wpa_ssid->passphrase;
	}
	return {{SupplicantStatusCode::SUCCESS, ""}, std::move(psk)};
}

std::pair<SupplicantStatus, std::vector<uint8_t>> StaNetwork::getWepKeyInternal(
    uint32_t key_idx)
{
	std::vector<uint8_t> wep_key;
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (key_idx >=
	    static_cast<uint32_t>(
		ISupplicantStaNetwork::ParamSizeLimits::WEP_KEYS_MAX_NUM)) {
		return {{SupplicantStatusCode::FAILURE_ARGS_INVALID, ""},
			wep_key};
	}
	wep_key.assign(
	    wpa_ssid->wep_key[key_idx],
	    wpa_ssid->wep_key[key_idx] + wpa_ssid->wep_key_len[key_idx]);
	return {{SupplicantStatusCode::SUCCESS, ""}, std::move(wep_key)};
}

std::pair<SupplicantStatus, uint32_t> StaNetwork::getWepTxKeyIdxInternal()
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	return {{SupplicantStatusCode::SUCCESS, ""}, wpa_ssid->wep_tx_keyidx};
}

std::pair<SupplicantStatus, bool> StaNetwork::getRequirePmfInternal()
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	return {{SupplicantStatusCode::SUCCESS, ""},
		(wpa_ssid->ieee80211w == MGMT_FRAME_PROTECTION_REQUIRED)};
}

SupplicantStatus StaNetwork::enableInternal(bool no_connect)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (wpa_ssid->disabled != 0) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	struct wpa_supplicant *wpa_s = retrieveIfacePtr();
	if (no_connect) {
		wpa_ssid->disabled = 0;
	} else {
		wpa_s->scan_min_time.sec = 0;
		wpa_s->scan_min_time.usec = 0;
		wpa_supplicant_enable_network(wpa_s, wpa_ssid);
	}
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::disableInternal()
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (wpa_ssid->disabled == 2) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	struct wpa_supplicant *wpa_s = retrieveIfacePtr();
	wpa_supplicant_disable_network(wpa_s, wpa_ssid);
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::selectInternal()
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	if (wpa_ssid->disabled == 2) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	struct wpa_supplicant *wpa_s = retrieveIfacePtr();
	wpa_s->scan_min_time.sec = 0;
	wpa_s->scan_min_time.usec = 0;
	wpa_supplicant_select_network(wpa_s, wpa_ssid);
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::sendNetworkEapSimGsmAuthResponseInternal(
    const ISupplicantStaNetwork::NetworkResponseEapSimGsmAuthParams &params)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	// Convert the incoming parameters to a string to pass to
	// wpa_supplicant.
	std::string ctrl_rsp_param;
	uint32_t kc_hex_len = params.kc.size() * 2 + 1;
	char *kc_hex = (char *)malloc(kc_hex_len);
	uint32_t sres_hex_len = params.sres.size() * 2 + 1;
	char *sres_hex = (char *)malloc(sres_hex_len);
	if (!kc_hex || !sres_hex) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	wpa_snprintf_hex(
	    kc_hex, kc_hex_len, params.kc.data(), params.kc.size());
	wpa_snprintf_hex(
	    sres_hex, sres_hex_len, params.sres.data(), params.sres.size());
	ctrl_rsp_param += "kc:";
	ctrl_rsp_param += kc_hex;
	ctrl_rsp_param += " sres:";
	ctrl_rsp_param += sres_hex;
	enum wpa_ctrl_req_type rtype = WPA_CTRL_REQ_SIM;
	struct wpa_supplicant *wpa_s = retrieveIfacePtr();
	if (wpa_supplicant_ctrl_rsp_handle(
		wpa_s, wpa_ssid, rtype, ctrl_rsp_param.c_str())) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	eapol_sm_notify_ctrl_response(wpa_s->eapol);
	wpa_hexdump_ascii_key(
	    MSG_DEBUG, "network sim gsm auth response param",
	    (const u8 *)ctrl_rsp_param.c_str(), ctrl_rsp_param.size());
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::sendNetworkEapSimUmtsAuthResponseInternal(
    const ISupplicantStaNetwork::NetworkResponseEapSimUmtsAuthParams &params)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	// Convert the incoming parameters to a string to pass to
	// wpa_supplicant.
	std::string ctrl_rsp_param;
	uint32_t ik_hex_len = params.ik.size() * 2 + 1;
	char *ik_hex = (char *)malloc(ik_hex_len);
	uint32_t ck_hex_len = params.ck.size() * 2 + 1;
	char *ck_hex = (char *)malloc(ck_hex_len);
	uint32_t res_hex_len = params.res.size() * 2 + 1;
	char *res_hex = (char *)malloc(res_hex_len);
	if (!ik_hex || !ck_hex || !res_hex) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	wpa_snprintf_hex(
	    ik_hex, ik_hex_len, params.ik.data(), params.ik.size());
	wpa_snprintf_hex(
	    ck_hex, ck_hex_len, params.ck.data(), params.ck.size());
	wpa_snprintf_hex(
	    res_hex, res_hex_len, params.res.data(), params.res.size());
	ctrl_rsp_param += "ik:";
	ctrl_rsp_param += ik_hex;
	ctrl_rsp_param += "ck:";
	ctrl_rsp_param += ck_hex;
	ctrl_rsp_param += " res:";
	ctrl_rsp_param += res_hex;
	enum wpa_ctrl_req_type rtype = WPA_CTRL_REQ_SIM;
	struct wpa_supplicant *wpa_s = retrieveIfacePtr();
	if (wpa_supplicant_ctrl_rsp_handle(
		wpa_s, wpa_ssid, rtype, ctrl_rsp_param.c_str())) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	eapol_sm_notify_ctrl_response(wpa_s->eapol);
	wpa_hexdump_ascii_key(
	    MSG_DEBUG, "network sim umts auth response param",
	    (const u8 *)ctrl_rsp_param.c_str(), ctrl_rsp_param.size());
	return {SupplicantStatusCode::SUCCESS, ""};
}

SupplicantStatus StaNetwork::sendNetworkEapIdentityResponseInternal(
    const std::vector<uint8_t> &identity)
{
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
	// Convert the incoming parameters to a string to pass to
	// wpa_supplicant.
	uint32_t identity_hex_len = identity.size() * 2 + 1;
	char *identity_hex = (char *)malloc(identity_hex_len);
	std::string ctrl_rsp_param;
	if (!identity_hex) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	wpa_snprintf_hex(
	    identity_hex, identity_hex_len, identity.data(), identity.size());
	ctrl_rsp_param = identity_hex;
	enum wpa_ctrl_req_type rtype = WPA_CTRL_REQ_EAP_IDENTITY;
	struct wpa_supplicant *wpa_s = retrieveIfacePtr();
	if (wpa_supplicant_ctrl_rsp_handle(
		wpa_s, wpa_ssid, rtype, ctrl_rsp_param.c_str())) {
		return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
	}
	eapol_sm_notify_ctrl_response(wpa_s->eapol);
	wpa_hexdump_ascii_key(
	    MSG_DEBUG, "network identity response param",
	    (const u8 *)ctrl_rsp_param.c_str(), ctrl_rsp_param.size());
	return {SupplicantStatusCode::SUCCESS, ""};
}

/**
 * Retrieve the underlying |wpa_ssid| struct pointer for
 * this network.
 * If the underlying network is removed or the interface
 * this network belong to
 * is removed, all RPC method calls on this object will
 * return failure.
 */
struct wpa_ssid *StaNetwork::retrieveNetworkPtr()
{
	wpa_supplicant *wpa_s = retrieveIfacePtr();
	if (!wpa_s)
		return nullptr;
	return wpa_config_get_network(wpa_s->conf, network_id_);
}

/**
 * Retrieve the underlying |wpa_supplicant| struct
 * pointer for
 * this network.
 */
struct wpa_supplicant *StaNetwork::retrieveIfacePtr()
{
	return wpa_supplicant_get_iface(
	    (struct wpa_global *)wpa_global_, ifname_.c_str());
}

/**
 * Check if the provided psk passhrase is valid or not.
 *
 * Returns 0 if valid, 1 otherwise.
 */
int StaNetwork::isPskPassphraseValid(const std::string &psk)
{
	if (psk.size() <
		static_cast<uint32_t>(ISupplicantStaNetwork::ParamSizeLimits::
					  PSK_PASSPHRASE_MIN_LEN_IN_BYTES) ||
	    psk.size() >
		static_cast<uint32_t>(ISupplicantStaNetwork::ParamSizeLimits::
					  PSK_PASSPHRASE_MAX_LEN_IN_BYTES)) {
		return 1;
	}
	if (has_ctrl_char((u8 *)psk.c_str(), psk.size())) {
		return 1;
	}
	return 0;
}

/**
 * Reset internal wpa_supplicant state machine state
 * after params update (except
 * bssid).
 */
void StaNetwork::resetInternalStateAfterParamsUpdate()
{
	struct wpa_supplicant *wpa_s = retrieveIfacePtr();
	struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();

	wpa_sm_pmksa_cache_flush(wpa_s->wpa, wpa_ssid);

	if (wpa_s->current_ssid == wpa_ssid || wpa_s->current_ssid == NULL) {
		/*
		 * Invalidate the EAP session cache if
		 * anything in the
		 * current or previously used
		 * configuration changes.
		 */
		eapol_sm_invalidate_cached_session(wpa_s->eapol);
	}
}

/**
 * Helper function to set value in a string field in |wpa_ssid| structue
 * instance for this network.
 * This function frees any existing data in these fields.
 */
int StaNetwork::setStringFieldAndResetState(
    const char *value, uint8_t **to_update_field, const char *hexdump_prefix)
{
	return setStringFieldAndResetState(
	    value, (char **)to_update_field, hexdump_prefix);
}

/**
 * Helper function to set value in a string field in |wpa_ssid| structue
 * instance for this network.
 * This function frees any existing data in these fields.
 */
int StaNetwork::setStringFieldAndResetState(
    const char *value, char **to_update_field, const char *hexdump_prefix)
{
	int value_len = strlen(value);
	if (*to_update_field) {
		os_free(*to_update_field);
	}
	*to_update_field = dup_binstr(value, value_len);
	if (!(*to_update_field)) {
		return 1;
	}
	wpa_hexdump_ascii(
	    MSG_MSGDUMP, hexdump_prefix, *to_update_field, value_len);
	resetInternalStateAfterParamsUpdate();
	return 0;
}

/**
 * Helper function to set value in a string key field in |wpa_ssid| structue
 * instance for this network.
 * This function frees any existing data in these fields.
 */
int StaNetwork::setStringKeyFieldAndResetState(
    const char *value, char **to_update_field, const char *hexdump_prefix)
{
	int value_len = strlen(value);
	if (*to_update_field) {
		str_clear_free(*to_update_field);
	}
	*to_update_field = dup_binstr(value, value_len);
	if (!(*to_update_field)) {
		return 1;
	}
	wpa_hexdump_ascii_key(
	    MSG_MSGDUMP, hexdump_prefix, *to_update_field, value_len);
	resetInternalStateAfterParamsUpdate();
	return 0;
}

/**
 * Helper function to set value in a string field with a corresponding length
 * field in |wpa_ssid| structue instance for this network.
 * This function frees any existing data in these fields.
 */
int StaNetwork::setByteArrayFieldAndResetState(
    const uint8_t *value, const size_t value_len, uint8_t **to_update_field,
    size_t *to_update_field_len, const char *hexdump_prefix)
{
	if (*to_update_field) {
		os_free(*to_update_field);
	}
	*to_update_field = (uint8_t *)os_malloc(value_len);
	if (!(*to_update_field)) {
		return 1;
	}
	os_memcpy(*to_update_field, value, value_len);
	*to_update_field_len = value_len;

	wpa_hexdump_ascii(
	    MSG_MSGDUMP, hexdump_prefix, *to_update_field,
	    *to_update_field_len);
	resetInternalStateAfterParamsUpdate();
	return 0;
}

/**
 * Helper function to set value in a string key field with a corresponding
 * length field in |wpa_ssid| structue instance for this network.
 * This function frees any existing data in these fields.
 */
int StaNetwork::setByteArrayKeyFieldAndResetState(
    const uint8_t *value, const size_t value_len, uint8_t **to_update_field,
    size_t *to_update_field_len, const char *hexdump_prefix)
{
	if (*to_update_field) {
		bin_clear_free(*to_update_field, *to_update_field_len);
	}
	*to_update_field = (uint8_t *)os_malloc(value_len);
	if (!(*to_update_field)) {
		return 1;
	}
	os_memcpy(*to_update_field, value, value_len);
	*to_update_field_len = value_len;

	wpa_hexdump_ascii_key(
	    MSG_MSGDUMP, hexdump_prefix, *to_update_field,
	    *to_update_field_len);
	resetInternalStateAfterParamsUpdate();
	return 0;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace wifi
}  // namespace supplicant
}  // namespace hardware
}  // namespace android