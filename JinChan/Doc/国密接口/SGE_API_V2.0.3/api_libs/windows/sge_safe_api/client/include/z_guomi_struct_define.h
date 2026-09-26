#ifndef  Z_GUOMI_STRUCT_DEFINE_H
#define  Z_GUOMI_STRUCT_DEFINE_H

typedef struct client_hello_s{
    char       trader_id_[16];
    char       sge_id_[16];
    long long  enc_mode_;
    char       safe_api_trader_version_[128];
} fld_client_hello_t;

typedef struct server_hello_s{
    char       trader_id_[16];
    char       sge_id_[16];
    char       sge_random_[64];
    long long  enc_mode_;
} fld_server_hello_t;

typedef struct authentication_info_s{
    char       trader_id_[16];
    char       sge_id_[16];
    char       trader_random_[64];
    char       sge_random_[64];
    char       authen_signed_data_[256];
} fld_authentication_info_t;

typedef struct sessionkey_info_s{
    char       trader_id_[16];
    char       sge_id_[16];
    char       cipher_key_[256];
    char       enc_random_[128];
    char       skey_signed_data_[256];
} fld_sessionkey_info_t;

#endif   /* Z_GUOMI_STRUCT_DEFINE_H */
