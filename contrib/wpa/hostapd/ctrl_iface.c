/*
 * hostapd / UNIX domain socket -based control interface
 * Copyright (c) 2004-2015, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"

#ifndef CONFIG_NATIVE_WINDOWS

#ifdef CONFIG_TESTING_OPTIONS
#include <net/ethernet.h>
#include <netinet/ip.h>
#endif /* CONFIG_TESTING_OPTIONS */

#include <sys/un.h>
#include <sys/stat.h>
#include <stddef.h>

#include "utils/common.h"
#include "utils/eloop.h"
#include "common/version.h"
#include "common/ieee802_11_defs.h"
#include "crypto/tls.h"
#include "drivers/driver.h"
#include "eapol_auth/eapol_auth_sm.h"
#include "radius/radius_client.h"
#include "radius/radius_server.h"
#include "l2_packet/l2_packet.h"
#include "ap/hostapd.h"
#include "ap/ap_config.h"
#include "ap/ieee802_1x.h"
#include "ap/wpa_auth.h"
#include "ap/ieee802_11.h"
#include "ap/sta_info.h"
#include "ap/wps_hostapd.h"
#include "ap/ctrl_iface_ap.h"
#include "ap/ap_drv_ops.h"
#include "ap/hs20.h"
#include "ap/wnm_ap.h"
#include "ap/wpa_auth.h"
#include "ap/beacon.h"
#include "wps/wps_defs.h"
#include "wps/wps.h"
#include "fst/fst_ctrl_iface.h"
#include "config_file.h"
#include "ctrl_iface.h"


#define HOSTAPD_CLI_DUP_VALUE_MAX_LEN 256

struct wpa_ctrl_dst {
	struct wpa_ctrl_dst *next;
	struct sockaddr_un addr;
	socklen_t addrlen;
	int debug_level;
	int errors;
};


static void hostapd_ctrl_iface_send(struct hostapd_data *hapd, int level,
				    enum wpa_msg_type type,
				    const char *buf, size_t len);


static int hostapd_ctrl_iface_attach(struct hostapd_data *hapd,
				     struct sockaddr_un *from,
				     socklen_t fromlen)
{
	struct wpa_ctrl_dst *dst;

	dst = os_zalloc(sizeof(*dst));
	if (dst == NULL)
		return -1;
	os_memcpy(&dst->addr, from, sizeof(struct sockaddr_un));
	dst->addrlen = fromlen;
	dst->debug_level = MSG_INFO;
	dst->next = hapd->ctrl_dst;
	hapd->ctrl_dst = dst;
	wpa_hexdump(MSG_DEBUG, "CTRL_IFACE monitor attached",
		    (u8 *) from->sun_path,
		    fromlen - offsetof(struct sockaddr_un, sun_path));
	return 0;
}


static int hostapd_ctrl_iface_detach(struct hostapd_data *hapd,
				     struct sockaddr_un *from,
				     socklen_t fromlen)
{
	struct wpa_ctrl_dst *dst, *prev = NULL;

	dst = hapd->ctrl_dst;
	while (dst) {
		if (fromlen == dst->addrlen &&
		    os_memcmp(from->sun_path, dst->addr.sun_path,
			      fromlen - offsetof(struct sockaddr_un, sun_path))
		    == 0) {
			wpa_hexdump(MSG_DEBUG, "CTRL_IFACE monitor detached",
				    (u8 *) from->sun_path,
				    fromlen -
				    offsetof(struct sockaddr_un, sun_path));
			if (prev == NULL)
				hapd->ctrl_dst = dst->next;
			else
				prev->next = dst->next;
			os_free(dst);
			return 0;
		}
		prev = dst;
		dst = dst->next;
	}
	return -1;
}


static int hostapd_ctrl_iface_level(struct hostapd_data *hapd,
				    struct sockaddr_un *from,
				    socklen_t fromlen,
				    char *level)
{
	struct wpa_ctrl_dst *dst;

	wpa_printf(MSG_DEBUG, "CTRL_IFACE LEVEL %s", level);

	dst = hapd->ctrl_dst;
	while (dst) {
		if (fromlen == dst->addrlen &&
		    os_memcmp(from->sun_path, dst->addr.sun_path,
			      fromlen - offsetof(struct sockaddr_un, sun_path))
		    == 0) {
			wpa_hexdump(MSG_DEBUG, "CTRL_IFACE changed monitor "
				    "level", (u8 *) from->sun_path, fromlen -
				    offsetof(struct sockaddr_un, sun_path));
			dst->debug_level = atoi(level);
			return 0;
		}
		dst = dst->next;
	}

	return -1;
}


static int hostapd_ctrl_iface_new_sta(struct hostapd_data *hapd,
				      const char *txtaddr)
{
	u8 addr[ETH_ALEN];
	struct sta_info *sta;

	wpa_printf(MSG_DEBUG, "CTRL_IFACE NEW_STA %s", txtaddr);

	if (hwaddr_aton(txtaddr, addr))
		return -1;

	sta = ap_get_sta(hapd, addr);
	if (sta)
		return 0;

	wpa_printf(MSG_DEBUG, "Add new STA " MACSTR " based on ctrl_iface "
		   "notification", MAC2STR(addr));
	sta = ap_sta_add(hapd, addr);
	if (sta == NULL)
		return -1;

	hostapd_new_assoc_sta(hapd, sta, 0);
	return 0;
}


#ifdef CONFIG_IEEE80211W
#ifdef NEED_AP_MLME
static int hostapd_ctrl_iface_sa_query(struct hostapd_data *hapd,
				       const char *txtaddr)
{
	u8 addr[ETH_ALEN];
	u8 trans_id[WLAN_SA_QUERY_TR_ID_LEN];

	wpa_printf(MSG_DEBUG, "CTRL_IFACE SA_QUERY %s", txtaddr);

	if (hwaddr_aton(txtaddr, addr) ||
	    os_get_random(trans_id, WLAN_SA_QUERY_TR_ID_LEN) < 0)
		return -1;

	ieee802_11_send_sa_query_req(hapd, addr, trans_id);

	return 0;
}
#endif /* NEED_AP_MLME */
#endif /* CONFIG_IEEE80211W */


#ifdef CONFIG_WPS
static int hostapd_ctrl_iface_wps_pin(struct hostapd_data *hapd, char *txt)
{
	char *pin = os_strchr(txt, ' ');
	char *timeout_txt;
	int timeout;
	u8 addr_buf[ETH_ALEN], *addr = NULL;
	char *pos;

	if (pin == NULL)
		return -1;
	*pin++ = '\0';

	timeout_txt = os_strchr(pin, ' ');
	if (timeout_txt) {
		*timeout_txt++ = '\0';
		timeout = atoi(timeout_txt);
		pos = os_strchr(timeout_txt, ' ');
		if (pos) {
			*pos++ = '\0';
			if (hwaddr_aton(pos, addr_buf) == 0)
				addr = addr_buf;
		}
	} else
		timeout = 0;

	return hostapd_wps_add_pin(hapd, addr, txt, pin, timeout);
}


static int hostapd_ctrl_iface_wps_check_pin(
	struct hostapd_data *hapd, char *cmd, char *buf, size_t buflen)
{
	char pin[9];
	size_t len;
	char *pos;
	int ret;

	wpa_hexdump_ascii_key(MSG_DEBUG, "WPS_CHECK_PIN",
			      (u8 *) cmd, os_strlen(cmd));
	for (pos = cmd, len = 0; *pos != '\0'; pos++) {
		if (*pos < '0' || *pos > '9')
			continue;
		pin[len++] = *pos;
		if (len == 9) {
			wpa_printf(MSG_DEBUG, "WPS: Too long PIN");
			return -1;
		}
	}
	if (len != 4 && len != 8) {
		wpa_printf(MSG_DEBUG, "WPS: Invalid PIN length %d", (int) len);
		return -1;
	}
	pin[len] = '\0';

	if (len == 8) {
		unsigned int pin_val;
		pin_val = atoi(pin);
		if (!wps_pin_valid(pin_val)) {
			wpa_printf(MSG_DEBUG, "WPS: Invalid checksum digit");
			ret = os_snprintf(buf, buflen, "FAIL-CHECKSUM\n");
			if (os_snprintf_error(buflen, ret))
				return -1;
			return ret;
		}
	}

	ret = os_snprintf(buf, buflen, "%s", pin);
	if (os_snprintf_error(buflen, ret))
		return -1;

	return ret;
}


#ifdef CONFIG_WPS_NFC
static int hostapd_ctrl_iface_wps_nfc_tag_read(struct hostapd_data *hapd,
					       char *pos)
{
	size_t len;
	struct wpabuf *buf;
	int ret;

	len = os_strlen(pos);
	if (len & 0x01)
		return -1;
	len /= 2;

	buf = wpabuf_alloc(len);
	if (buf == NULL)
		return -1;
	if (hexstr2bin(pos, wpabuf_put(buf, len), len) < 0) {
		wpabuf_free(buf);
		return -1;
	}

	ret = hostapd_wps_nfc_tag_read(hapd, buf);
	wpabuf_free(buf);

	return ret;
}


static int hostapd_ctrl_iface_wps_nfc_config_token(struct hostapd_data *hapd,
						   char *cmd, char *reply,
						   size_t max_len)
{
	int ndef;
	struct wpabuf *buf;
	int res;

	if (os_strcmp(cmd, "WPS") == 0)
		ndef = 0;
	else if (os_strcmp(cmd, "NDEF") == 0)
		ndef = 1;
	else
		return -1;

	buf = hostapd_wps_nfc_config_token(hapd, ndef);
	if (buf == NULL)
		return -1;

	res = wpa_snprintf_hex_uppercase(reply, max_len, wpabuf_head(buf),
					 wpabuf_len(buf));
	reply[res++] = '\n';
	reply[res] = '\0';

	wpabuf_free(buf);

	return res;
}


static int hostapd_ctrl_iface_wps_nfc_token_gen(struct hostapd_data *hapd,
						char *reply, size_t max_len,
						int ndef)
{
	struct wpabuf *buf;
	int res;

	buf = hostapd_wps_nfc_token_gen(hapd, ndef);
	if (buf == NULL)
		return -1;

	res = wpa_snprintf_hex_uppercase(reply, max_len, wpabuf_head(buf),
					 wpabuf_len(buf));
	reply[res++] = '\n';
	reply[res] = '\0';

	wpabuf_free(buf);

	return res;
}


static int hostapd_ctrl_iface_wps_nfc_token(struct hostapd_data *hapd,
					    char *cmd, char *reply,
					    size_t max_len)
{
	if (os_strcmp(cmd, "WPS") == 0)
		return hostapd_ctrl_iface_wps_nfc_token_gen(hapd, reply,
							    max_len, 0);

	if (os_strcmp(cmd, "NDEF") == 0)
		return hostapd_ctrl_iface_wps_nfc_token_gen(hapd, reply,
							    max_len, 1);

	if (os_strcmp(cmd, "enable") == 0)
		return hostapd_wps_nfc_token_enable(hapd);

	if (os_strcmp(cmd, "disable") == 0) {
		hostapd_wps_nfc_token_disable(hapd);
		return 0;
	}

	return -1;
}


static int hostapd_ctrl_iface_nfc_get_handover_sel(struct hostapd_data *hapd,
						   char *cmd, char *reply,
						   size_t max_len)
{
	struct wpabuf *buf;
	int res;
	char *pos;
	int ndef;

	pos = os_strchr(cmd, ' ');
	if (pos == NULL)
		return -1;
	*pos++ = '\0';

	if (os_strcmp(cmd, "WPS") == 0)
		ndef = 0;
	else if (os_strcmp(cmd, "NDEF") == 0)
		ndef = 1;
	else
		return -1;

	if (os_strcmp(pos, "WPS-CR") == 0)
		buf = hostapd_wps_nfc_hs_cr(hapd, ndef);
	else
		buf = NULL;
	if (buf == NULL)
		return -1;

	res = wpa_snprintf_hex_uppercase(reply, max_len, wpabuf_head(buf),
					 wpabuf_len(buf));
	reply[res++] = '\n';
	reply[res] = '\0';

	wpabuf_free(buf);

	return res;
}


static int hostapd_ctrl_iface_nfc_report_handover(struct hostapd_data *hapd,
						  char *cmd)
{
	size_t len;
	struct wpabuf *req, *sel;
	int ret;
	char *pos, *role, *type, *pos2;

	role = cmd;
	pos = os_strchr(role, ' ');
	if (pos == NULL)
		return -1;
	*pos++ = '\0';

	type = pos;
	pos = os_strchr(type, ' ');
	if (pos == NULL)
		return -1;
	*pos++ = '\0';

	pos2 = os_strchr(pos, ' ');
	if (pos2 == NULL)
		return -1;
	*pos2++ = '\0';

	len = os_strlen(pos);
	if (len & 0x01)
		return -1;
	len /= 2;

	req = wpabuf_alloc(len);
	if (req == NULL)
		return -1;
	if (hexstr2bin(pos, wpabuf_put(req, len), len) < 0) {
		wpabuf_free(req);
		return -1;
	}

	len = os_strlen(pos2);
	if (len & 0x01) {
		wpabuf_free(req);
		return -1;
	}
	len /= 2;

	sel = wpabuf_alloc(len);
	if (sel == NULL) {
		wpabuf_free(req);
		return -1;
	}
	if (hexstr2bin(pos2, wpabuf_put(sel, len), len) < 0) {
		wpabuf_free(req);
		wpabuf_free(sel);
		return -1;
	}

	if (os_strcmp(role, "RESP") == 0 && os_strcmp(type, "WPS") == 0) {
		ret = hostapd_wps_nfc_report_handover(hapd, req, sel);
	} else {
		wpa_printf(MSG_DEBUG, "NFC: Unsupported connection handover "
			   "reported: role=%s type=%s", role, type);
		ret = -1;
	}
	wpabuf_free(req);
	wpabuf_free(sel);

	return ret;
}

