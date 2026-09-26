#ifndef __SGE_SAFE_CLIENT_INTERFACE_H__
#define __SGE_SAFE_CLIENT_INTERFACE_H__

#include "sge_packet_struct.h"
#include "client_cb_list_define.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef WIN32

#ifdef DLL_EXPORT
#define DLL_DECLARE  __declspec(dllexport)
#else
#define DLL_DECLARE  __declspec(dllimport)
#endif /* DLL_EXPORT */

#else //linux
#define DLL_DECLARE
#endif /* WIN32 */

typedef struct cli_safe_handle_s api_safe_client_t;

DLL_DECLARE api_safe_client_t *sge_safe_client_api_create(client_dev_cb_list_t *data, const int client_enc_mode);
DLL_DECLARE int sge_safe_client_api_destroy(api_safe_client_t *ptr);

typedef struct client_info_s {
	char sge_id_[16];
	char trader_id_[16];
	int enc_mode_;
	unsigned char svr_enc_pubkey_[4096];
	unsigned int svr_enc_pubkey_len_;
	unsigned char svr_sign_pubkey_[4096];
	unsigned int svr_sign_pubkey_len_;
}session_client_info_t;

typedef struct cli_session_handle_s api_safe_client_session_t;
DLL_DECLARE api_safe_client_session_t *sge_safe_create_client_session(api_safe_client_t *ptr, const session_client_info_t *info);
DLL_DECLARE int sge_safe_free_client_session(api_safe_client_session_t *ptr);

DLL_DECLARE int sge_safe_client_create_hello(api_safe_client_session_t *ptr, fld_client_hello_t *msg);

typedef struct cli_link_s api_safe_client_link_t;
DLL_DECLARE int sge_safe_client_handle_hello_rsp(api_safe_client_session_t *ptr, api_safe_client_link_t *link, const fld_server_hello_t *msg);

DLL_DECLARE int sge_safe_client_create_authentication(api_safe_client_session_t *ptr, api_safe_client_link_t *link, fld_authentication_info_t *msg);

DLL_DECLARE int sge_safe_client_accept_sessionkey(api_safe_client_session_t *ptr, api_safe_client_link_t *link, const fld_sessionkey_info_t *msg);

DLL_DECLARE int sge_safe_client_packet_encrypt(api_safe_client_session_t *ptr, api_safe_client_link_t *link, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);

DLL_DECLARE int sge_safe_client_packet_decrypt(api_safe_client_session_t *ptr, api_safe_client_link_t *link, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);

DLL_DECLARE char *sge_safe_client_get_api_version(void);

DLL_DECLARE api_safe_client_link_t *sge_safe_client_get_session_current_used_in_link(api_safe_client_session_t *ptr, api_safe_client_link_t *link);
DLL_DECLARE api_safe_client_link_t *sge_safe_client_link_create(api_safe_client_t *ptr);
DLL_DECLARE int sge_safe_client_link_destroy(api_safe_client_link_t *ptr);
DLL_DECLARE int sge_safe_client_link_reset(api_safe_client_link_t *ptr);
DLL_DECLARE int sge_safe_client_link_reconnect(api_safe_client_link_t *ptr);
DLL_DECLARE int sge_safe_client_link_is_connected(api_safe_client_link_t *ptr);
DLL_DECLARE int sge_safe_client_link_get_mode(api_safe_client_link_t *ptr);


#ifdef  __cplusplus
}
#endif  /*  end of __cplusplus */

#endif
