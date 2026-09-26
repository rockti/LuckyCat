#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sge_safe_client_interface.h"
#include "api.h"


//SgeSafeApiDemo

const char *g_sge_id = "sge";
const char *g_trader_id = "demo_trader";

#define ENC_MODE_HARD 0
#define ENC_MODE_SOFT 1
#define ENCRYPTOR_IP "127.0.0.1"
#define ENCRYPTOR_PORT 6666
#define TIME_OUT  1
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888 

typedef struct encryptor_ctx_s{
    char encryptor_ip_[16];
    int port_;
    int timeout_;
    int other_infomation_;
    //extension
}encryptor_ctx_t;

typedef struct api_ctx_s{
    api_safe_client_t *safe_api_;
    api_safe_client_session_t *safe_sess_;
    api_safe_client_link_t *safe_link_;
    int shakehand_state_;
}api_ctx_t;


/* encryptor callback function define
 *==============================================================================================*/
unsigned char *__do_real_connect_operation(char *ip, int port, int other_infomation){
    return (unsigned char*)malloc(1);
}
int __do_real_disconnect_operation(void *handle){
    return 0;
}

void *cb_connect(void *ctx){
	encryptor_ctx_t *encryptor_ctx = (encryptor_ctx_t*)ctx;
	if(!encryptor_ctx )
		return NULL;

	printf("connect to device, encryptor_ip:%s, port:%d\n", encryptor_ctx->encryptor_ip_, encryptor_ctx->port_);
	void *p = (void *)__do_real_connect_operation(encryptor_ctx->encryptor_ip_, encryptor_ctx->port_, encryptor_ctx->other_infomation_);
	return p;
}
int cb_disconnect(void *handle){
    if(!handle)
        return -1;

    printf("disconnect\n");
    __do_real_disconnect_operation(handle);
    
    free(handle);

    return 0;
}
int cb_reconnect(void *handle){
    if(!handle)
        return -1;

    printf("reconnect\n");
    return 0;
}
int cb_reset_connect(void *handle){
    if(!handle)
        return -1;

    printf("reset_connect\n");
    return 0;
}
int cb_is_connected(void *handle){
    if(!handle)
        return -1;

    printf("is_connected\n");
    return 0;
}
int cb_encrypt_data(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap){
    if(handle == NULL || skey == NULL || src == NULL || dst == NULL || cap == NULL)
        return -1;
    printf("dev_encrypt_data\n");
    //Todo: real encrypt action

    return 0;
}
int cb_decrypt_data(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap){
    if(handle == NULL || skey == NULL || src == NULL || dst == NULL || cap == NULL)
        return -1;
    printf("dev_decrypt_data\n");
    //Todo: real decrypt action

    return 0;
}
int cb_mac_data(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap){
    if(handle == NULL || skey == NULL || src == NULL || dst == NULL || cap == NULL)
        return -1;
    printf("dev_mac_data\n");
    //Todo: real mac action

    return 0;
}
int cb_hash_data(void *handle, const unsigned char *data, const unsigned int data_len, unsigned char *hash, unsigned int *hash_cap){
    if(handle == NULL || data == NULL || hash == NULL) 
        return -1;
    printf("hash data\n");
    //Todo: real hash action

    return 0;
}
int cb_sign_data(void *handle, const void *sign_keypair, const unsigned char *data, const unsigned int data_len, unsigned char *signed_data, unsigned int *cap){
    if(handle == NULL || sign_keypair == NULL || data == NULL || signed_data == NULL || cap == NULL)
        return -1;
    printf("dev_sign_data\n");
    //Todo: real sign action

    return 0;
}
int cb_verify_signed_data(void *handle, const unsigned char *data, const unsigned int data_len, const unsigned char *signed_data, unsigned int signed_data_len, const unsigned char *pub_key, const unsigned int pub_key_len){
    if(handle == NULL || data == NULL || signed_data == NULL || pub_key == NULL)
        return -1;
    printf("dev_verify_signed_data\n");
    //Todo: real verify sign action

    return 0;
}
int cb_get_skey(void *handle, const char *idx, const unsigned char *cipher_key, const unsigned int cipher_key_len, unsigned char *skey, unsigned int *cap){
	if(handle == NULL || idx == NULL || cipher_key == NULL || skey == NULL || cap == NULL)
		return -1;
	printf("dev_get_skey\n");
	//Todo: real get_skey action

	return 0;
}
int cb_gen_random(void *handle, const unsigned int len, unsigned char *random){
    if(handle == NULL || random == NULL)
        return -1;
    printf("dev_gen_random\n");
    //Todo: real gen_random action

    return 0;
}