#endif /* CONFIG_WPS_NFC */


static int hostapd_ctrl_iface_wps_ap_pin(struct hostapd_data *hapd, char *txt,
					 char *buf, size_t buflen)
{
	int timeout = 300;
	char *pos;
	const char *pin_txt;

	pos = os_strchr(txt, ' ');
	if (pos)
		*pos++ = '\0';

	if (os_strcmp(txt, "disable") == 0) {
		hostapd_wps_ap_pin_disable(hapd);
		return os_snprintf(buf, buflen, "OK\n");
	}

	if (os_strcmp(txt, "random") == 0) {
		if (pos)
			timeout = atoi(pos);
		pin_txt = hostapd_wps_ap_pin_random(hapd, timeout);
		if (pin_txt == NULL)
			return -1;
		return os_snprintf(buf, buflen, "%s", pin_txt);
	}

	if (os_strcmp(txt, "get") == 0) {
		pin_txt = hostapd_wps_ap_pin_get(hapd);
		if (pin_txt == NULL)
			return -1;
		return os_snprintf(buf, buflen, "%s", pin_txt);
	}

	if (os_strcmp(txt, "set") == 0) {
		char *pin;
		if (pos == NULL)
			return -1;
		pin = pos;
		pos = os_strchr(pos, ' ');
		if (pos) {
			*pos++ = '\0';
			timeout = atoi(pos);
		}
		if (os_strlen(pin) > buflen)
			return -1;
		if (hostapd_wps_ap_pin_set(hapd, pin, timeout) < 0)
			return -1;
		return os_snprintf(buf, buflen, "%s", pin);
	}

	return -1;
}


static int hostapd_ctrl_iface_wps_config(struct hostapd_data *hapd, char *txt)
{
	char *pos;
	char *ssid, *auth, *encr = NULL, *key = NULL;

	ssid = txt;
	pos = os_strchr(txt, ' ');
	if (!pos)
		return -1;
	*pos++ = '\0';

	auth = pos;
	pos = os_strchr(pos, ' ');
	if (pos) {
		*pos++ = '\0';
		encr = pos;
		pos = os_strchr(pos, ' ');
		if (pos) {
			*pos++ = '\0';
			key = pos;
		}
	}

	return hostapd_wps_config_ap(hapd, ssid, auth, encr, key);
}


static const char * pbc_status_str(enum pbc_status status)
{
	switch (status) {
	case WPS_PBC_STATUS_DISABLE:
		return "Disabled";
	case WPS_PBC_STATUS_ACTIVE:
		return "Active";
	case WPS_PBC_STATUS_TIMEOUT:
		return "Timed-out";
	case WPS_PBC_STATUS_OVERLAP:
		return "Overlap";
	default:
		return "Unknown";
	}
}


static int hostapd_ctrl_iface_wps_get_status(struct hostapd_data *hapd,
					     char *buf, size_t buflen)
{
	int ret;
	char *pos, *end;

	pos = buf;
	end = buf + buflen;

	ret = os_snprintf(pos, end - pos, "PBC Status: %s\n",
			  pbc_status_str(hapd->wps_stats.pbc_status));

	if (os_snprintf_error(end - pos, ret))
		return pos - buf;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "Last WPS result: %s\n",
			  (hapd->wps_stats.status == WPS_STATUS_SUCCESS ?
			   "Success":
			   (hapd->wps_stats.status == WPS_STATUS_FAILURE ?
			    "Failed" : "None")));

	if (os_snprintf_error(end - pos, ret))
		return pos - buf;
	pos += ret;

	/* If status == Failure - Add possible Reasons */
	if(hapd->wps_stats.status == WPS_STATUS_FAILURE &&
	   hapd->wps_stats.failure_reason > 0) {
		ret = os_snprintf(pos, end - pos,
				  "Failure Reason: %s\n",
				  wps_ei_str(hapd->wps_stats.failure_reason));

		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}

	if (hapd->wps_stats.status) {
		ret = os_snprintf(pos, end - pos, "Peer Address: " MACSTR "\n",
				  MAC2STR(hapd->wps_stats.peer_addr));

		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}

	return pos - buf;
}

#endif /* CONFIG_WPS */

#ifdef CONFIG_HS20

static int hostapd_ctrl_iface_hs20_wnm_notif(struct hostapd_data *hapd,
					     const char *cmd)
{
	u8 addr[ETH_ALEN];
	const char *url;

	if (hwaddr_aton(cmd, addr))
		return -1;
	url = cmd + 17;
	if (*url == '\0') {
		url = NULL;
	} else {
		if (*url != ' ')
			return -1;
		url++;
		if (*url == '\0')
			url = NULL;
	}

	return hs20_send_wnm_notification(hapd, addr, 1, url);
}


static int hostapd_ctrl_iface_hs20_deauth_req(struct hostapd_data *hapd,
					      const char *cmd)
{
	u8 addr[ETH_ALEN];
	int code, reauth_delay, ret;
	const char *pos;
	size_t url_len;
	struct wpabuf *req;

	/* <STA MAC Addr> <Code(0/1)> <Re-auth-Delay(sec)> [URL] */
	if (hwaddr_aton(cmd, addr))
		return -1;

	pos = os_strchr(cmd, ' ');
	if (pos == NULL)
		return -1;
	pos++;
	code = atoi(pos);

	pos = os_strchr(pos, ' ');
	if (pos == NULL)
		return -1;
	pos++;
	reauth_delay = atoi(pos);

	url_len = 0;
	pos = os_strchr(pos, ' ');
	if (pos) {
		pos++;
		url_len = os_strlen(pos);
	}

	req = wpabuf_alloc(4 + url_len);
	if (req == NULL)
		return -1;
	wpabuf_put_u8(req, code);
	wpabuf_put_le16(req, reauth_delay);
	wpabuf_put_u8(req, url_len);
	if (pos)
		wpabuf_put_data(req, pos, url_len);

	wpa_printf(MSG_DEBUG, "HS 2.0: Send WNM-Notification to " MACSTR
		   " to indicate imminent deauthentication (code=%d "
		   "reauth_delay=%d)", MAC2STR(addr), code, reauth_delay);
	ret = hs20_send_wnm_notification_deauth_req(hapd, addr, req);
	wpabuf_free(req);
	return ret;
}

#endif /* CONFIG_HS20 */


#ifdef CONFIG_INTERWORKING

static int hostapd_ctrl_iface_set_qos_map_set(struct hostapd_data *hapd,
					      const char *cmd)
{
	u8 qos_map_set[16 + 2 * 21], count = 0;
	const char *pos = cmd;
	int val, ret;

	for (;;) {
		if (count == sizeof(qos_map_set)) {
			wpa_printf(MSG_ERROR, "Too many qos_map_set parameters");
			return -1;
		}

		val = atoi(pos);
		if (val < 0 || val > 255) {
			wpa_printf(MSG_INFO, "Invalid QoS Map Set");
			return -1;
		}

		qos_map_set[count++] = val;
		pos = os_strchr(pos, ',');
		if (!pos)
			break;
		pos++;
	}

	if (count < 16 || count & 1) {
		wpa_printf(MSG_INFO, "Invalid QoS Map Set");
		return -1;
	}

	ret = hostapd_drv_set_qos_map(hapd, qos_map_set, count);
	if (ret) {
		wpa_printf(MSG_INFO, "Failed to set QoS Map Set");
		return -1;
	}

	os_memcpy(hapd->conf->qos_map_set, qos_map_set, count);
	hapd->conf->qos_map_set_len = count;

	return 0;
}


static int hostapd_ctrl_iface_send_qos_map_conf(struct hostapd_data *hapd,
						const char *cmd)
{
	u8 addr[ETH_ALEN];
	struct sta_info *sta;
	struct wpabuf *buf;
	u8 *qos_map_set = hapd->conf->qos_map_set;
	u8 qos_map_set_len = hapd->conf->qos_map_set_len;
	int ret;

	if (!qos_map_set_len) {
		wpa_printf(MSG_INFO, "QoS Map Set is not set");
		return -1;
	}

	if (hwaddr_aton(cmd, addr))
		return -1;

	sta = ap_get_sta(hapd, addr);
	if (sta == NULL) {
		wpa_printf(MSG_DEBUG, "Station " MACSTR " not found "
			   "for QoS Map Configuration message",
			   MAC2STR(addr));
		return -1;
	}

	if (!sta->qos_map_enabled) {
		wpa_printf(MSG_DEBUG, "Station " MACSTR " did not indicate "
			   "support for QoS Map", MAC2STR(addr));
		return -1;
	}

	buf = wpabuf_alloc(2 + 2 + qos_map_set_len);
	if (buf == NULL)
		return -1;

	wpabuf_put_u8(buf, WLAN_ACTION_QOS);
	wpabuf_put_u8(buf, QOS_QOS_MAP_CONFIG);

	/* QoS Map Set Element */
	wpabuf_put_u8(buf, WLAN_EID_QOS_MAP_SET);
	wpabuf_put_u8(buf, qos_map_set_len);
	wpabuf_put_data(buf, qos_map_set, qos_map_set_len);

	ret = hostapd_drv_send_action(hapd, hapd->iface->freq, 0, addr,
				      wpabuf_head(buf), wpabuf_len(buf));
	wpabuf_free(buf);

	return ret;
}

#endif /* CONFIG_INTERWORKING */


#ifdef CONFIG_WNM

static int hostapd_ctrl_iface_disassoc_imminent(struct hostapd_data *hapd,
						const char *cmd)
{
	u8 addr[ETH_ALEN];
	int disassoc_timer;
	struct sta_info *sta;

	if (hwaddr_aton(cmd, addr))
		return -1;
	if (cmd[17] != ' ')
		return -1;
	disassoc_timer = atoi(cmd + 17);

	sta = ap_get_sta(hapd, addr);
	if (sta == NULL) {
		wpa_printf(MSG_DEBUG, "Station " MACSTR
			   " not found for disassociation imminent message",
			   MAC2STR(addr));
		return -1;
	}

	return wnm_send_disassoc_imminent(hapd, sta, disassoc_timer);
}


static int hostapd_ctrl_iface_ess_disassoc(struct hostapd_data *hapd,
					   const char *cmd)
{
	u8 addr[ETH_ALEN];
	const char *url, *timerstr;
	int disassoc_timer;
	struct sta_info *sta;

	if (hwaddr_aton(cmd, addr))
		return -1;

	sta = ap_get_sta(hapd, addr);
	if (sta == NULL) {
		wpa_printf(MSG_DEBUG, "Station " MACSTR
			   " not found for ESS disassociation imminent message",
			   MAC2STR(addr));
		return -1;
	}

	timerstr = cmd + 17;
	if (*timerstr != ' ')
		return -1;
	timerstr++;
	disassoc_timer = atoi(timerstr);
	if (disassoc_timer < 0 || disassoc_timer > 65535)
		return -1;

	url = os_strchr(timerstr, ' ');
	if (url == NULL)
		return -1;
	url++;

	return wnm_send_ess_disassoc_imminent(hapd, sta, url, disassoc_timer);
}


