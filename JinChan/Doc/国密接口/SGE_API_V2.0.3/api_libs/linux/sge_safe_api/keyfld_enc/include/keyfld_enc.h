#ifndef __KEYFLD_ENC__
#define __KEYFLD_ENC__

#include <stddef.h>

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

/**
 * sge_safe_keyfld_enc_verification，验证类API
 *
 * @param in         输入参数，sm3加密后的杂凑值
 * @param in_len     输入参数，sm3加密后的杂凑值长度
 * @param random     输入参数，随机码
 * @param random_len 输入参数，随机码长度
 * @param out        输入参数，对in和random处理后的字符串
 * @param out_cap    输入、输出参数，输入参数时，out的最大容量；输出参数时，out中存储的对in和random处理后的字符串长度
 *
 * @return  0: SUCCESS
 *         -1: NULL pointer
 *         -2：in_len is not required 32 bytes
 *         -3：random_len is not required 32 bytes
 *         -4: The length of the out_cap is not enough
 *         -5：encoding failed
 */
DLL_DECLARE int sge_safe_keyfld_enc_verification(const unsigned char *in, const size_t in_len, const unsigned char *random, const size_t random_len, char *out, size_t *out_cap); 

/**
 * sge_safe_keyfld_enc_mgmt，管理类API
 *
 * @param in      输入参数，sm3加密后的杂凑值
 * @param in_len  输入参数，sm3加密后的杂凑值长度
 * @param out     输入参数，对in处理后的字符串
 * @param out_cap 输入、输出参数，输入参数时，out最大容量；输出参数时，out中存储对in处理后的字符串长度
 *
 * @return  0: SUCCESS
 *         -1：NULL pointer
 *         -2：in_len is not required 32 bytes
 *         -4: The length of the out_cap is not enough
 *         -5：encoding failed
 */
DLL_DECLARE int sge_safe_keyfld_enc_mgmt(const unsigned char *in, const size_t in_len, char *out, size_t *out_cap);

#ifdef  __cplusplus
}
#endif 

#endif

