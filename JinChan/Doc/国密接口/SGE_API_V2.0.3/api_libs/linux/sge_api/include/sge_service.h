#ifndef __API_SGE_SERVICE_H__
#define __API_SGE_SERVICE_H__

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


struct sge_service_s;
typedef struct sge_service_s sge_service_t;

typedef void (*sge_service_on_before_run_t)(void * ctx);
typedef void (*sge_service_on_after_stop_t)(void * ctx);

typedef struct sge_service_cb_s {
    sge_service_on_before_run_t    on_before_run_;
    sge_service_on_after_stop_t    on_after_stop_;
} sge_service_cb_t;

DLL_DECLARE sge_service_t * sge_service_create(sge_service_cb_t * svr_callbacks, void * ctx);
DLL_DECLARE int sge_service_destroy(sge_service_t * ptr);

DLL_DECLARE int sge_service_run(sge_service_t * ptr);
DLL_DECLARE int sge_service_stop(sge_service_t * ptr);

DLL_DECLARE int sge_service_is_running(sge_service_t * ptr);

#ifdef  __cplusplus
}
#endif  /*  end of __cplusplus  */

#endif  /* __API_SGE_SERVICE_H__ */ 