static int hostapd_ctrl_iface_bss_tm_req(struct hostapd_data *hapd,
					 const char *cmd)
{
	u8 addr[ETH_ALEN];
	const char *pos, *end;
	int disassoc_timer = 0;
	struct sta_info *sta;
	u8 req_mode = 0, valid_int = 0x01;
	u8 bss_term_dur[12];
	char *url = NULL;
	int ret;
	u8 nei_rep[1000];
	u8 *nei_pos = nei_rep;

	if (hwaddr_aton(cmd, addr)) {
		wpa_printf(MSG_DEBUG, "Invalid STA MAC address");
		return -1;
	}

	sta = ap_get_sta(hapd, addr);
	if (sta == NULL) {
		wpa_printf(MSG_DEBUG, "Station " MACSTR
			   " not found for BSS TM Request message",
			   MAC2STR(addr));
		return -1;
	}

	pos = os_strstr(cmd, " disassoc_timer=");
	if (pos) {
		pos += 16;
		disassoc_timer = atoi(pos);
		if (disassoc_timer < 0 || disassoc_timer > 65535) {
			wpa_printf(MSG_DEBUG, "Invalid disassoc_timer");
			return -1;
		}
	}

	pos = os_strstr(cmd, " valid_int=");
	if (pos) {
		pos += 11;
		valid_int = atoi(pos);
	}

	pos = os_strstr(cmd, " bss_term=");
	if (pos) {
		pos += 10;
		req_mode |= WNM_BSS_TM_REQ_BSS_TERMINATION_INCLUDED;
		/* TODO: TSF configurable/learnable */
		bss_term_dur[0] = 4; /* Subelement ID */
		bss_term_dur[1] = 10; /* Length */
		os_memset(bss_term_dur, 2, 8);
		end = os_strchr(pos, ',');
		if (end == NULL) {
			wpa_printf(MSG_DEBUG, "Invalid bss_term data");
			return -1;
		}
		end++;
		WPA_PUT_LE16(&bss_term_dur[10], atoi(end));
	}


	/*
	 * BSS Transition Candidate List Entries - Neighbor Report elements
	 * neighbor=<BSSID>,<BSSID Information>,<Operating Class>,
	 * <Channel Number>,<PHY Type>[,<hexdump of Optional Subelements>]
	 */
	pos = cmd;
	while (pos) {
		u8 *nei_start;
		long int val;
		char *endptr, *tmp;

		pos = os_strstr(pos, " neighbor=");
		if (!pos)
			break;
		if (nei_pos + 15 > nei_rep + sizeof(nei_rep)) {
			wpa_printf(MSG_DEBUG,
				   "Not enough room for additional neighbor");
			return -1;
		}
		pos += 10;

		nei_start = nei_pos;
		*nei_pos++ = WLAN_EID_NEIGHBOR_REPORT;
		nei_pos++; /* length to be filled in */

		if (hwaddr_aton(pos, nei_pos)) {
			wpa_printf(MSG_DEBUG, "Invalid BSSID");
			return -1;
		}
		nei_pos += ETH_ALEN;
		pos += 17;
		if (*pos != ',') {
			wpa_printf(MSG_DEBUG, "Missing BSSID Information");
			return -1;
		}
		pos++;

		val = strtol(pos, &endptr, 0);
		WPA_PUT_LE32(nei_pos, val);
		nei_pos += 4;
		if (*endptr != ',') {
			wpa_printf(MSG_DEBUG, "Missing Operating Class");
			return -1;
		}
		pos = endptr + 1;

		*nei_pos++ = atoi(pos); /* Operating Class */
		pos = os_strchr(pos, ',');
		if (pos == NULL) {
			wpa_printf(MSG_DEBUG, "Missing Channel Number");
			return -1;
		}
		pos++;

		*nei_pos++ = atoi(pos); /* Channel Number */
		pos = os_strchr(pos, ',');
		if (pos == NULL) {
			wpa_printf(MSG_DEBUG, "Missing PHY Type");
			return -1;
		}
		pos++;

		*nei_pos++ = atoi(pos); /* PHY Type */
		end = os_strchr(pos, ' ');
		tmp = os_strchr(pos, ',');
		if (tmp && (!end || tmp < end)) {
			/* Optional Subelements (hexdump) */
			size_t len;

			pos = tmp + 1;
			end = os_strchr(pos, ' ');
			if (end)
				len = end - pos;
			else
				len = os_strlen(pos);
			if (nei_pos + len / 2 > nei_rep + sizeof(nei_rep)) {
				wpa_printf(MSG_DEBUG,
					   "Not enough room for neighbor subelements");
				return -1;
			}
			if (len & 0x01 ||
			    hexstr2bin(pos, nei_pos, len / 2) < 0) {
				wpa_printf(MSG_DEBUG,
					   "Invalid neighbor subelement info");
				return -1;
			}
			nei_pos += len / 2;
			pos = end;
		}

		nei_start[1] = nei_pos - nei_start - 2;
	}

	pos = os_strstr(cmd, " url=");
	if (pos) {
		size_t len;
		pos += 5;
		end = os_strchr(pos, ' ');
		if (end)
			len = end - pos;
		else
			len = os_strlen(pos);
		url = os_malloc(len + 1);
		if (url == NULL)
			return -1;
		os_memcpy(url, pos, len);
		url[len] = '\0';
		req_mode |= WNM_BSS_TM_REQ_ESS_DISASSOC_IMMINENT;
	}

	if (os_strstr(cmd, " pref=1"))
		req_mode |= WNM_BSS_TM_REQ_PREF_CAND_LIST_INCLUDED;
	if (os_strstr(cmd, " abridged=1"))
		req_mode |= WNM_BSS_TM_REQ_ABRIDGED;
	if (os_strstr(cmd, " disassoc_imminent=1"))
		req_mode |= WNM_BSS_TM_REQ_DISASSOC_IMMINENT;

	ret = wnm_send_bss_tm_req(hapd, sta, req_mode, disassoc_timer,
				  valid_int, bss_term_dur, url,
				  nei_pos > nei_rep ? nei_rep : NULL,
				  nei_pos - nei_rep);
	os_free(url);
	return ret;
}

#endif /* CONFIG_WNM */


static int hostapd_ctrl_iface_get_key_mgmt(struct hostapd_data *hapd,
					   char *buf, size_t buflen)
{
	int ret = 0;
	char *pos, *end;

	pos = buf;
	end = buf + buflen;

	WPA_ASSERT(hapd->conf->wpa_key_mgmt);

	if (hapd->conf->wpa_key_mgmt & WPA_KEY_MGMT_PSK) {
		ret = os_snprintf(pos, end - pos, "WPA-PSK ");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}
	if (hapd->conf->wpa_key_mgmt & WPA_KEY_MGMT_IEEE8021X) {
		ret = os_snprintf(pos, end - pos, "WPA-EAP ");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}
#ifdef CONFIG_IEEE80211R
	if (hapd->conf->wpa_key_mgmt & WPA_KEY_MGMT_FT_PSK) {
		ret = os_snprintf(pos, end - pos, "FT-PSK ");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}
	if (hapd->conf->wpa_key_mgmt & WPA_KEY_MGMT_FT_IEEE8021X) {
		ret = os_snprintf(pos, end - pos, "FT-EAP ");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}
#ifdef CONFIG_SAE
	if (hapd->conf->wpa_key_mgmt & WPA_KEY_MGMT_FT_SAE) {
		ret = os_snprintf(pos, end - pos, "FT-SAE ");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}
#endif /* CONFIG_SAE */
#endif /* CONFIG_IEEE80211R */
#ifdef CONFIG_IEEE80211W
	if (hapd->conf->wpa_key_mgmt & WPA_KEY_MGMT_PSK_SHA256) {
		ret = os_snprintf(pos, end - pos, "WPA-PSK-SHA256 ");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}
	if (hapd->conf->wpa_key_mgmt & WPA_KEY_MGMT_IEEE8021X_SHA256) {
		ret = os_snprintf(pos, end - pos, "WPA-EAP-SHA256 ");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}
#endif /* CONFIG_IEEE80211W */
#ifdef CONFIG_SAE
	if (hapd->conf->wpa_key_mgmt & WPA_KEY_MGMT_SAE) {
		ret = os_snprintf(pos, end - pos, "SAE ");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}
#endif /* CONFIG_SAE */
	if (hapd->conf->wpa_key_mgmt & WPA_KEY_MGMT_IEEE8021X_SUITE_B) {
		ret = os_snprintf(pos, end - pos, "WPA-EAP-SUITE-B ");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}
	if (hapd->conf->wpa_key_mgmt &
	    WPA_KEY_MGMT_IEEE8021X_SUITE_B_192) {
		ret = os_snprintf(pos, end - pos,
				  "WPA-EAP-SUITE-B-192 ");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}

	if (pos > buf && *(pos - 1) == ' ') {
		*(pos - 1) = '\0';
		pos--;
	}

	return pos - buf;
}


static int hostapd_ctrl_iface_get_config(struct hostapd_data *hapd,
					 char *buf, size_t buflen)
{
	int ret;
	char *pos, *end;

	pos = buf;
	end = buf + buflen;

	ret = os_snprintf(pos, end - pos, "bssid=" MACSTR "\n"
			  "ssid=%s\n",
			  MAC2STR(hapd->own_addr),
			  wpa_ssid_txt(hapd->conf->ssid.ssid,
				       hapd->conf->ssid.ssid_len));
	if (os_snprintf_error(end - pos, ret))
		return pos - buf;
	pos += ret;

#ifdef CONFIG_WPS
	ret = os_snprintf(pos, end - pos, "wps_state=%s\n",
			  hapd->conf->wps_state == 0 ? "disabled" :
			  (hapd->conf->wps_state == 1 ? "not configured" :
			   "configured"));
	if (os_snprintf_error(end - pos, ret))
		return pos - buf;
	pos += ret;

	if (hapd->conf->wps_state && hapd->conf->wpa &&
	    hapd->conf->ssid.wpa_passphrase) {
		ret = os_snprintf(pos, end - pos, "passphrase=%s\n",
				  hapd->conf->ssid.wpa_passphrase);
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}

	if (hapd->conf->wps_state && hapd->conf->wpa &&
	    hapd->conf->ssid.wpa_psk &&
	    hapd->conf->ssid.wpa_psk->group) {
		char hex[PMK_LEN * 2 + 1];
		wpa_snprintf_hex(hex, sizeof(hex),
				 hapd->conf->ssid.wpa_psk->psk, PMK_LEN);
		ret = os_snprintf(pos, end - pos, "psk=%s\n", hex);
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}
#endif /* CONFIG_WPS */

	if (hapd->conf->wpa) {
		ret = os_snprintf(pos, end - pos, "wpa=%d\n", hapd->conf->wpa);
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}

	if (hapd->conf->wpa && hapd->conf->wpa_key_mgmt) {
		ret = os_snprintf(pos, end - pos, "key_mgmt=");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;

		pos += hostapd_ctrl_iface_get_key_mgmt(hapd, pos, end - pos);

		ret = os_snprintf(pos, end - pos, "\n");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}

	if (hapd->conf->wpa) {
		ret = os_snprintf(pos, end - pos, "group_cipher=%s\n",
				  wpa_cipher_txt(hapd->conf->wpa_group));
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}

	if ((hapd->conf->wpa & WPA_PROTO_RSN) && hapd->conf->rsn_pairwise) {
		ret = os_snprintf(pos, end - pos, "rsn_pairwise_cipher=");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;

		ret = wpa_write_ciphers(pos, end, hapd->conf->rsn_pairwise,
					" ");
		if (ret < 0)
			return pos - buf;
		pos += ret;

		ret = os_snprintf(pos, end - pos, "\n");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}

	if ((hapd->conf->wpa & WPA_PROTO_WPA) && hapd->conf->wpa_pairwise) {
		ret = os_snprintf(pos, end - pos, "wpa_pairwise_cipher=");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;

		ret = wpa_write_ciphers(pos, end, hapd->conf->wpa_pairwise,
					" ");
		if (ret < 0)
			return pos - buf;
		pos += ret;

		ret = os_snprintf(pos, end - pos, "\n");
		if (os_snprintf_error(end - pos, ret))
			return pos - buf;
		pos += ret;
	}

	return pos - buf;
}


static int hostapd_ctrl_iface_set(struct hostapd_data *hapd, char *cmd)
{
	char *value;
	int ret = 0;

	value = os_strchr(cmd, ' ');
	if (value == NULL)
		return -1;
	*value++ = '\0';

	wpa_printf(MSG_DEBUG, "CTRL_IFACE SET '%s'='%s'", cmd, value);
	if (0) {
#ifdef CONFIG_WPS_TESTING
	} else if (os_strcasecmp(cmd, "wps_version_number") == 0) {
		long int val;
		val = strtol(value, NULL, 0);
		if (val < 0 || val > 0xff) {
			ret = -1;
			wpa_printf(MSG_DEBUG, "WPS: Invalid "
				   "wps_version_number %ld", val);
		} else {
			wps_version_number = val;
			wpa_printf(MSG_DEBUG, "WPS: Testing - force WPS "
				   "version %u.%u",
				   (wps_version_number & 0xf0) >> 4,
				   wps_version_number & 0x0f);
			hostapd_wps_update_ie(hapd);
		}
	} else if (os_strcasecmp(cmd, "wps_testing_dummy_cred") == 0) {
		wps_testing_dummy_cred = atoi(value);
		wpa_printf(MSG_DEBUG, "WPS: Testing - dummy_cred=%d",
			   wps_testing_dummy_cred);
	} else if (os_strcasecmp(cmd, "wps_corrupt_pkhash") == 0) {
		wps_corrupt_pkhash = atoi(value);
		wpa_printf(MSG_DEBUG, "WPS: Testing - wps_corrupt_pkhash=%d",
			   wps_corrupt_pkhash);
#endif /* CONFIG_WPS_TESTING */
#ifdef CONFIG_INTERWORKING
	} else if (os_strcasecmp(cmd, "gas_frag_limit") == 0) {
		int val = atoi(value);
		if (val <= 0)
			ret = -1;
		else
			hapd->gas_frag_limit = val;
#endif /* CONFIG_INTERWORKING */
#ifdef CONFIG_TESTING_OPTIONS
	} else if (os_strcasecmp(cmd, "ext_mgmt_frame_handling") == 0) {
		hapd->ext_mgmt_frame_handling = atoi(value);
	} else if (os_strcasecmp(cmd, "ext_eapol_frame_io") == 0) {
		hapd->ext_eapol_frame_io = atoi(value);
#endif /* CONFIG_TESTING_OPTIONS */
	} else {
		struct sta_info *sta;
		int vlan_id;

		ret = hostapd_set_iface(hapd->iconf, hapd->conf, cmd, value);
		if (ret)
			return ret;

		if (os_strcasecmp(cmd, "deny_mac_file") == 0) {
			for (sta = hapd->sta_list; sta; sta = sta->next) {
				if (hostapd_maclist_found(
					    hapd->conf->deny_mac,
					    hapd->conf->num_deny_mac, sta->addr,
					    &vlan_id) &&
				    (!vlan_id || vlan_id == sta->vlan_id))
					ap_sta_disconnect(
						hapd, sta, sta->addr,
						WLAN_REASON_UNSPECIFIED);
			}
		} else if (hapd->conf->macaddr_acl == DENY_UNLESS_ACCEPTED &&
			   os_strcasecmp(cmd, "accept_mac_file") == 0) {
			for (sta = hapd->sta_list; sta; sta = sta->next) {
				if (!hostapd_maclist_found(
					    hapd->conf->accept_mac,
					    hapd->conf->num_accept_mac,
					    sta->addr, &vlan_id) ||
				    (vlan_id && vlan_id != sta->vlan_id))
					ap_sta_disconnect(
						hapd, sta, sta->addr,
						WLAN_REASON_UNSPECIFIED);
			}
		}
	}

	return ret;
}


