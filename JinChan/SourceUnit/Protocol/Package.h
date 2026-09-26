#pragma once

//CPackageBuffer:CPackage中数据真正存放的地方，记录了CPackage对象对数据缓冲取的引用个数
class  CPackageBuffer
{
public:
	CPackageBuffer(int length)
		:m_nLength(length),m_nRefCount(1),m_bReleaseData(true)
	{
		m_pData = new char[length];
		memset(m_pData, 0, length);
	}	
	CPackageBuffer(char* pData, int length)
		:m_pData(pData),m_nLength(length),m_nRefCount(1),m_bReleaseData(false)
	{
	}
	~CPackageBuffer()
	{
		if (m_bReleaseData)
		{
			delete[] m_pData;
		}
	}
	
	char* GetData() { return m_pData; }
	int GetLength() const { return m_nLength; }
	
	void AddRef() { m_nRefCount++; }
	void Release()
	{
		if( --m_nRefCount <= 0 )
			delete this;
	}

protected:
	char* m_pData;		//指向缓冲区数据的指针
	int m_nLength;		//缓冲区长度
	int m_nRefCount;	//缓冲区的引用个数
	bool m_bReleaseData;//是否管理缓冲区的生命周期
};

//定义了一个通讯数据包的基类
class  CPackage  
{
public:
	CPackage();
	void Clear();
	
	//数据包上传时，上层协议的编号，子类必须实现此函数
	virtual DWORD GetActiveID() { return 0; }

	/** 检查包内数据是否构成一个完整的包。若有多余的数据，则截掉。
	* @return >=0 是一个完整的包 
	* @return -1 数据包不完整
	* @return <-1 数据包有错误
	* @remark 派生类一般要重载该函数，实现自已对数据的校验
	*/
	virtual int ValidPackage() { return GetLength(); }

	//组装数据包
	virtual int MakePackage() { return 0; }

	char* Push(int size);
	char* Pop(int size);	
	char* GetAddress() { return m_pHead; }
	int GetLength() { return int(m_pTail-m_pHead); }
	
	//将另一个包的缓冲区关联到本包中
	void BufAddRef(CPackage* pPackage);
	
	//脱离本包对Buff的引用
	CPackageBuffer* BufRelease();
	
	//实际为package分配空间
	void ConstructAllocate(int nCapacity, int nReserve);
	
	//从关联的PackagBuffer上分配一块空间
	//@return 分配空间的首地址。若未关联PackgeBuffer或PackageBuffer空间不足，则返回NULL
	char* Allocate(int length);

	//从关联的PackagBuffer上分配最大空间
	//@return 分配空间的首地址。若未关联PackgeBuffer或PackageBuffer空间不足，则返回NULL
	char* AllocateMax();

	//缓冲区尾部截去一段
	void Truncate(int newLength);
	
	//减少对本包的引用，当引用为0时，析构本对象
	void Release();
	
	//增加对本包的引用
	void AddRef();
	
	//将源包的数据拷贝到本包中
	void AttachBuffer(CPackageBuffer* pPackageBuffer);
	
	//设置包中的数据
	//remark 该函数不会产生PackageBuffer，只是设置了头、尾指针，因此，本包只能用于读取，不能用于写入数据
	void SetData(char* pData, int nDataLen);
protected:
	virtual ~CPackage();

protected:
	CPackageBuffer* m_pPackageBuffer;  //指向数据缓冲区的指针
	char* m_pHead;			//指向数据缓冲区的头指针
	char* m_pTail;			//指向数据缓冲区的尾指针
	int m_nRefCount;		//对本包的引用个数
	int m_nReserve;			//保留空间大小，供报文头使用
};
