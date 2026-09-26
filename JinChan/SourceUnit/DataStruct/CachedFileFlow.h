#pragma once
#include "CachedFlow.h"

//CCachedFileFlow是带缓冲的文件流。它首先是CCahcedFlow，同时挂接了一个CFileFlow
class CCachedFileFlow : public CCachedFlow  
{
public:
	/**构造函数
	* @param pszFlowName 流文件的名称（真正的文件有两个，会自动加上"id"和"con"的扩展名
	* @param pszFlowPath 流文件存贮的目录
	* @param bReuse 是否重用流文件
	* @param nMaxObjects 缓存包的最大数量
	* @param nDataBlockSize CacheList使用的数据区块大小，不要小于一个数据包的长度
	*/
	CCachedFileFlow(char const* pszFlowName, char const* pszFlowPath, bool bReuse, int nMaxObjects, int nDataBlockSize);

	/**构造函数
	* @param nFlowID 数字表示的流文件的名称，nFlowID会按16进制转换成字符串做为文件名
	* @param pszFlowPath 流文件存贮的目录
	* @param bReuse 是否重用流文件
	* @param nMaxObjects 缓存包的最大数量
	* @param nDataBlockSize CacheList使用的数据区块大小，不要小于一个数据包的长度
	*/
	CCachedFileFlow(int nFlowID, char const* pszFlowPath, bool bReuse, int nMaxObjects, int nDataBlockSize);

	/**析构函数
	*/
	virtual ~CCachedFileFlow();

	/**挂接另一个流
	* @param pFlow 所要挂接的流
	* @return 原挂接的流
	* @remark 由于已经挂接好了文件流，调用本函数会导致抛出异常
	*/
	virtual int AttachUnderFlow(IFlow* pFlow);
	
	/* 将文件流的缓冲区数据写入到硬盘中去 */
	virtual void FlushToDisk(void);
};