static int hostapd_ctrl_iface_get(struct hostapd_data *hapd, char *cmd,
				  char *buf, size_t buflen)
{
	int res;

	wpa_printf(MSG_DEBUG, "CTRL_IFACE GET '%s'", cmd);

	if (os_strcmp(cmd, "version") == 0) {
		res = os_snprintf(buf, buflen, "%s", VERSION_STR);
		if (os_snprintf_error(buflen, res))
			return -1;
		return res;
	} else if (os_strcmp(cmd, "tls_library") == 0) {
		res = tls_get_library_version(buf, buflen);
		if (os_snprintf_error(buflen, res))
			return -1;
		return res;
	}

	return -1;
}


static int hostapd_ctrl_iface_enable(struct hostapd_iface *iface)
{
	if (hostapd_enable_iface(iface) < 0) {
		wpa_printf(MSG_ERROR, "Enabling of interface failed");
		return -1;
	}
	return 0;
}


static int hostapd_ctrl_iface_reload(struct hostapd_iface *iface)
{
	if (hostapd_reload_iface(iface) < 0) {
		wpa_printf(MSG_ERROR, "Reloading of interface failed");
		return -1;
	}
	return 0;
}


static int hostapd_ctrl_iface_disable(struct hostapd_iface *iface)
{
	if (hostapd_disable_iface(iface) < 0) {
		wpa_printf(MSG_ERROR, "Disabling of interface failed");
		return -1;
	}
	return 0;
}


#ifdef CONFIG_TESTING_OPTIONS

static int hostapd_ctrl_iface_radar(struct hostapd_data *hapd, char *cmd)
{
	union wpa_event_data data;
	char *pos, *param;
	enum wpa_event_type event;

	wpa_printf(MSG_DEBUG, "RADAR TEST: %s", cmd);

	os_memset(&data, 0, sizeof(data));

	param = os_strchr(cmd, ' ');
	if (param == NULL)
		return -1;
	*param++ = '\0';

	if (os_strcmp(cmd, "DETECTED") == 0)
		event = EVENT_DFS_RADAR_DETECTED;
	else if (os_strcmp(cmd, "CAC-FINISHED") == 0)
		event = EVENT_DFS_CAC_FINISHED;
	else if (os_strcmp(cmd, "CAC-ABORTED") == 0)
		event = EVENT_DFS_CAC_ABORTED;
	else if (os_strcmp(cmd, "NOP-FINISHED") == 0)
		event = EVENT_DFS_NOP_FINISHED;
	else {
		wpa_printf(MSG_DEBUG, "Unsupported RADAR test command: %s",
			   cmd);
		return -1;
	}

	pos = os_strstr(param, "freq=");
	if (pos)
		data.dfs_event.freq = atoi(pos + 5);

	pos = os_strstr(param, "ht_enabled=1");
	if (pos)
		data.dfs_event.ht_enabled = 1;

	pos = os_strstr(param, "chan_offset=");
	if (pos)
		data.dfs_event.chan_offset = atoi(pos + 12);

	pos = os_strstr(param, "chan_width=");
	if (pos)
		data.dfs_event.chan_width = atoi(pos + 11);

	pos = os_strstr(param, "cf1=");
	if (pos)
		data.dfs_event.cf1 = atoi(pos + 4);

	pos = os_strstr(param, "cf2=");
	if (pos)
		data.dfs_event.cf2 = atoi(pos + 4);

	wpa_supplicant_event(hapd, event, &data);

	return 0;
}


static int hostapd_ctrl_iface_mgmt_tx(struct hostapd_data *hapd, char *cmd)
{
	size_t len;
	u8 *buf;
	int res;

	wpa_printf(MSG_DEBUG, "External MGMT TX: %s", cmd);

	len = os_strlen(cmd);
	if (len & 1)
		return -1;
	len /= 2;

	buf = os_malloc(len);
	if (buf == NULL)
		return -1;

	if (hexstr2bin(cmd, buf, len) < 0) {
		os_free(buf);
		return -1;
	}

	res = hostapd_drv_send_mlme(hapd, buf, len, 0);
	os_free(buf);
	return res;
}


static int hostapd_ctrl_iface_eapol_rx(struct hostapd_data *hapd, char *cmd)
{
	char *pos;
	u8 src[ETH_ALEN], *buf;
	int used;
	size_t len;

	wpa_printf(MSG_DEBUG, "External EAPOL RX: %s", cmd);

	pos = cmd;
	used = hwaddr_aton2(pos, src);
	if (used < 0)
		return -1;
	pos += used;
	while (*pos == ' ')
		pos++;

	len = os_strlen(pos);
	if (len & 1)
		return -1;
	len /= 2;

	buf = os_malloc(len);
	if (buf == NULL)
		return -1;

	if (hexstr2bin(pos, buf, len) < 0) {
		os_free(buf);
		return -1;
	}

	ieee802_1x_receive(hapd, src, buf, len);
	os_free(buf);

	return 0;
}


static u16 ipv4_hdr_checksum(const void *buf, size_t len)
{
	size_t i;
	u32 sum = 0;
	const u16 *pos = buf;

	for (i = 0; i < len / 2; i++)
		sum += *pos++;

	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return sum ^ 0xffff;
}


#define HWSIM_PACKETLEN 1500
#define HWSIM_IP_LEN (HWSIM_PACKETLEN - sizeof(struct ether_header))

void hostapd_data_test_rx(void *ctx, const u8 *src_addr, const u8 *buf,
			  size_t len)
{
	struct hostapd_data *hapd = ctx;
	const struct ether_header *eth;
	struct iphdr ip;
	const u8 *pos;
	unsigned int i;

	if (len != HWSIM_PACKETLEN)
		return;

	eth = (const struct ether_header *) buf;
	os_memcpy(&ip, eth + 1, sizeof(ip));
	pos = &buf[sizeof(*eth) + sizeof(ip)];

	if (ip.ihl != 5 || ip.version != 4 ||
	    ntohs(ip.tot_len) != HWSIM_IP_LEN)
		return;

	for (i = 0; i < HWSIM_IP_LEN - sizeof(ip); i++) {
		if (*pos != (u8) i)
			return;
		pos++;
	}

	wpa_msg(hapd->msg_ctx, MSG_INFO, "DATA-TEST-RX " MACSTR " " MACSTR,
		MAC2STR(eth->ether_dhost), MAC2STR(eth->ether_shost));
}


static int hostapd_ctrl_iface_data_test_config(struct hostapd_data *hapd,
					       char *cmd)
{
	int enabled = atoi(cmd);
	char *pos;
	const char *ifname;

	if (!enabled) {
		if (hapd->l2_test) {
			l2_packet_deinit(hapd->l2_test);
			hapd->l2_test = NULL;
			wpa_dbg(hapd->msg_ctx, MSG_DEBUG,
				"test data: Disabled");
		}
		return 0;
	}

	if (hapd->l2_test)
		return 0;

	pos = os_strstr(cmd, " ifname=");
	if (pos)
		ifname = pos + 8;
	else
		ifname = hapd->conf->iface;

	hapd->l2_test = l2_packet_init(ifname, hapd->own_addr,
					ETHERTYPE_IP, hostapd_data_test_rx,
					hapd, 1);
	if (hapd->l2_test == NULL)
		return -1;

	wpa_dbg(hapd->msg_ctx, MSG_DEBUG, "test data: Enabled");

	return 0;
}


static int hostapd_ctrl_iface_data_test_tx(struct hostapd_data *hapd, char *cmd)
{
	u8 dst[ETH_ALEN], src[ETH_ALEN];
	char *pos;
	int used;
	long int val;
	u8 tos;
	u8 buf[2 + HWSIM_PACKETLEN];
	struct ether_header *eth;
	struct iphdr *ip;
	u8 *dpos;
	unsigned int i;

	if (hapd->l2_test == NULL)
		return -1;

	/* format: <dst> <src> <tos> */

	pos = cmd;
	used = hwaddr_aton2(pos, dst);
	if (used < 0)
		return -1;
	pos += used;
	while (*pos == ' ')
		pos++;
	used = hwaddr_aton2(pos, src);
	if (used < 0)
		return -1;
	pos += used;

	val = strtol(pos, NULL, 0);
	if (val < 0 || val > 0xff)
		return -1;
	tos = val;

	eth = (struct ether_header *) &buf[2];
	os_memcpy(eth->ether_dhost, dst, ETH_ALEN);
	os_memcpy(eth->ether_shost, src, ETH_ALEN);
	eth->ether_type = htons(ETHERTYPE_IP);
	ip = (struct iphdr *) (eth + 1);
	os_memset(ip, 0, sizeof(*ip));
	ip->ihl = 5;
	ip->version = 4;
	ip->ttl = 64;
	ip->tos = tos;
	ip->tot_len = htons(HWSIM_IP_LEN);
	ip->protocol = 1;
	ip->saddr = htonl(192U << 24 | 168 << 16 | 1 << 8 | 1);
	ip->daddr = htonl(192U << 24 | 168 << 16 | 1 << 8 | 2);
	ip->check = ipv4_hdr_checksum(ip, sizeof(*ip));
	dpos = (u8 *) (ip + 1);
	for (i = 0; i < HWSIM_IP_LEN - sizeof(*ip); i++)
		*dpos++ = i;

	if (l2_packet_send(hapd->l2_test, dst, ETHERTYPE_IP, &buf[2],
			   HWSIM_PACKETLEN) < 0)
		return -1;

	wpa_dbg(hapd->msg_ctx, MSG_DEBUG, "test data: TX dst=" MACSTR
		" src=" MACSTR " tos=0x%x", MAC2STR(dst), MAC2STR(src), tos);

	return 0;
}


static int hostapd_ctrl_iface_data_test_frame(struct hostapd_data *hapd,
					      char *cmd)
{
	u8 *buf;
	struct ether_header *eth;
	struct l2_packet_data *l2 = NULL;
	size_t len;
	u16 ethertype;
	int res = -1;
	const char *ifname = hapd->conf->iface;

	if (os_strncmp(cmd, "ifname=", 7) == 0) {
		cmd += 7;
		ifname = cmd;
		cmd = os_strchr(cmd, ' ');
		if (cmd == NULL)
			return -1;
		*cmd++ = '\0';
	}

	len = os_strlen(cmd);
	if (len & 1 || len < ETH_HLEN * 2)
		return -1;
	len /= 2;

	buf = os_malloc(len);
	if (buf == NULL)
		return -1;

	if (hexstr2bin(cmd, buf, len) < 0)
		goto done;

	eth = (struct ether_header *) buf;
	ethertype = ntohs(eth->ether_type);

	l2 = l2_packet_init(ifname, hapd->own_addr, ethertype,
			    hostapd_data_test_rx, hapd, 1);
	if (l2 == NULL)
		goto done;

	res = l2_packet_send(l2, eth->ether_dhost, ethertype, buf, len);
	wpa_dbg(hapd->msg_ctx, MSG_DEBUG, "test data: TX frame res=%d", res);
done:
	if (l2)
		l2_packet_deinit(l2);
	os_free(buf);

	return res < 0 ? -1 : 0;
}


