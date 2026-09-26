#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sge_api.h"

#ifdef WIN32
    #include <windows.h>
    #define STRNCPY      strncpy
    #define SNPRINTF     _snprintf_s
    #define SLEEP(sec)   Sleep((sec)*1000)
#else
    #include <unistd.h>
    #define STRNCPY     strncpy
    #define SNPRINTF    snprintf
    #define SLEEP(sec)  sleep(sec)
#endif

#define MAX_GTP_MSG_LENGTH  (4096*2)

typedef struct ctx_A {
    sge_session_t * session_;
} ctx_A_t;

static void __read_A(int type, const char * gtp_msg, unsigned int len, void * ctx) {
    ctx_A_t * pctx = (ctx_A_t *)ctx;
    
    printf("ConnectionA IP:%s, PORT:%d receive a msg, msg type: %d, msg len: %d\n", sge_session_get_ip(pctx->session_), sge_session_get_port(pctx->session_), type, len);

    if (type == MSG_TYPE_PLAIN) {
        printf("msg: %s\n", gtp_msg);
    }
    else if (type == MSG_TYPE_CIPHER) {
        printf("msg is encrypted, need to decrypt\n");
        //decrypt msg
    }
    else {
        printf("type[%d] is unrecognized\n", type);
    }
}

static void __connected_A(void * ctx) {
    ctx_A_t * pctx = (ctx_A_t *)ctx;
    printf("ConnectionA IP:%s, PORT:%d, connect sucessfully\n", sge_session_get_ip(pctx->session_), sge_session_get_port(pctx->session_));
}

static void __disconnected_A(int error_no, void * ctx) {
    ctx_A_t * pctx = (ctx_A_t *)ctx;
    printf("ConnectionA IP:%s, PORT:%d, disconnected\n", sge_session_get_ip(pctx->session_), sge_session_get_port(pctx->session_));
}

typedef struct ctx_B {
    sge_session_t * session_;
} ctx_B_t;

static void __read_B(int type, const char * gtp_msg, unsigned int len, void * ctx) {
    ctx_B_t * pctx = (ctx_B_t *)ctx;
    
    printf("ConnectionB IP:%s, PORT:%d receive a msg, msg type: %d, msg len: %d\n", sge_session_get_ip(pctx->session_), sge_session_get_port(pctx->session_), type, len);
    
    if (type == MSG_TYPE_PLAIN) {
        printf("msg: %s\n", gtp_msg);
    }
    else if (type == MSG_TYPE_CIPHER) {
        printf("msg is encrypted, need to decrypt\n");
        //decrypt msg
    }
    else {
        printf("type[%d] is unrecognized\n", type);
    }
}

static void __connected_B(void * ctx) {
    ctx_B_t * pctx = (ctx_B_t *)ctx;
    
    printf("ConnectionB IP:%s, PORT:%d, connect sucessfully\n", sge_session_get_ip(pctx->session_), sge_session_get_port(pctx->session_));
}

static void __disconnected_B(int error_no, void * ctx) {
    ctx_B_t * pctx = (ctx_B_t *)ctx;
    
    printf("ConnectionB IP:%s, PORT:%d, disconnected\n", sge_session_get_ip(pctx->session_), sge_session_get_port(pctx->session_));
}

static void * send_msg_a(void *ctx) {
    if (NULL == ctx) {
        printf("ctx is NULL\n");
    }
    ctx_A_t * pctx = (ctx_A_t *)ctx;
    int ret = -1;
    //log begin 
    sge_mthread_log_t * send_a_log = sge_mthread_log_create("./log", "send_a",LEVEL_DEBUG);

    while (!sge_session_is_connected(pctx->session_)) {
        SLEEP(1);
        printf("wait the session connected!\n");
    }

    //if current network delay is large, change the heartbeat timeout to 100s
    ret = sge_session_set_heartbeat_timeout(pctx->session_, 100);
    if (ret == 0) {
        printf("current heartbeat timeout is %d(s)\n", sge_session_get_heartbeat_timeout(pctx->session_));
    }
    const char * msg = "X01=GTP1.0,X02=5869,X04=1,X05=1,X06=L,X07=0,X08=0000,X09=0000,X03=A100,M60=bjs01261,M20=026"; //the msg is faked, 
    int msg_len = strlen(msg);
    ret = sge_session_write(pctx->session_, MSG_TYPE_PLAIN, msg, msg_len);
    if (ret != 0) {
        printf("send msg failed\n");
    }
    
    //log end 
    sge_mthread_log_destroy(send_a_log);

    return NULL;
}

