#pragma once

#include "Package.h"
#include "ReactorThread.h"
#include <vector>
class CProtocol;
class CProtocolCallback
{
public:
	virtual int HandlePackage(CPackage* pPackage, CProtocol* pProtocol) = 0;
	virtual ~CProtocolCallback(){};
};

//CProtocol是一个抽象的网络协议类，每个协议的实现都需要继承它。
//一个CProtocol对象可以注册一个下层协议和多个上层协议。通过Pop()函数将数据包的传递到上层协议，通过Push()函数将数据包传递到下层协议。
class  CProtocol : public CProtocolCallback, public CEventHandler
{
public:
	/**构造函数
	* @param pReactor 指向运行引擎的指针
	* @param nHdrReserveLen 本层协议的包头长度
	* @param pDemuxPackage 本层协议上传使用的包
	* @param pSendPackage 本层协议下传使用的包
	*/
	CProtocol(CReactorThread* pReactor, int nHdrReserveLen, CPackage* pDemuxPackage, CPackage* pSendPackage);
	virtual ~CProtocol();
	
	//注册下层协议	param pLower下层协议	param nActiveID 对下层协议而言，本层协议的编号
	virtual void AttachLower(CProtocol* pLower, DWORD nActiveID);
	virtual void DetachLower(CProtocol* pLower);
	
	/**将数据包从本层协议传递到下层协议
	* @param pPackage 本层协议的包
	* @return 0   成功
	* @return <>0 失败
	*/
	virtual int Send(CPackage* pPackage, CProtocol* pUpper = NULL);
	
	//由下层协议或ZTPControl容器调用，在本层协议实现包的检查、处理和继续上传至上层协议。
	virtual int HandlePackage(CPackage* pPackage, CProtocol* pProtocol);

	//在协议栈中，从底层到本层协议的所有包头保留长度的和
	int GetHdrReserveLen()	{ return m_nHdrReserveLen; }

	//获到本协议对象的协议编号, 协议编号是针对下层协议而言的，只有注册了下层协议，本函数才有意义
	DWORD GetActiveID()	{ return m_nActiveID; }

	//注册错误处理器, 出现错误时，通知该错误处理器进行处理
	void RegisterErrorHandler(CEventHandler* pErrorHandler)	{ m_pErrorHandler = pErrorHandler; }

	/**注册数据包处理器
	*@param pUpperHandler 数据包处理器
	*@remark 如果分发数据包时，找不到合适的上层协议，则将数据包交由注册的数据包处理器来处理。通常发生在协议栈的最上层。
	*/
	void RegisterUpperHandler(CProtocolCallback* pUpperHandler)	{ m_pUpperHandler = pUpperHandler; }		
protected:	
	//注册上层协议
	void AddUpper(CProtocol *pUpper);

	//根据协议编号，查找上层协议
	CProtocol* GetUpper(DWORD nActiveID);

	//根据协议编号，将上层协议取消注册
	CProtocol* RemoveUpper(DWORD nActiveID);
	
	//将数据包分发到到合适的上层协议	0:成功
	virtual int Pop(CPackage* pPackage);
	
	//上层协议将包传送到本层协议
	virtual int Push(CPackage* pPackage, CProtocol* pUpper);

	//通知错误处理器
	void NotifyError(int message, DWORD dwParam, void* pParam);

	//为向下层协议传送而组装数据包
	virtual int MakePackage(CPackage* pPackage, CProtocol* pUpper)	{ return pPackage->MakePackage(); }

	//收到一个错误的数据包时调用
	virtual int OnRecvErrorPackage(CPackage* pPackage)	{ return 0; }

protected:
	DWORD m_nActiveID;			//本协议对在下层协议中的协议编号
	int m_nHdrReserveLen;		//在协议栈中，从底层到本层协议的所有包头长度的和

	CPackage* m_pDemuxPackage;	//多路分解时使用的临时包，防止破坏下层传上来的包
	CPackage* m_pSendPackage;	//下传使用的临时包

	typedef std::vector<CProtocol*> CProtocolArray;
	CProtocolArray m_lowers;	//下层协议
	CProtocol* m_pUpper;		//上层协议
	CProtocol* m_pBrother;		//同层协议

	CProtocolCallback* m_pUpperHandler;	//数据包处理器
	CEventHandler* m_pErrorHandler;		//错误处理器
};