typedef struct keypair_s{
    char keypair_information[1];
}keypair_t;
int __do_real_get_enc_keypair(void *handle, const char *idx, char *keypair_information)
{
    printf("__do_real_get_enc_keypair\n");
    return 0;
}
int cb_get_enc_keypair(void *handle, const char *idx, void **enc_keypair){
    if(handle == NULL || idx == NULL || enc_keypair == NULL)
        return -1;
    printf("dev_get_enc_keypair\n");

    *enc_keypair = calloc(1, sizeof(keypair_t));
    //Do real get enc_keypair
    int ret = __do_real_get_enc_keypair(handle, idx, ((keypair_t *)(*enc_keypair))->keypair_information);

    return ret;
}
int cb_free_enc_keypair(void *handle, void *enc_keypair){
    if(handle == NULL || enc_keypair != NULL) 
        return -1;
    free(enc_keypair);

    printf("dev_free_enk_keypair\n");

    return 0;
}

int __do_real_get_sign_keypair(void *handle, const char *idx, char *keypair_information)
{
    printf("__do_real_get_sign_keypair\n");
    return 0;
}
int cb_get_sign_keypair(void *handle, const char *idx, void **sign_keypair){
    if(handle == NULL || idx == NULL || sign_keypair == NULL)
        return -1;
    printf("dev_get_sign_keypair\n");

    *sign_keypair = calloc(1, sizeof(keypair_t));
    int ret = __do_real_get_sign_keypair(handle, idx, ((keypair_t *)(*sign_keypair))->keypair_information);

    return ret;
}
int cb_free_sign_keypair(void *handle, void *sign_keypair){
    if(handle == NULL || sign_keypair != NULL) 
        return -1;
    free(sign_keypair);
    printf("dev_get_sign_keypair\n");

    return 0;
}
/*==============================================================================================*/

int get_svr_msg_type(const char *msg, unsigned int con_len){
    // get gtp msg type

    return 0;
}

#define HANDSHAKED 1
int __is_safe_handshake_finished(api_ctx_t ctx){
    if(ctx.shakehand_state_ == HANDSHAKED)
        return 1;
    return 0;
}
fld_server_hello_t *gtp2gems_server_hello(const char *msg, unsigned int con_len){
    // convert gtp to fld_server_hello_t

    // mock
    fld_server_hello_t *svr_hello = (fld_server_hello_t*)malloc(sizeof(fld_server_hello_t));

    return svr_hello;
}
int handle_svr_hello(api_safe_client_session_t *ptr, api_safe_client_link_t *enc_link, const char *msg, unsigned int con_len){
    // 1. gtp->gems: server hello
    fld_server_hello_t *svr_hello = gtp2gems_server_hello(msg, con_len);
    if(!svr_hello){
        printf("convert gtp to server hello failed\n");
        return -1;
    }

    // 2. handle server hello
    int ret = sge_safe_client_handle_hello_rsp(ptr, enc_link, svr_hello);
    free(svr_hello);
    if(0 != ret){
        printf("handle server hello failed! error code:%d\n", ret);
        return -2;
    }

    // 3. create authentication
    fld_authentication_info_t *auth_info = (fld_authentication_info_t *)calloc(1, sizeof(fld_authentication_info_t));
    ret = sge_safe_client_create_authentication(ptr, enc_link, auth_info);
    if(0 != ret){
        printf("create authentication info failed! error code:%d\n", ret);
        return -3;
    }

    // 4. convert auth info to gtp

    // 5. send gtp

    free(auth_info);

    return 0;
}