static int hostapd_ctrl_test_alloc_fail(struct hostapd_data *hapd, char *cmd)
{
#ifdef WPA_TRACE_BFD
	extern char wpa_trace_fail_func[256];
	extern unsigned int wpa_trace_fail_after;
	char *pos;

	wpa_trace_fail_after = atoi(cmd);
	pos = os_strchr(cmd, ':');
	if (pos) {
		pos++;
		os_strlcpy(wpa_trace_fail_func, pos,
			   sizeof(wpa_trace_fail_func));
	} else {
		wpa_trace_fail_after = 0;
	}

	return 0;
#else /* WPA_TRACE_BFD */
	return -1;
#endif /* WPA_TRACE_BFD */
}


static int hostapd_ctrl_get_alloc_fail(struct hostapd_data *hapd,
				       char *buf, size_t buflen)
{
#ifdef WPA_TRACE_BFD
	extern char wpa_trace_fail_func[256];
	extern unsigned int wpa_trace_fail_after;

	return os_snprintf(buf, buflen, "%u:%s", wpa_trace_fail_after,
			   wpa_trace_fail_func);
#else /* WPA_TRACE_BFD */
	return -1;
#endif /* WPA_TRACE_BFD */
}


static int hostapd_ctrl_test_fail(struct hostapd_data *hapd, char *cmd)
{
#ifdef WPA_TRACE_BFD
	extern char wpa_trace_test_fail_func[256];
	extern unsigned int wpa_trace_test_fail_after;
	char *pos;

	wpa_trace_test_fail_after = atoi(cmd);
	pos = os_strchr(cmd, ':');
	if (pos) {
		pos++;
		os_strlcpy(wpa_trace_test_fail_func, pos,
			   sizeof(wpa_trace_test_fail_func));
	} else {
		wpa_trace_test_fail_after = 0;
	}

	return 0;
#else /* WPA_TRACE_BFD */
	return -1;
#endif /* WPA_TRACE_BFD */
}


static int hostapd_ctrl_get_fail(struct hostapd_data *hapd,
				 char *buf, size_t buflen)
{
#ifdef WPA_TRACE_BFD
	extern char wpa_trace_test_fail_func[256];
	extern unsigned int wpa_trace_test_fail_after;

	return os_snprintf(buf, buflen, "%u:%s", wpa_trace_test_fail_after,
			   wpa_trace_test_fail_func);
#else /* WPA_TRACE_BFD */
	return -1;
#endif /* WPA_TRACE_BFD */
}

#endif /* CONFIG_TESTING_OPTIONS */


static int hostapd_ctrl_iface_chan_switch(struct hostapd_iface *iface,
					  char *pos)
{
#ifdef NEED_AP_MLME
	struct csa_settings settings;
	int ret;
	unsigned int i;

	ret = hostapd_parse_csa_settings(pos, &settings);
	if (ret)
		return ret;

	for (i = 0; i < iface->num_bss; i++) {
		ret = hostapd_switch_channel(iface->bss[i], &settings);
		if (ret) {
			/* FIX: What do we do if CSA fails in the middle of
			 * submitting multi-BSS CSA requests? */
			return ret;
		}
	}

	return 0;
#else /* NEED_AP_MLME */
	return -1;
#endif /* NEED_AP_MLME */
}


static int hostapd_ctrl_iface_mib(struct hostapd_data *hapd, char *reply,
				  int reply_size, const char *param)
{
#ifdef RADIUS_SERVER
	if (os_strcmp(param, "radius_server") == 0) {
		return radius_server_get_mib(hapd->radius_srv, reply,
					     reply_size);
	}
#endif /* RADIUS_SERVER */
	return -1;
}


static int hostapd_ctrl_iface_vendor(struct hostapd_data *hapd, char *cmd,
				     char *buf, size_t buflen)
{
	int ret;
	char *pos;
	u8 *data = NULL;
	unsigned int vendor_id, subcmd;
	struct wpabuf *reply;
	size_t data_len = 0;

	/* cmd: <vendor id> <subcommand id> [<hex formatted data>] */
	vendor_id = strtoul(cmd, &pos, 16);
	if (!isblank(*pos))
		return -EINVAL;

	subcmd = strtoul(pos, &pos, 10);

	if (*pos != '\0') {
		if (!isblank(*pos++))
			return -EINVAL;
		data_len = os_strlen(pos);
	}

	if (data_len) {
		data_len /= 2;
		data = os_malloc(data_len);
		if (!data)
			return -ENOBUFS;

		if (hexstr2bin(pos, data, data_len)) {
			wpa_printf(MSG_DEBUG,
				   "Vendor command: wrong parameter format");
			os_free(data);
			return -EINVAL;
		}
	}

	reply = wpabuf_alloc((buflen - 1) / 2);
	if (!reply) {
		os_free(data);
		return -ENOBUFS;
	}

	ret = hostapd_drv_vendor_cmd(hapd, vendor_id, subcmd, data, data_len,
				     reply);

	if (ret == 0)
		ret = wpa_snprintf_hex(buf, buflen, wpabuf_head_u8(reply),
				       wpabuf_len(reply));

	wpabuf_free(reply);
	os_free(data);

	return ret;
}


static int hostapd_ctrl_iface_eapol_reauth(struct hostapd_data *hapd,
					   const char *cmd)
{
	u8 addr[ETH_ALEN];
	struct sta_info *sta;

	if (hwaddr_aton(cmd, addr))
		return -1;

	sta = ap_get_sta(hapd, addr);
	if (!sta || !sta->eapol_sm)
		return -1;

	eapol_auth_reauthenticate(sta->eapol_sm);
	return 0;
}


static int hostapd_ctrl_iface_eapol_set(struct hostapd_data *hapd, char *cmd)
{
	u8 addr[ETH_ALEN];
	struct sta_info *sta;
	char *pos = cmd, *param;

	if (hwaddr_aton(pos, addr) || pos[17] != ' ')
		return -1;
	pos += 18;
	param = pos;
	pos = os_strchr(pos, ' ');
	if (!pos)
		return -1;
	*pos++ = '\0';

	sta = ap_get_sta(hapd, addr);
	if (!sta || !sta->eapol_sm)
		return -1;

	return eapol_auth_set_conf(sta->eapol_sm, param, pos);
}


static int hostapd_ctrl_iface_log_level(struct hostapd_data *hapd, char *cmd,
					char *buf, size_t buflen)
{
	char *pos, *end, *stamp;
	int ret;

	/* cmd: "LOG_LEVEL [<level>]" */
	if (*cmd == '\0') {
		pos = buf;
		end = buf + buflen;
		ret = os_snprintf(pos, end - pos, "Current level: %s\n"
				  "Timestamp: %d\n",
				  debug_level_str(wpa_debug_level),
				  wpa_debug_timestamp);
		if (os_snprintf_error(end - pos, ret))
			ret = 0;

		return ret;
	}

	while (*cmd == ' ')
		cmd++;

	stamp = os_strchr(cmd, ' ');
	if (stamp) {
		*stamp++ = '\0';
		while (*stamp == ' ') {
			stamp++;
		}
	}

	if (os_strlen(cmd)) {
		int level = str_to_debug_level(cmd);
		if (level < 0)
			return -1;
		wpa_debug_level = level;
	}

	if (stamp && os_strlen(stamp))
		wpa_debug_timestamp = atoi(stamp);

	os_memcpy(buf, "OK\n", 3);
	return 3;
}


#ifdef NEED_AP_MLME
static int hostapd_ctrl_iface_track_sta_list(struct hostapd_data *hapd,
					     char *buf, size_t buflen)
{
	struct hostapd_iface *iface = hapd->iface;
	char *pos, *end;
	struct hostapd_sta_info *info;
	struct os_reltime now;

	sta_track_expire(iface, 0);

	pos = buf;
	end = buf + buflen;

	os_get_reltime(&now);
	dl_list_for_each_reverse(info, &iface->sta_seen,
				 struct hostapd_sta_info, list) {
		struct os_reltime age;
		int ret;

		os_reltime_sub(&now, &info->last_seen, &age);
		ret = os_snprintf(pos, end - pos, MACSTR " %u\n",
				  MAC2STR(info->addr), (unsigned int) age.sec);
		if (os_snprintf_error(end - pos, ret))
			break;
		pos += ret;
	}

	return pos - buf;
}
#endif /* NEED_AP_MLME */


