#ifndef __API_SGE_UTIL_H__
#define __API_SGE_UTIL_H__

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

#define LEVEL_DEBUG       1
#define LEVEL_TRACE       2
#define LEVEL_NOTICE      3
#define LEVEL_WARNING     64
#define LEVEL_ERROR       65
#define LEVEL_FATAL       66


struct sge_mthread_log_s;
typedef struct sge_mthread_log_s sge_mthread_log_t;

DLL_DECLARE sge_mthread_log_t * sge_mthread_log_create_with_date_format(const char * dir, const char * date_format, const char * log_name, int level);
DLL_DECLARE sge_mthread_log_t * sge_mthread_log_create(const char * dir, const char * log_name, int level);
DLL_DECLARE int sge_mthread_log_destroy(sge_mthread_log_t * ptr);
DLL_DECLARE int sge_mthread_log_set_level(sge_mthread_log_t * ptr, int level);

DLL_DECLARE const char * sge_mthread_log_get_dir(sge_mthread_log_t * ptr);
DLL_DECLARE const char * sge_mthread_log_get_name(sge_mthread_log_t * ptr);
DLL_DECLARE const char * sge_mthread_log_get_path(sge_mthread_log_t * ptr);

DLL_DECLARE int sge_mthread_log_set_date_roll(sge_mthread_log_t * ptr);
DLL_DECLARE int sge_mthread_log_unset_date_roll(sge_mthread_log_t * ptr);

DLL_DECLARE int sge_mthread_log_set_size_roll(sge_mthread_log_t * ptr, int max_file_size_m);
DLL_DECLARE int sge_mthread_log_unset_size_roll(sge_mthread_log_t * ptr);

DLL_DECLARE const char * sge_get_version();

#ifdef  __cplusplus
}
#endif  /*  end of __cplusplus  */

#endif /* __API_SGE_UTIL_H__ */ 