static void * send_msg_b(void *ctx) {
    ctx_B_t * pctx = (ctx_B_t *)ctx;
    int ret = -1;
    //log begin 
    sge_mthread_log_t * send_b_log = sge_mthread_log_create("./log", "send_b",LEVEL_DEBUG);

    while (!sge_session_is_connected(pctx->session_)) {
        SLEEP(1);
        printf("wait the session connected!\n");
    }

    //if current network delay is large, change the heartbeat timeout to 100s
    ret = sge_session_set_heartbeat_timeout(pctx->session_, 100);
    if (ret == 0) {
        printf("current heartbeat timeout is %d(s)\n", sge_session_get_heartbeat_timeout(pctx->session_));
    }

    const char * msg = "X01=GTP1.0,X02=5869,X04=1,X05=1,X06=L,X07=0,X08=0000,X09=0000,X03=A100,M60=bjj01261,M20=026"; //the msg is faked, 
    int msg_len = strlen(msg);
    ret = sge_session_write(pctx->session_, MSG_TYPE_PLAIN, msg, msg_len);
    if (ret != 0) {
        printf("send msg failed\n");
    }
    
    //log end 
    sge_mthread_log_destroy(send_b_log);

    return NULL;
}

typedef struct ctx{
	sge_mthread_log_t * service_log;
}ctx_t;

static void __before_service_run(void * ptr){
	if (ptr) { 
		ctx_t * pctx = (ctx_t *) ptr;
		pctx->service_log = sge_mthread_log_create("./log", "service", LEVEL_DEBUG);
	}
	return ;
}

static void __after_service_stop(void * ptr){
	if (ptr) { 
		ctx_t * pctx = (ctx_t *) ptr;
		if (pctx->service_log) {
			sge_mthread_log_destroy(pctx->service_log);
		}
	}
	return ;
}

int main(int argc, char ** argv) {
    int ret = -1;
    //0 log begin 
    sge_mthread_log_t * main_log = sge_mthread_log_create("./log", "main",LEVEL_DEBUG);
    //TODO:check result

    //1 create service
	sge_service_cb_t svr_callbacks = {  
		__before_service_run,  
		__after_service_stop,  
	}; 

	ctx_t ctx = {0};
    sge_service_t * service = sge_service_create(&svr_callbacks, &ctx);
    //TODO:check result

    //2 service run
    ret = sge_service_run(service);
    if (ret !=0) {
        printf("service run failed\n");
    }

	//3 connectA connect and send msg
    // 3.1 build connectA 
    //    3.1.1 prepare callbacks for A
    sge_session_callbacks_t cbs_a = {
		__read_A,
		__connected_A,
		__disconnected_A
    };
    //    3.1.2 prepare ctx for A
    ctx_A_t ctx_a;
    //    3.1.3 create connection for A
    ctx_a.session_ = sge_session_create(service, "180.2.30.177", 16777, &cbs_a, 20, 60, &ctx_a);
	sge_session_connect(ctx_a.session_);

    //    3.1.4 create pthread for sending msg by ConnectionA
    pthread_t tid_a;
    if (pthread_create(&tid_a, NULL, send_msg_a, &ctx_a) != 0) {
        printf("pthread create fail.\n");
    }

	//4 connectB connect and send msg
    // 4.1 build connectB 
    //    4.1.1 prepare callbacks for B 
    sge_session_callbacks_t cbs_b = {
        __read_B,
        __connected_B,
        __disconnected_B
    };
    //    4.1.2 prepare ctx for B 
    ctx_B_t ctx_b;
    //    4.1.3 create connection for B 
    ctx_b.session_ = sge_session_create(service, "180.3.10.177", 16777, &cbs_b, 20, 60, &ctx_b);
	sge_session_connect(ctx_b.session_);

    //    4.1.4 create pthread for sending msg by ConnectionB 
    pthread_t tid_b;
    if (pthread_create(&tid_b, NULL, send_msg_b, &ctx_b) != 0) {
        printf("pthread create fail.\n");
    }
    
	//5 wait A&B send msg over 
    pthread_join(tid_a, NULL);
    pthread_join(tid_b, NULL);

    SLEEP(15);

    //6 disconnections
    // 6.1 disconnectA
    ret = sge_session_disconnect(ctx_a.session_);
	
	sge_session_destroy(ctx_a.session_);
    // 6.2 disconnectB
    ret = sge_session_disconnect(ctx_b.session_);
	sge_session_destroy(ctx_b.session_);

    //7 service stop 
    ret = sge_service_stop(service);

    //8 service destroy
    ret = sge_service_destroy(service);

    //9 log end
    ret = sge_mthread_log_destroy(main_log);
}
