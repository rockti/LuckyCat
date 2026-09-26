#ifndef __API_SGE_SESSION_H__
#define __API_SGE_SESSION_H__

#include "sge_service.h"

#define MSG_TYPE_PLAIN  1
#define MSG_TYPE_CIPHER 4

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

struct sge_session_s;
typedef struct sge_session_s sge_session_t;

typedef void (*sge_session_on_read_t)(int type, const char * gtp_msg, unsigned int len, void * ctx);
typedef void (*sge_session_on_connected_t)(void * ctx);
typedef void (*sge_session_on_disconnected_t)(int error_no, void * ctx);

typedef struct sge_session_callbacks_s {
    sge_session_on_read_t           on_read_;
    sge_session_on_connected_t      on_connected_;
    sge_session_on_disconnected_t   on_disconnected_;
} sge_session_callbacks_t;

DLL_DECLARE int sge_session_destroy(sge_session_t * ptr);

DLL_DECLARE sge_session_t * sge_session_create(sge_service_t * service, const char * ip, int port, sge_session_callbacks_t * cbs, unsigned int heartbeat_send_interval, unsigned int heartbeat_timeout, void * ctx);

DLL_DECLARE sge_session_t * sge_session_create_with_connect_timeout(sge_service_t * service, const char * ip, int port, unsigned long long connect_time_ms, sge_session_callbacks_t * cbs, unsigned int heartbeat_send_interval, unsigned int heartbeat_timeout, void * ctx);

DLL_DECLARE int sge_session_connect(sge_session_t * ptr);

DLL_DECLARE int sge_session_disconnect(sge_session_t * ptr);

DLL_DECLARE int sge_session_is_connected(sge_session_t * ptr);

DLL_DECLARE int sge_session_write(sge_session_t * ptr, int type, const char * gtp_msg, unsigned int len);

DLL_DECLARE const char * sge_session_get_ip(sge_session_t * ptr);

DLL_DECLARE int sge_session_get_port(sge_session_t * ptr);

DLL_DECLARE int sge_session_get_heartbeat_timeout(sge_session_t * ptr);

DLL_DECLARE int sge_session_set_heartbeat_timeout(sge_session_t * ptr, unsigned short second);

#ifdef  __cplusplus
}
#endif  /*  end of __cplusplus */ 

#endif /* __API_SGE_SESSION_H__ */ 
