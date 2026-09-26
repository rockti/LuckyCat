#ifndef __CLIENT_DEV_CB_DEFINE_H__
#define __CLIENT_DEV_CB_DEFINE_H__


typedef void *(*cli_cb_connect_t)(void *ctx);
typedef int (*cli_cb_disconnect_t)(void *handle);
typedef int (*cli_cb_reconnect_t)(void *handle);
typedef int (*cli_cb_reset_connect_t)(void *handle);
typedef int (*cli_cb_is_connected_t)(void *handle);
typedef int (*cli_cb_encrypt_data_t)(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);
typedef int (*cli_cb_decrypt_data_t)(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);
typedef int (*cli_cb_mac_data_t)(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);
typedef int (*cli_cb_hash_data_t)(void *handle, const unsigned char *data, const unsigned int data_len, unsigned char *hash, unsigned int *hash_cap);
typedef int (*cli_cb_sign_data_t)(void *handle, const void *sign_keypair, const unsigned char *data, const unsigned int data_len, unsigned char *signed_data, unsigned int *cap);
typedef int (*cli_cb_verify_signed_data_t)(void *handle, const unsigned char *data, const unsigned int data_len, const unsigned char *signed_data, const unsigned int signed_data_len, const unsigned char *pub_key, const unsigned int pub_key_len);
typedef int (*cli_cb_get_skey_t)(void *handle, const char *idx, const unsigned char *cipher_key, const unsigned int cipher_key_len, unsigned char *skey, unsigned int *cap);
typedef int (*cli_cb_gen_random_t)(void *handle, const unsigned int len, unsigned char *random);
typedef int (*cli_cb_get_enc_keypair_t)(void *handle, const char *idx, void **enc_keypair);
typedef int (*cli_cb_get_sign_keypair_t)(void *handle, const char *idx, void **sign_keypair);
typedef int (*cli_cb_free_enc_keypair_t)(void *handle, void *enc_keypair);
typedef int (*cli_cb_free_sign_keypair_t)(void *handle, void *sign_keypair);


typedef struct client_dev_cb_list_s {
	cli_cb_connect_t cb_connect_;
	cli_cb_disconnect_t cb_disconnect_;
	cli_cb_reconnect_t cb_reconnect_;
    cli_cb_reset_connect_t cb_reset_connect_;
	cli_cb_is_connected_t cb_is_connected_;
	cli_cb_encrypt_data_t cb_encrypt_;
	cli_cb_decrypt_data_t cb_decrypt_;
	cli_cb_mac_data_t cb_mac_;
	cli_cb_hash_data_t cb_hash_;
	cli_cb_sign_data_t cb_sign_;
	cli_cb_verify_signed_data_t cb_verify_sign_;
	cli_cb_gen_random_t cb_gen_random_;
	cli_cb_get_skey_t cb_get_skey_;
	cli_cb_get_enc_keypair_t cb_get_enc_keypair_;
	cli_cb_free_enc_keypair_t cb_free_enc_keypair_;
	cli_cb_get_sign_keypair_t cb_get_sign_keypair_;
	cli_cb_free_sign_keypair_t cb_free_sign_keypair_;
	void *ctx_;
}client_dev_cb_list_t;

#endif 