static int hostapd_ctrl_iface_receive_process(struct hostapd_data *hapd,
					      char *buf, char *reply,
					      int reply_size,
					      struct sockaddr_un *from,
					      socklen_t fromlen)
{
	int reply_len, res;

	os_memcpy(reply, "OK\n", 3);
	reply_len = 3;

	if (os_strcmp(buf, "PING") == 0) {
		os_memcpy(reply, "PONG\n", 5);
		reply_len = 5;
	} else if (os_strncmp(buf, "RELOG", 5) == 0) {
		if (wpa_debug_reopen_file() < 0)
			reply_len = -1;
	} else if (os_strcmp(buf, "STATUS") == 0) {
		reply_len = hostapd_ctrl_iface_status(hapd, reply,
						      reply_size);
	} else if (os_strcmp(buf, "STATUS-DRIVER") == 0) {
		reply_len = hostapd_drv_status(hapd, reply, reply_size);
	} else if (os_strcmp(buf, "MIB") == 0) {
		reply_len = ieee802_11_get_mib(hapd, reply, reply_size);
		if (reply_len >= 0) {
			res = wpa_get_mib(hapd->wpa_auth, reply + reply_len,
					  reply_size - reply_len);
			if (res < 0)
				reply_len = -1;
			else
				reply_len += res;
		}
		if (reply_len >= 0) {
			res = ieee802_1x_get_mib(hapd, reply + reply_len,
						 reply_size - reply_len);
			if (res < 0)
				reply_len = -1;
			else
				reply_len += res;
		}
#ifndef CONFIG_NO_RADIUS
		if (reply_len >= 0) {
			res = radius_client_get_mib(hapd->radius,
						    reply + reply_len,
						    reply_size - reply_len);
			if (res < 0)
				reply_len = -1;
			else
				reply_len += res;
		}
#endif /* CONFIG_NO_RADIUS */
	} else if (os_strncmp(buf, "MIB ", 4) == 0) {
		reply_len = hostapd_ctrl_iface_mib(hapd, reply, reply_size,
						   buf + 4);
	} else if (os_strcmp(buf, "STA-FIRST") == 0) {
		reply_len = hostapd_ctrl_iface_sta_first(hapd, reply,
							 reply_size);
	} else if (os_strncmp(buf, "STA ", 4) == 0) {
		reply_len = hostapd_ctrl_iface_sta(hapd, buf + 4, reply,
						   reply_size);
	} else if (os_strncmp(buf, "STA-NEXT ", 9) == 0) {
		reply_len = hostapd_ctrl_iface_sta_next(hapd, buf + 9, reply,
							reply_size);
	} else if (os_strcmp(buf, "ATTACH") == 0) {
		if (hostapd_ctrl_iface_attach(hapd, from, fromlen))
			reply_len = -1;
	} else if (os_strcmp(buf, "DETACH") == 0) {
		if (hostapd_ctrl_iface_detach(hapd, from, fromlen))
			reply_len = -1;
	} else if (os_strncmp(buf, "LEVEL ", 6) == 0) {
		if (hostapd_ctrl_iface_level(hapd, from, fromlen,
						    buf + 6))
			reply_len = -1;
	} else if (os_strncmp(buf, "NEW_STA ", 8) == 0) {
		if (hostapd_ctrl_iface_new_sta(hapd, buf + 8))
			reply_len = -1;
	} else if (os_strncmp(buf, "DEAUTHENTICATE ", 15) == 0) {
		if (hostapd_ctrl_iface_deauthenticate(hapd, buf + 15))
			reply_len = -1;
	} else if (os_strncmp(buf, "DISASSOCIATE ", 13) == 0) {
		if (hostapd_ctrl_iface_disassociate(hapd, buf + 13))
			reply_len = -1;
	} else if (os_strcmp(buf, "STOP_AP") == 0) {
		if (hostapd_ctrl_iface_stop_ap(hapd))
			reply_len = -1;
#ifdef CONFIG_IEEE80211W
#ifdef NEED_AP_MLME
	} else if (os_strncmp(buf, "SA_QUERY ", 9) == 0) {
		if (hostapd_ctrl_iface_sa_query(hapd, buf + 9))
			reply_len = -1;
#endif /* NEED_AP_MLME */
#endif /* CONFIG_IEEE80211W */
#ifdef CONFIG_WPS
	} else if (os_strncmp(buf, "WPS_PIN ", 8) == 0) {
		if (hostapd_ctrl_iface_wps_pin(hapd, buf + 8))
			reply_len = -1;
	} else if (os_strncmp(buf, "WPS_CHECK_PIN ", 14) == 0) {
		reply_len = hostapd_ctrl_iface_wps_check_pin(
			hapd, buf + 14, reply, reply_size);
	} else if (os_strcmp(buf, "WPS_PBC") == 0) {
		if (hostapd_wps_button_pushed(hapd, NULL))
			reply_len = -1;
	} else if (os_strcmp(buf, "WPS_CANCEL") == 0) {
		if (hostapd_wps_cancel(hapd))
			reply_len = -1;
	} else if (os_strncmp(buf, "WPS_AP_PIN ", 11) == 0) {
		reply_len = hostapd_ctrl_iface_wps_ap_pin(hapd, buf + 11,
							  reply, reply_size);
	} else if (os_strncmp(buf, "WPS_CONFIG ", 11) == 0) {
		if (hostapd_ctrl_iface_wps_config(hapd, buf + 11) < 0)
			reply_len = -1;
	} else if (os_strncmp(buf, "WPS_GET_STATUS", 13) == 0) {
		reply_len = hostapd_ctrl_iface_wps_get_status(hapd, reply,
							      reply_size);
#ifdef CONFIG_WPS_NFC
	} else if (os_strncmp(buf, "WPS_NFC_TAG_READ ", 17) == 0) {
		if (hostapd_ctrl_iface_wps_nfc_tag_read(hapd, buf + 17))
			reply_len = -1;
	} else if (os_strncmp(buf, "WPS_NFC_CONFIG_TOKEN ", 21) == 0) {
		reply_len = hostapd_ctrl_iface_wps_nfc_config_token(
			hapd, buf + 21, reply, reply_size);
	} else if (os_strncmp(buf, "WPS_NFC_TOKEN ", 14) == 0) {
		reply_len = hostapd_ctrl_iface_wps_nfc_token(
			hapd, buf + 14, reply, reply_size);
	} else if (os_strncmp(buf, "NFC_GET_HANDOVER_SEL ", 21) == 0) {
		reply_len = hostapd_ctrl_iface_nfc_get_handover_sel(
			hapd, buf + 21, reply, reply_size);
	} else if (os_strncmp(buf, "NFC_REPORT_HANDOVER ", 20) == 0) {
		if (hostapd_ctrl_iface_nfc_report_handover(hapd, buf + 20))
			reply_len = -1;
#endif /* CONFIG_WPS_NFC */
#endif /* CONFIG_WPS */
#ifdef CONFIG_INTERWORKING
	} else if (os_strncmp(buf, "SET_QOS_MAP_SET ", 16) == 0) {
		if (hostapd_ctrl_iface_set_qos_map_set(hapd, buf + 16))
			reply_len = -1;
	} else if (os_strncmp(buf, "SEND_QOS_MAP_CONF ", 18) == 0) {
		if (hostapd_ctrl_iface_send_qos_map_conf(hapd, buf + 18))
			reply_len = -1;
#endif /* CONFIG_INTERWORKING */
#ifdef CONFIG_HS20
	} else if (os_strncmp(buf, "HS20_WNM_NOTIF ", 15) == 0) {
		if (hostapd_ctrl_iface_hs20_wnm_notif(hapd, buf + 15))
			reply_len = -1;
	} else if (os_strncmp(buf, "HS20_DEAUTH_REQ ", 16) == 0) {
		if (hostapd_ctrl_iface_hs20_deauth_req(hapd, buf + 16))
			reply_len = -1;
#endif /* CONFIG_HS20 */
#ifdef CONFIG_WNM
	} else if (os_strncmp(buf, "DISASSOC_IMMINENT ", 18) == 0) {
		if (hostapd_ctrl_iface_disassoc_imminent(hapd, buf + 18))
			reply_len = -1;
	} else if (os_strncmp(buf, "ESS_DISASSOC ", 13) == 0) {
		if (hostapd_ctrl_iface_ess_disassoc(hapd, buf + 13))
			reply_len = -1;
	} else if (os_strncmp(buf, "BSS_TM_REQ ", 11) == 0) {
		if (hostapd_ctrl_iface_bss_tm_req(hapd, buf + 11))
			reply_len = -1;
#endif /* CONFIG_WNM */
	} else if (os_strcmp(buf, "GET_CONFIG") == 0) {
		reply_len = hostapd_ctrl_iface_get_config(hapd, reply,
							  reply_size);
	} else if (os_strncmp(buf, "SET ", 4) == 0) {
		if (hostapd_ctrl_iface_set(hapd, buf + 4))
			reply_len = -1;
	} else if (os_strncmp(buf, "GET ", 4) == 0) {
		reply_len = hostapd_ctrl_iface_get(hapd, buf + 4, reply,
						   reply_size);
	} else if (os_strncmp(buf, "ENABLE", 6) == 0) {
		if (hostapd_ctrl_iface_enable(hapd->iface))
			reply_len = -1;
	} else if (os_strncmp(buf, "RELOAD", 6) == 0) {
		if (hostapd_ctrl_iface_reload(hapd->iface))
			reply_len = -1;
	} else if (os_strncmp(buf, "DISABLE", 7) == 0) {
		if (hostapd_ctrl_iface_disable(hapd->iface))
			reply_len = -1;
	} else if (os_strcmp(buf, "UPDATE_BEACON") == 0) {
		if (ieee802_11_set_beacon(hapd))
			reply_len = -1;
#ifdef CONFIG_TESTING_OPTIONS
	} else if (os_strncmp(buf, "RADAR ", 6) == 0) {
		if (hostapd_ctrl_iface_radar(hapd, buf + 6))
			reply_len = -1;
	} else if (os_strncmp(buf, "MGMT_TX ", 8) == 0) {
		if (hostapd_ctrl_iface_mgmt_tx(hapd, buf + 8))
			reply_len = -1;
	} else if (os_strncmp(buf, "EAPOL_RX ", 9) == 0) {
		if (hostapd_ctrl_iface_eapol_rx(hapd, buf + 9) < 0)
			reply_len = -1;
	} else if (os_strncmp(buf, "DATA_TEST_CONFIG ", 17) == 0) {
		if (hostapd_ctrl_iface_data_test_config(hapd, buf + 17) < 0)
			reply_len = -1;
	} else if (os_strncmp(buf, "DATA_TEST_TX ", 13) == 0) {
		if (hostapd_ctrl_iface_data_test_tx(hapd, buf + 13) < 0)
			reply_len = -1;
	} else if (os_strncmp(buf, "DATA_TEST_FRAME ", 16) == 0) {
		if (hostapd_ctrl_iface_data_test_frame(hapd, buf + 16) < 0)
			reply_len = -1;
	} else if (os_strncmp(buf, "TEST_ALLOC_FAIL ", 16) == 0) {
		if (hostapd_ctrl_test_alloc_fail(hapd, buf + 16) < 0)
			reply_len = -1;
	} else if (os_strcmp(buf, "GET_ALLOC_FAIL") == 0) {
		reply_len = hostapd_ctrl_get_alloc_fail(hapd, reply,
							reply_size);
	} else if (os_strncmp(buf, "TEST_FAIL ", 10) == 0) {
		if (hostapd_ctrl_test_fail(hapd, buf + 10) < 0)
			reply_len = -1;
	} else if (os_strcmp(buf, "GET_FAIL") == 0) {
		reply_len = hostapd_ctrl_get_fail(hapd, reply, reply_size);
#endif /* CONFIG_TESTING_OPTIONS */
	} else if (os_strncmp(buf, "CHAN_SWITCH ", 12) == 0) {
		if (hostapd_ctrl_iface_chan_switch(hapd->iface, buf + 12))
			reply_len = -1;
	} else if (os_strncmp(buf, "VENDOR ", 7) == 0) {
		reply_len = hostapd_ctrl_iface_vendor(hapd, buf + 7, reply,
						      reply_size);
	} else if (os_strcmp(buf, "ERP_FLUSH") == 0) {
		ieee802_1x_erp_flush(hapd);
#ifdef RADIUS_SERVER
		radius_server_erp_flush(hapd->radius_srv);
#endif /* RADIUS_SERVER */
	} else if (os_strncmp(buf, "EAPOL_REAUTH ", 13) == 0) {
		if (hostapd_ctrl_iface_eapol_reauth(hapd, buf + 13))
			reply_len = -1;
	} else if (os_strncmp(buf, "EAPOL_SET ", 10) == 0) {
		if (hostapd_ctrl_iface_eapol_set(hapd, buf + 10))
			reply_len = -1;
	} else if (os_strncmp(buf, "LOG_LEVEL", 9) == 0) {
		reply_len = hostapd_ctrl_iface_log_level(
			hapd, buf + 9, reply, reply_size);
#ifdef NEED_AP_MLME
	} else if (os_strcmp(buf, "TRACK_STA_LIST") == 0) {
		reply_len = hostapd_ctrl_iface_track_sta_list(
			hapd, reply, reply_size);
#endif /* NEED_AP_MLME */
	} else {
		os_memcpy(reply, "UNKNOWN COMMAND\n", 16);
		reply_len = 16;
	}

	if (reply_len < 0) {
		os_memcpy(reply, "FAIL\n", 5);
		reply_len = 5;
	}

	return reply_len;
}


static void hostapd_ctrl_iface_receive(int sock, void *eloop_ctx,
				       void *sock_ctx)
{
	struct hostapd_data *hapd = eloop_ctx;
	char buf[4096];
	int res;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);
	char *reply;
	const int reply_size = 4096;
	int reply_len;
	int level = MSG_DEBUG;

	res = recvfrom(sock, buf, sizeof(buf) - 1, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		wpa_printf(MSG_ERROR, "recvfrom(ctrl_iface): %s",
			   strerror(errno));
		return;
	}
	buf[res] = '\0';
	if (os_strcmp(buf, "PING") == 0)
		level = MSG_EXCESSIVE;
	wpa_hexdump_ascii(level, "RX ctrl_iface", (u8 *) buf, res);

	reply = os_malloc(reply_size);
	if (reply == NULL) {
		if (sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *) &from,
			   fromlen) < 0) {
			wpa_printf(MSG_DEBUG, "CTRL: sendto failed: %s",
				   strerror(errno));
		}
		return;
	}

	reply_len = hostapd_ctrl_iface_receive_process(hapd, buf,
						       reply, reply_size,
						       &from, fromlen);

	if (sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from,
		   fromlen) < 0) {
		wpa_printf(MSG_DEBUG, "CTRL: sendto failed: %s",
			   strerror(errno));
	}
	os_free(reply);
}


static char * hostapd_ctrl_iface_path(struct hostapd_data *hapd)
{
	char *buf;
	size_t len;

	if (hapd->conf->ctrl_interface == NULL)
		return NULL;

	len = os_strlen(hapd->conf->ctrl_interface) +
		os_strlen(hapd->conf->iface) + 2;
	buf = os_malloc(len);
	if (buf == NULL)
		return NULL;

	os_snprintf(buf, len, "%s/%s",
		    hapd->conf->ctrl_interface, hapd->conf->iface);
	buf[len - 1] = '\0';
	return buf;
}


static void hostapd_ctrl_iface_msg_cb(void *ctx, int level,
				      enum wpa_msg_type type,
				      const char *txt, size_t len)
{
	struct hostapd_data *hapd = ctx;
	if (hapd == NULL)
		return;
	hostapd_ctrl_iface_send(hapd, level, type, txt, len);
}