fld_sessionkey_info_t *gtp2gems_sessionkey(const char *msg, unsigned int con_len){
    // convert gtp to fld_sessionkey_info_t

    // mock
    fld_sessionkey_info_t *sessionkey = (fld_sessionkey_info_t*)malloc(sizeof(fld_sessionkey_info_t));

    return sessionkey;
}
int handle_svr_session_key(api_safe_client_session_t *ptr, api_safe_client_link_t *enc_link, const char *msg, unsigned int con_len){
    // 1. gtp to server sessionkey
    fld_sessionkey_info_t *sessionkey = gtp2gems_sessionkey(msg, con_len);
    if(!sessionkey){
        printf("convert gtp2gems sessionkey failed!\n");
        return -1;
    }

    // 2. accept sessionkey
    int ret = sge_safe_client_accept_sessionkey(ptr, enc_link, sessionkey);
    free(sessionkey);
    if(0 != ret){
        printf("handle session key failed! error code:%d\n", ret);
        return -2;
    }

    return 0;
}

#define MSG_SVR_HELLO 1
#define MSG_SVR_SESSION_KEY 2
static void __recv_msg(const char * msg, int type, unsigned int con_len, void * ctx) {
    api_ctx_t *api_ctx = (api_ctx_t*)ctx;
    if(!api_ctx){
        printf("invalid api ctx!\n");
        return;
    }

    int ret;
    if(GTP_TYPE_STRING == type){
        int msg_type = get_svr_msg_type(msg, con_len);
        switch(msg_type){
            case MSG_SVR_HELLO:
                ret = handle_svr_hello(api_ctx->safe_sess_, api_ctx->safe_link_, msg, con_len);
                if(0 != ret){
                    // disconnect
                }
                break;
            case MSG_SVR_SESSION_KEY:
                ret = handle_svr_session_key(api_ctx->safe_sess_, api_ctx->safe_link_, msg, con_len);
                if(0 != ret){
                    // disconnect
                }
                else {
                    api_ctx->shakehand_state_ = HANDSHAKED;                  
                }
                break;
            default:
                printf("error svr message!\n");
                // disconnect
        }
    }
    else if(GTP_TYPE_ENCRYPT == type){
        unsigned char *buf = (unsigned char*)malloc(con_len);
        unsigned int cap;
        ret = sge_safe_client_packet_decrypt(api_ctx->safe_sess_, api_ctx->safe_link_, (unsigned char*)msg, con_len, buf, &cap);
        if(0 != ret){
            printf("decrypt msg failed, error code:%d\n", ret);
            // disconnect
        }
        printf("recv msg:%s\n", buf);
        // process msg
        free(buf);
    }
    else{
        printf("invalid msg type!\n");
        // disconnect
    }
}

char *svr_enc_pubkey = "server enc pubkey";
char *svr_sign_pubkey = "server sign pubkey";

session_client_info_t *create_session_client_info(){
    session_client_info_t *session_info = (session_client_info_t*)malloc(sizeof(session_client_info_t));
    if(!session_info)
        return NULL;

    unsigned int len = sizeof(session_info->sge_id_);
    strncpy(session_info->sge_id_, g_sge_id, len);
    session_info->sge_id_[len - 1] = '\0';
    len = sizeof(session_info->trader_id_);
    strncpy(session_info->trader_id_, g_trader_id, len);
    session_info->trader_id_[len - 1] = '\0';
    session_info->enc_mode_ = ENC_MODE_HARD;
    memcpy(session_info->svr_enc_pubkey_, svr_enc_pubkey, strlen(svr_enc_pubkey));
    session_info->svr_enc_pubkey_len_ = strlen(svr_enc_pubkey);
    memcpy(session_info->svr_sign_pubkey_, svr_sign_pubkey, strlen(svr_sign_pubkey));
    session_info->svr_sign_pubkey_len_ = strlen(svr_sign_pubkey);

    return session_info;
}

