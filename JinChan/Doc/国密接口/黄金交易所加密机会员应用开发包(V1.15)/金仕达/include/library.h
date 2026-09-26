


typedef void* (*pcb_connect_)(void *ctx);
typedef int (*pcb_disconnect_)(void *handle);
typedef int (*pcb_reconnect_)(void *handle);
typedef int (*pcb_reset_connect_)(void *handle);
typedef int (*pcb_is_connected_)(void *handle);
typedef int (*pcb_encrypt_)(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);
typedef int (*pcb_decrypt_)(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);
typedef int (*pcb_mac_)(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);
typedef int (*pcb_hash_)(void *handle, const unsigned char *data, const unsigned int data_len, unsigned char *hash, unsigned int *hash_cap);
typedef int (*pcb_sign_)(void *handle, const void *sign_keypair, const unsigned char *data, const unsigned int data_len, unsigned char *signed_data, unsigned int *cap);
typedef int (*pcb_verify_sign_)(void *handle, const unsigned char *data, const unsigned int data_len, const unsigned char *signed_data, const unsigned int signed_data_len, const unsigned char *pub_key, const unsigned int pub_key_len);
typedef int (*pcb_get_skey_)(void *handle , const char *idx, const unsigned char *cipher_key, const unsigned int cipher_key_len, unsigned char *skey, unsigned int *cap);
typedef int (*pcb_gen_random_)(void *handle, const unsigned int len, unsigned char *random);
typedef int (*pcb_get_enc_keypair_)(void *handle , const char *idx, void **enc_keypair);
typedef int (*pcb_free_enc_keypair_)(void *handle, void *enc_keypair);
typedef int (*pcb_get_sign_keypair_)(void *handle, const char *idx, void **sign_keypair);
typedef int (*pcb_free_sign_keypair_)(void *handle, void *sign_keypair);

pcb_connect_         m_cb_connect_;
pcb_disconnect_      m_cb_disconnect_;
pcb_reconnect_       m_cb_reconnect_;
pcb_reset_connect_   m_cb_reset_connect_;
pcb_is_connected_    m_cb_is_connected_;
pcb_encrypt_         m_cb_encrypt_;
pcb_decrypt_         m_cb_decrypt_;
pcb_mac_             m_cb_mac_;
pcb_hash_		    m_cb_hash_;
pcb_sign_            m_cb_sign_;
pcb_verify_sign_     m_cb_verify_sign_;
pcb_get_skey_	    m_cb_get_skey_;
pcb_gen_random_	    m_cb_gen_random_;
pcb_get_enc_keypair_	m_cb_get_enc_keypair_;
pcb_free_enc_keypair_	m_cb_free_enc_keypair_;
pcb_get_sign_keypair_	m_cb_get_sign_keypair_;
pcb_free_sign_keypair_	m_cb_free_sign_keypair_;