int hostapd_ctrl_iface_init(struct hostapd_data *hapd)
{
	struct sockaddr_un addr;
	int s = -1;
	char *fname = NULL;

	if (hapd->ctrl_sock > -1) {
		wpa_printf(MSG_DEBUG, "ctrl_iface already exists!");
		return 0;
	}

	if (hapd->conf->ctrl_interface == NULL)
		return 0;

	if (mkdir(hapd->conf->ctrl_interface, S_IRWXU | S_IRWXG) < 0) {
		if (errno == EEXIST) {
			wpa_printf(MSG_DEBUG, "Using existing control "
				   "interface directory.");
		} else {
			wpa_printf(MSG_ERROR, "mkdir[ctrl_interface]: %s",
				   strerror(errno));
			goto fail;
		}
	}

	if (hapd->conf->ctrl_interface_gid_set &&
	    chown(hapd->conf->ctrl_interface, -1,
		  hapd->conf->ctrl_interface_gid) < 0) {
		wpa_printf(MSG_ERROR, "chown[ctrl_interface]: %s",
			   strerror(errno));
		return -1;
	}

	if (!hapd->conf->ctrl_interface_gid_set &&
	    hapd->iface->interfaces->ctrl_iface_group &&
	    chown(hapd->conf->ctrl_interface, -1,
		  hapd->iface->interfaces->ctrl_iface_group) < 0) {
		wpa_printf(MSG_ERROR, "chown[ctrl_interface]: %s",
			   strerror(errno));
		return -1;
	}

#ifdef ANDROID
	/*
	 * Android is using umask 0077 which would leave the control interface
	 * directory without group access. This breaks things since Wi-Fi
	 * framework assumes that this directory can be accessed by other
	 * applications in the wifi group. Fix this by adding group access even
	 * if umask value would prevent this.
	 */
	if (chmod(hapd->conf->ctrl_interface, S_IRWXU | S_IRWXG) < 0) {
		wpa_printf(MSG_ERROR, "CTRL: Could not chmod directory: %s",
			   strerror(errno));
		/* Try to continue anyway */
	}
#endif /* ANDROID */

	if (os_strlen(hapd->conf->ctrl_interface) + 1 +
	    os_strlen(hapd->conf->iface) >= sizeof(addr.sun_path))
		goto fail;

	s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (s < 0) {
		wpa_printf(MSG_ERROR, "socket(PF_UNIX): %s", strerror(errno));
		goto fail;
	}

	os_memset(&addr, 0, sizeof(addr));
#ifdef __FreeBSD__
	addr.sun_len = sizeof(addr);
#endif /* __FreeBSD__ */
	addr.sun_family = AF_UNIX;
	fname = hostapd_ctrl_iface_path(hapd);
	if (fname == NULL)
		goto fail;
	os_strlcpy(addr.sun_path, fname, sizeof(addr.sun_path));
	if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		wpa_printf(MSG_DEBUG, "ctrl_iface bind(PF_UNIX) failed: %s",
			   strerror(errno));
		if (connect(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			wpa_printf(MSG_DEBUG, "ctrl_iface exists, but does not"
				   " allow connections - assuming it was left"
				   "over from forced program termination");
			if (unlink(fname) < 0) {
				wpa_printf(MSG_ERROR,
					   "Could not unlink existing ctrl_iface socket '%s': %s",
					   fname, strerror(errno));
				goto fail;
			}
			if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) <
			    0) {
				wpa_printf(MSG_ERROR,
					   "hostapd-ctrl-iface: bind(PF_UNIX): %s",
					   strerror(errno));
				goto fail;
			}
			wpa_printf(MSG_DEBUG, "Successfully replaced leftover "
				   "ctrl_iface socket '%s'", fname);
		} else {
			wpa_printf(MSG_INFO, "ctrl_iface exists and seems to "
				   "be in use - cannot override it");
			wpa_printf(MSG_INFO, "Delete '%s' manually if it is "
				   "not used anymore", fname);
			os_free(fname);
			fname = NULL;
			goto fail;
		}
	}

	if (hapd->conf->ctrl_interface_gid_set &&
	    chown(fname, -1, hapd->conf->ctrl_interface_gid) < 0) {
		wpa_printf(MSG_ERROR, "chown[ctrl_interface/ifname]: %s",
			   strerror(errno));
		goto fail;
	}

	if (!hapd->conf->ctrl_interface_gid_set &&
	    hapd->iface->interfaces->ctrl_iface_group &&
	    chown(fname, -1, hapd->iface->interfaces->ctrl_iface_group) < 0) {
		wpa_printf(MSG_ERROR, "chown[ctrl_interface/ifname]: %s",
			   strerror(errno));
		goto fail;
	}

	if (chmod(fname, S_IRWXU | S_IRWXG) < 0) {
		wpa_printf(MSG_ERROR, "chmod[ctrl_interface/ifname]: %s",
			   strerror(errno));
		goto fail;
	}
	os_free(fname);

	hapd->ctrl_sock = s;
	if (eloop_register_read_sock(s, hostapd_ctrl_iface_receive, hapd,
				     NULL) < 0) {
		hostapd_ctrl_iface_deinit(hapd);
		return -1;
	}
	hapd->msg_ctx = hapd;
	wpa_msg_register_cb(hostapd_ctrl_iface_msg_cb);

	return 0;

fail:
	if (s >= 0)
		close(s);
	if (fname) {
		unlink(fname);
		os_free(fname);
	}
	return -1;
}


void hostapd_ctrl_iface_deinit(struct hostapd_data *hapd)
{
	struct wpa_ctrl_dst *dst, *prev;

	if (hapd->ctrl_sock > -1) {
		char *fname;
		eloop_unregister_read_sock(hapd->ctrl_sock);
		close(hapd->ctrl_sock);
		hapd->ctrl_sock = -1;
		fname = hostapd_ctrl_iface_path(hapd);
		if (fname)
			unlink(fname);
		os_free(fname);

		if (hapd->conf->ctrl_interface &&
		    rmdir(hapd->conf->ctrl_interface) < 0) {
			if (errno == ENOTEMPTY) {
				wpa_printf(MSG_DEBUG, "Control interface "
					   "directory not empty - leaving it "
					   "behind");
			} else {
				wpa_printf(MSG_ERROR,
					   "rmdir[ctrl_interface=%s]: %s",
					   hapd->conf->ctrl_interface,
					   strerror(errno));
			}
		}
	}

	dst = hapd->ctrl_dst;
	hapd->ctrl_dst = NULL;
	while (dst) {
		prev = dst;
		dst = dst->next;
		os_free(prev);
	}

#ifdef CONFIG_TESTING_OPTIONS
	l2_packet_deinit(hapd->l2_test);
	hapd->l2_test = NULL;
#endif /* CONFIG_TESTING_OPTIONS */
}


static int hostapd_ctrl_iface_add(struct hapd_interfaces *interfaces,
				  char *buf)
{
	if (hostapd_add_iface(interfaces, buf) < 0) {
		wpa_printf(MSG_ERROR, "Adding interface %s failed", buf);
		return -1;
	}
	return 0;
}


static int hostapd_ctrl_iface_remove(struct hapd_interfaces *interfaces,
				     char *buf)
{
	if (hostapd_remove_iface(interfaces, buf) < 0) {
		wpa_printf(MSG_ERROR, "Removing interface %s failed", buf);
		return -1;
	}
	return 0;
}


static int hostapd_global_ctrl_iface_attach(struct hapd_interfaces *interfaces,
					    struct sockaddr_un *from,
					    socklen_t fromlen)
{
	struct wpa_ctrl_dst *dst;

	dst = os_zalloc(sizeof(*dst));
	if (dst == NULL)
		return -1;
	os_memcpy(&dst->addr, from, sizeof(struct sockaddr_un));
	dst->addrlen = fromlen;
	dst->debug_level = MSG_INFO;
	dst->next = interfaces->global_ctrl_dst;
	interfaces->global_ctrl_dst = dst;
	wpa_hexdump(MSG_DEBUG, "CTRL_IFACE monitor attached (global)",
		    from->sun_path,
		    fromlen - offsetof(struct sockaddr_un, sun_path));
	return 0;
}


static int hostapd_global_ctrl_iface_detach(struct hapd_interfaces *interfaces,
					    struct sockaddr_un *from,
					    socklen_t fromlen)
{
	struct wpa_ctrl_dst *dst, *prev = NULL;

	dst = interfaces->global_ctrl_dst;
	while (dst) {
		if (fromlen == dst->addrlen &&
		    os_memcmp(from->sun_path, dst->addr.sun_path,
			      fromlen - offsetof(struct sockaddr_un, sun_path))
		    == 0) {
			wpa_hexdump(MSG_DEBUG,
				    "CTRL_IFACE monitor detached (global)",
				    from->sun_path,
				    fromlen -
				    offsetof(struct sockaddr_un, sun_path));
			if (prev == NULL)
				interfaces->global_ctrl_dst = dst->next;
			else
				prev->next = dst->next;
			os_free(dst);
			return 0;
		}
		prev = dst;
		dst = dst->next;
	}
	return -1;
}


static void hostapd_ctrl_iface_flush(struct hapd_interfaces *interfaces)
{
#ifdef CONFIG_WPS_TESTING
	wps_version_number = 0x20;
	wps_testing_dummy_cred = 0;
	wps_corrupt_pkhash = 0;
#endif /* CONFIG_WPS_TESTING */
}


#ifdef CONFIG_FST

static int
hostapd_global_ctrl_iface_fst_attach(struct hapd_interfaces *interfaces,
				     const char *cmd)
{
	char ifname[IFNAMSIZ + 1];
	struct fst_iface_cfg cfg;
	struct hostapd_data *hapd;
	struct fst_wpa_obj iface_obj;

	if (!fst_parse_attach_command(cmd, ifname, sizeof(ifname), &cfg)) {
		hapd = hostapd_get_iface(interfaces, ifname);
		if (hapd) {
			if (hapd->iface->fst) {
				wpa_printf(MSG_INFO, "FST: Already attached");
				return -1;
			}
			fst_hostapd_fill_iface_obj(hapd, &iface_obj);
			hapd->iface->fst = fst_attach(ifname, hapd->own_addr,
						      &iface_obj, &cfg);
			if (hapd->iface->fst)
				return 0;
		}
	}

	return -EINVAL;
}


static int
hostapd_global_ctrl_iface_fst_detach(struct hapd_interfaces *interfaces,
				     const char *cmd)
{
	char ifname[IFNAMSIZ + 1];
	struct hostapd_data * hapd;

	if (!fst_parse_detach_command(cmd, ifname, sizeof(ifname))) {
		hapd = hostapd_get_iface(interfaces, ifname);
		if (hapd) {
			if (!fst_iface_detach(ifname)) {
				hapd->iface->fst = NULL;
				hapd->iface->fst_ies = NULL;
				return 0;
			}
		}
	}

	return -EINVAL;
}

#endif /* CONFIG_FST */


static struct hostapd_data *
hostapd_interfaces_get_hapd(struct hapd_interfaces *interfaces,
			    const char *ifname)
{
	size_t i, j;

	for (i = 0; i < interfaces->count; i++) {
		struct hostapd_iface *iface = interfaces->iface[i];

		for (j = 0; j < iface->num_bss; j++) {
			struct hostapd_data *hapd;

			hapd = iface->bss[j];
			if (os_strcmp(ifname, hapd->conf->iface) == 0)
				return hapd;
		}
	}

	return NULL;
}


static int hostapd_ctrl_iface_dup_param(struct hostapd_data *src_hapd,
					struct hostapd_data *dst_hapd,
					const char *param)
{
	int res;
	char *value;

	value = os_zalloc(HOSTAPD_CLI_DUP_VALUE_MAX_LEN);
	if (!value) {
		wpa_printf(MSG_ERROR,
			   "DUP: cannot allocate buffer to stringify %s",
			   param);
		goto error_return;
	}

	if (os_strcmp(param, "wpa") == 0) {
		os_snprintf(value, HOSTAPD_CLI_DUP_VALUE_MAX_LEN, "%d",
			    src_hapd->conf->wpa);
	} else if (os_strcmp(param, "wpa_key_mgmt") == 0 &&
		   src_hapd->conf->wpa_key_mgmt) {
		res = hostapd_ctrl_iface_get_key_mgmt(
			src_hapd, value, HOSTAPD_CLI_DUP_VALUE_MAX_LEN);
		if (os_snprintf_error(HOSTAPD_CLI_DUP_VALUE_MAX_LEN, res))
			goto error_stringify;
	} else if (os_strcmp(param, "wpa_pairwise") == 0 &&
		   src_hapd->conf->wpa_pairwise) {
		res = wpa_write_ciphers(value,
					value + HOSTAPD_CLI_DUP_VALUE_MAX_LEN,
					src_hapd->conf->wpa_pairwise, " ");
		if (res < 0)
			goto error_stringify;
	} else if (os_strcmp(param, "rsn_pairwise") == 0 &&
		   src_hapd->conf->rsn_pairwise) {
		res = wpa_write_ciphers(value,
					value + HOSTAPD_CLI_DUP_VALUE_MAX_LEN,
					src_hapd->conf->rsn_pairwise, " ");
		if (res < 0)
			goto error_stringify;
	} else if (os_strcmp(param, "wpa_passphrase") == 0 &&
		   src_hapd->conf->ssid.wpa_passphrase) {
		os_snprintf(value, HOSTAPD_CLI_DUP_VALUE_MAX_LEN, "%s",
			    src_hapd->conf->ssid.wpa_passphrase);
	} else if (os_strcmp(param, "wpa_psk") == 0 &&
		   src_hapd->conf->ssid.wpa_psk_set) {
		wpa_snprintf_hex(value, HOSTAPD_CLI_DUP_VALUE_MAX_LEN,
			src_hapd->conf->ssid.wpa_psk->psk, PMK_LEN);
	} else {
		wpa_printf(MSG_WARNING, "DUP: %s cannot be duplicated", param);
		goto error_return;
	}

	res = hostapd_set_iface(dst_hapd->iconf, dst_hapd->conf, param, value);
	os_free(value);
	return res;

error_stringify:
	wpa_printf(MSG_ERROR, "DUP: cannot stringify %s", param);
error_return:
	os_free(value);
	return -1;
}


static int
hostapd_global_ctrl_iface_dup_network(struct hapd_interfaces *interfaces,
				      char *cmd)
{
	char *p_start = cmd, *p_end;
	struct hostapd_data *src_hapd, *dst_hapd;

	/* cmd: "<src ifname> <dst ifname> <variable name> */

	p_end = os_strchr(p_start, ' ');
	if (!p_end) {
		wpa_printf(MSG_ERROR, "DUP: no src ifname found in cmd: '%s'",
			   cmd);
		return -1;
	}

	*p_end = '\0';
	src_hapd = hostapd_interfaces_get_hapd(interfaces, p_start);
	if (!src_hapd) {
		wpa_printf(MSG_ERROR, "DUP: no src ifname found: '%s'",
			   p_start);
		return -1;
	}

	p_start = p_end + 1;
	p_end = os_strchr(p_start, ' ');
	if (!p_end) {
		wpa_printf(MSG_ERROR, "DUP: no dst ifname found in cmd: '%s'",
			   cmd);
		return -1;
	}

	*p_end = '\0';
	dst_hapd = hostapd_interfaces_get_hapd(interfaces, p_start);
	if (!dst_hapd) {
		wpa_printf(MSG_ERROR, "DUP: no dst ifname found: '%s'",
			   p_start);
		return -1;
	}

	p_start = p_end + 1;
	return hostapd_ctrl_iface_dup_param(src_hapd, dst_hapd, p_start);
}