static void __cb_connected(api_connection_state_t * cs, void * ctx) {
    printf("connected to acsvr. ip: %s, port: %d, state: %d\n", cs->ip, cs->port, cs->state);

    api_ctx_t *api_ctx = (api_ctx_t*)ctx;
    if(!api_ctx){
        printf("invalid api ctx!\n");
        // disconnect
        return;
    }

    // 1. create session info
    session_client_info_t *session_info = create_session_client_info();
    if(NULL == session_info){
        printf("alloc session client info failed!\n");
        // disconnect
        return;
    }

    // 2. create session handle
    api_ctx->safe_sess_ = sge_safe_create_client_session(api_ctx->safe_api_, session_info);
    free(session_info);
    if(!api_ctx->safe_sess_){
        printf("create session handle failed\n");
        goto err;
    }

    // 3. create client hello
    fld_client_hello_t *cli_hello = (fld_client_hello_t *)calloc(1, sizeof(fld_client_hello_t));
    int ret = sge_safe_client_create_hello(api_ctx->safe_sess_, cli_hello);
    if(ret != 0){
        printf("create client hello failed!\n");
        goto err;
    }

    // 4.convert client hello to gtp

    // 5.send gtp

    free(cli_hello);
    return;

err:
    if(session_info)
        free(session_info);
    if(api_ctx->safe_sess_){
        sge_safe_free_client_session(api_ctx->safe_sess_);
        api_ctx->safe_sess_ = NULL;
    }
}
static void __cb_disconnect(api_connection_state_t * cs, void * ctx) {
    printf("disconnect with acsvr. ip: %s, port: %d, state: %d\n", cs->ip, cs->port, cs->state);

    api_ctx_t *api_ctx = (api_ctx_t*)ctx;
    if(!api_ctx){
        printf("invalid api ctx!\n");
        return;
    }

    // free session
    if(api_ctx->safe_sess_){
        sge_safe_free_client_session(api_ctx->safe_sess_);
        api_ctx->safe_sess_ = NULL;
    }
}
void send_msg(api_ctx_t *api_ctx){
    char *login_msg = "X03=A100";
    unsigned int msg_len = strlen(login_msg);
    unsigned char *dst = (unsigned char*)calloc(1, msg_len + 16);
    unsigned int dst_cap = msg_len + 16;
    int ret = sge_safe_client_packet_encrypt(api_ctx->safe_sess_, api_ctx->safe_link_, (unsigned char*)login_msg, msg_len, dst, &dst_cap);
    if(0 != ret){
        printf("encrypt msg failed!\n");
    }

    // send msg

    free(dst);
}

int main(int argc, char *argv[]){
    // 1. create safe api ctx
    encryptor_ctx_t encryptor_ctx = {ENCRYPTOR_IP, ENCRYPTOR_PORT, TIME_OUT};

    // 2. create api ctx
    client_dev_cb_list_t safe_api_cb = {
        cb_connect,
        cb_disconnect,
        cb_reconnect,
        cb_reset_connect,
        cb_is_connected,
        cb_encrypt_data,
        cb_decrypt_data,
        cb_mac_data,
        cb_hash_data,
        cb_sign_data,
        cb_verify_signed_data,
        cb_gen_random,
        cb_get_skey,
        cb_get_enc_keypair,
        cb_free_enc_keypair,
        cb_get_sign_keypair,
        cb_free_sign_keypair,
        &encryptor_ctx,
    };

    api_ctx_t api_ctx;

    // 3. create safe api
    api_ctx.safe_api_ = sge_safe_client_api_create(&safe_api_cb, ENC_MODE_HARD);
    if(!api_ctx.safe_api_){
        printf("create safe api failed!\n");
        goto end;
    }

    // 4. create safe link
    api_ctx.safe_link_ = sge_safe_client_link_create(api_ctx.safe_api_);
        
    // 5. create api
    api_cb_t api_cb = {__recv_msg, __cb_connected, __cb_disconnect};
    api_t *api = api_init(SERVER_IP, SERVER_PORT, &api_cb, &api_ctx);
    if(!api){
        printf("init api failed!\n");
        goto end;
    }

    // 6. wait server session key
    while(__is_safe_handshake_finished(api_ctx) != 1) {
        ;
    }

    // 7. login, order apply, ..., logout
    send_msg(&api_ctx);

    api_wait(api);

end:
    if(api)
        api_free(api);
    if(api_ctx.safe_sess_)
        sge_safe_free_client_session(api_ctx.safe_sess_);
    if(api_ctx.safe_link_)
        sge_safe_client_link_destroy(api_ctx.safe_link_);
    if(api_ctx.safe_api_)
        sge_safe_client_api_destroy(api_ctx.safe_api_);

    return 0;
}