static int hostapd_global_ctrl_iface_ifname(struct hapd_interfaces *interfaces,
					    const char *ifname,
					    char *buf, char *reply,
					    int reply_size,
					    struct sockaddr_un *from,
					    socklen_t fromlen)
{
	struct hostapd_data *hapd;

	hapd = hostapd_interfaces_get_hapd(interfaces, ifname);
	if (hapd == NULL) {
		int res;

		res = os_snprintf(reply, reply_size, "FAIL-NO-IFNAME-MATCH\n");
		if (os_snprintf_error(reply_size, res))
			return -1;
		return res;
	}

	return hostapd_ctrl_iface_receive_process(hapd, buf, reply,reply_size,
						  from, fromlen);
}


static void hostapd_global_ctrl_iface_receive(int sock, void *eloop_ctx,
					      void *sock_ctx)
{
	void *interfaces = eloop_ctx;
	char buf[256];
	int res;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);
	char *reply;
	int reply_len;
	const int reply_size = 4096;

	res = recvfrom(sock, buf, sizeof(buf) - 1, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		wpa_printf(MSG_ERROR, "recvfrom(ctrl_iface): %s",
			   strerror(errno));
		return;
	}
	buf[res] = '\0';
	wpa_printf(MSG_DEBUG, "Global ctrl_iface command: %s", buf);

	reply = os_malloc(reply_size);
	if (reply == NULL) {
		if (sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *) &from,
			   fromlen) < 0) {
			wpa_printf(MSG_DEBUG, "CTRL: sendto failed: %s",
				   strerror(errno));
		}
		return;
	}

	os_memcpy(reply, "OK\n", 3);
	reply_len = 3;

	if (os_strncmp(buf, "IFNAME=", 7) == 0) {
		char *pos = os_strchr(buf + 7, ' ');

		if (pos) {
			*pos++ = '\0';
			reply_len = hostapd_global_ctrl_iface_ifname(
				interfaces, buf + 7, pos, reply, reply_size,
				&from, fromlen);
			goto send_reply;
		}
	}

	if (os_strcmp(buf, "PING") == 0) {
		os_memcpy(reply, "PONG\n", 5);
		reply_len = 5;
	} else if (os_strncmp(buf, "RELOG", 5) == 0) {
		if (wpa_debug_reopen_file() < 0)
			reply_len = -1;
	} else if (os_strcmp(buf, "FLUSH") == 0) {
		hostapd_ctrl_iface_flush(interfaces);
	} else if (os_strncmp(buf, "ADD ", 4) == 0) {
		if (hostapd_ctrl_iface_add(interfaces, buf + 4) < 0)
			reply_len = -1;
	} else if (os_strncmp(buf, "REMOVE ", 7) == 0) {
		if (hostapd_ctrl_iface_remove(interfaces, buf + 7) < 0)
			reply_len = -1;
	} else if (os_strcmp(buf, "ATTACH") == 0) {
		if (hostapd_global_ctrl_iface_attach(interfaces, &from,
						     fromlen))
			reply_len = -1;
	} else if (os_strcmp(buf, "DETACH") == 0) {
		if (hostapd_global_ctrl_iface_detach(interfaces, &from,
			fromlen))
			reply_len = -1;
#ifdef CONFIG_MODULE_TESTS
	} else if (os_strcmp(buf, "MODULE_TESTS") == 0) {
		int hapd_module_tests(void);
		if (hapd_module_tests() < 0)
			reply_len = -1;
#endif /* CONFIG_MODULE_TESTS */
#ifdef CONFIG_FST
	} else if (os_strncmp(buf, "FST-ATTACH ", 11) == 0) {
		if (!hostapd_global_ctrl_iface_fst_attach(interfaces, buf + 11))
			reply_len = os_snprintf(reply, reply_size, "OK\n");
		else
			reply_len = -1;
	} else if (os_strncmp(buf, "FST-DETACH ", 11) == 0) {
		if (!hostapd_global_ctrl_iface_fst_detach(interfaces, buf + 11))
			reply_len = os_snprintf(reply, reply_size, "OK\n");
		else
			reply_len = -1;
	} else if (os_strncmp(buf, "FST-MANAGER ", 12) == 0) {
		reply_len = fst_ctrl_iface_receive(buf + 12, reply, reply_size);
#endif /* CONFIG_FST */
	} else if (os_strncmp(buf, "DUP_NETWORK ", 12) == 0) {
		if (!hostapd_global_ctrl_iface_dup_network(interfaces,
							   buf + 12))
			reply_len = os_snprintf(reply, reply_size, "OK\n");
		else
			reply_len = -1;
	} else {
		wpa_printf(MSG_DEBUG, "Unrecognized global ctrl_iface command "
			   "ignored");
		reply_len = -1;
	}

send_reply:
	if (reply_len < 0) {
		os_memcpy(reply, "FAIL\n", 5);
		reply_len = 5;
	}

	if (sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from,
		   fromlen) < 0) {
		wpa_printf(MSG_DEBUG, "CTRL: sendto failed: %s",
			   strerror(errno));
	}
	os_free(reply);
}


static char * hostapd_global_ctrl_iface_path(struct hapd_interfaces *interface)
{
	char *buf;
	size_t len;

	if (interface->global_iface_path == NULL)
		return NULL;

	len = os_strlen(interface->global_iface_path) +
		os_strlen(interface->global_iface_name) + 2;
	buf = os_malloc(len);
	if (buf == NULL)
		return NULL;

	os_snprintf(buf, len, "%s/%s", interface->global_iface_path,
		    interface->global_iface_name);
	buf[len - 1] = '\0';
	return buf;
}


int hostapd_global_ctrl_iface_init(struct hapd_interfaces *interface)
{
	struct sockaddr_un addr;
	int s = -1;
	char *fname = NULL;

	if (interface->global_iface_path == NULL) {
		wpa_printf(MSG_DEBUG, "ctrl_iface not configured!");
		return 0;
	}

	if (mkdir(interface->global_iface_path, S_IRWXU | S_IRWXG) < 0) {
		if (errno == EEXIST) {
			wpa_printf(MSG_DEBUG, "Using existing control "
				   "interface directory.");
		} else {
			wpa_printf(MSG_ERROR, "mkdir[ctrl_interface]: %s",
				   strerror(errno));
			goto fail;
		}
	} else if (interface->ctrl_iface_group &&
		   chown(interface->global_iface_path, -1,
			 interface->ctrl_iface_group) < 0) {
		wpa_printf(MSG_ERROR, "chown[ctrl_interface]: %s",
			   strerror(errno));
		goto fail;
	}

	if (os_strlen(interface->global_iface_path) + 1 +
	    os_strlen(interface->global_iface_name) >= sizeof(addr.sun_path))
		goto fail;

	s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (s < 0) {
		wpa_printf(MSG_ERROR, "socket(PF_UNIX): %s", strerror(errno));
		goto fail;
	}

	os_memset(&addr, 0, sizeof(addr));
#ifdef __FreeBSD__
	addr.sun_len = sizeof(addr);
#endif /* __FreeBSD__ */
	addr.sun_family = AF_UNIX;
	fname = hostapd_global_ctrl_iface_path(interface);
	if (fname == NULL)
		goto fail;
	os_strlcpy(addr.sun_path, fname, sizeof(addr.sun_path));
	if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		wpa_printf(MSG_DEBUG, "ctrl_iface bind(PF_UNIX) failed: %s",
			   strerror(errno));
		if (connect(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			wpa_printf(MSG_DEBUG, "ctrl_iface exists, but does not"
				   " allow connections - assuming it was left"
				   "over from forced program termination");
			if (unlink(fname) < 0) {
				wpa_printf(MSG_ERROR,
					   "Could not unlink existing ctrl_iface socket '%s': %s",
					   fname, strerror(errno));
				goto fail;
			}
			if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) <
			    0) {
				wpa_printf(MSG_ERROR, "bind(PF_UNIX): %s",
					   strerror(errno));
				goto fail;
			}
			wpa_printf(MSG_DEBUG, "Successfully replaced leftover "
				   "ctrl_iface socket '%s'", fname);
		} else {
			wpa_printf(MSG_INFO, "ctrl_iface exists and seems to "
				   "be in use - cannot override it");
			wpa_printf(MSG_INFO, "Delete '%s' manually if it is "
				   "not used anymore", fname);
			os_free(fname);
			fname = NULL;
			goto fail;
		}
	}

	if (interface->ctrl_iface_group &&
	    chown(fname, -1, interface->ctrl_iface_group) < 0) {
		wpa_printf(MSG_ERROR, "chown[ctrl_interface]: %s",
			   strerror(errno));
		goto fail;
	}

	if (chmod(fname, S_IRWXU | S_IRWXG) < 0) {
		wpa_printf(MSG_ERROR, "chmod[ctrl_interface/ifname]: %s",
			   strerror(errno));
		goto fail;
	}
	os_free(fname);

	interface->global_ctrl_sock = s;
	eloop_register_read_sock(s, hostapd_global_ctrl_iface_receive,
				 interface, NULL);

	return 0;

fail:
	if (s >= 0)
		close(s);
	if (fname) {
		unlink(fname);
		os_free(fname);
	}
	return -1;
}


void hostapd_global_ctrl_iface_deinit(struct hapd_interfaces *interfaces)
{
	char *fname = NULL;
	struct wpa_ctrl_dst *dst, *prev;

	if (interfaces->global_ctrl_sock > -1) {
		eloop_unregister_read_sock(interfaces->global_ctrl_sock);
		close(interfaces->global_ctrl_sock);
		interfaces->global_ctrl_sock = -1;
		fname = hostapd_global_ctrl_iface_path(interfaces);
		if (fname) {
			unlink(fname);
			os_free(fname);
		}

		if (interfaces->global_iface_path &&
		    rmdir(interfaces->global_iface_path) < 0) {
			if (errno == ENOTEMPTY) {
				wpa_printf(MSG_DEBUG, "Control interface "
					   "directory not empty - leaving it "
					   "behind");
			} else {
				wpa_printf(MSG_ERROR,
					   "rmdir[ctrl_interface=%s]: %s",
					   interfaces->global_iface_path,
					   strerror(errno));
			}
		}
	}

	os_free(interfaces->global_iface_path);
	interfaces->global_iface_path = NULL;

	dst = interfaces->global_ctrl_dst;
	interfaces->global_ctrl_dst = NULL;
	while (dst) {
		prev = dst;
		dst = dst->next;
		os_free(prev);
	}
}


static void hostapd_ctrl_iface_send(struct hostapd_data *hapd, int level,
				    enum wpa_msg_type type,
				    const char *buf, size_t len)
{
	struct wpa_ctrl_dst *dst, *next;
	struct msghdr msg;
	int idx;
	struct iovec io[2];
	char levelstr[10];
	int s;

	if (type != WPA_MSG_ONLY_GLOBAL) {
		s = hapd->ctrl_sock;
		dst = hapd->ctrl_dst;
	} else {
		s = hapd->iface->interfaces->global_ctrl_sock;
		dst = hapd->iface->interfaces->global_ctrl_dst;
	}

	if (s < 0 || dst == NULL)
		return;

	os_snprintf(levelstr, sizeof(levelstr), "<%d>", level);
	io[0].iov_base = levelstr;
	io[0].iov_len = os_strlen(levelstr);
	io[1].iov_base = (char *) buf;
	io[1].iov_len = len;
	os_memset(&msg, 0, sizeof(msg));
	msg.msg_iov = io;
	msg.msg_iovlen = 2;

	idx = 0;
	while (dst) {
		next = dst->next;
		if (level >= dst->debug_level) {
			wpa_hexdump(MSG_DEBUG, "CTRL_IFACE monitor send",
				    (u8 *) dst->addr.sun_path, dst->addrlen -
				    offsetof(struct sockaddr_un, sun_path));
			msg.msg_name = &dst->addr;
			msg.msg_namelen = dst->addrlen;
			if (sendmsg(s, &msg, 0) < 0) {
				int _errno = errno;
				wpa_printf(MSG_INFO, "CTRL_IFACE monitor[%d]: "
					   "%d - %s",
					   idx, errno, strerror(errno));
				dst->errors++;
				if (dst->errors > 10 || _errno == ENOENT) {
					if (type != WPA_MSG_ONLY_GLOBAL)
						hostapd_ctrl_iface_detach(
							hapd, &dst->addr,
							dst->addrlen);
					else
						hostapd_global_ctrl_iface_detach(
							hapd->iface->interfaces,
							&dst->addr,
							dst->addrlen);
				}
			} else
				dst->errors = 0;
		}
		idx++;
		dst = next;
	}
}

#endif /* CONFIG_NATIVE_WINDOWS */
