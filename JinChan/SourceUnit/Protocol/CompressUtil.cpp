#include "stdafx.h"
#include "CompressUtil.h"

CompressUtil::CompressUtil(void)
{
}
CompressUtil::~CompressUtil(void)
{
}

/********************************************************************
inputBuffer [in,const] -- 输入流
inputNumBytes [in] -- 输入流的长度
outputBuffer [out] -- 输出缓冲
outputNumBytes [in,out] -- 输出缓冲的最大长度, 返回输出流的长度
	
利用在rsp流中0的个数来压缩rsp流, 如果0连续出现n次, 用0xEn来表示连续出现的0,(n = 1,...,F)
用0xE0来转义0xEn(n=0,..,F)
这里我们要求outputBuffer为调用者分配, 空间的大小由调用者来决定.
*********************************************************************/
void CompressUtil::Zerocompress(unsigned char const* inputBuffer, unsigned long inputNumBytes, unsigned char* outputBuffer, unsigned long& outputNumBytes)
{
	assert(NULL != outputBuffer);

	unsigned long iteratorIn = 0;
	unsigned long iteratorOut = 0;
	while(iteratorIn < inputNumBytes)
	{
		assert(iteratorOut < outputNumBytes);

		//如果是E0,E1,E2,...,EF的字符要进行转义, 前面加E0
		if((inputBuffer[iteratorIn] > 0xDF) &&(inputBuffer[iteratorIn]<0xF0))
		{
			outputBuffer[iteratorOut++] = 0xE0;
			outputBuffer[iteratorOut++] = inputBuffer[iteratorIn];
			iteratorIn ++;
			continue;
		}

		//如果是0, 那么我们要找到0的步长,最多15个(F)
		if (0x00 == inputBuffer[iteratorIn])
		{
			unsigned long step = iteratorIn+1;
			while((0x00 == inputBuffer[step]) && ((step-iteratorIn)<15) && step < inputNumBytes)
			{
				step ++;
			}

			unsigned char tempChar = (unsigned char)(step - iteratorIn);

			tempChar |= 0xE0;

			outputBuffer[iteratorOut++] = tempChar;

			iteratorIn = step;
			continue;

		}

		//剩下的字符拷贝即可
		outputBuffer[iteratorOut++] = inputBuffer[iteratorIn++];

	}
	outputNumBytes = iteratorOut;
}
void CompressUtil::Zerodecompress(unsigned char const* inputBuffer, unsigned long inputNumBytes, unsigned char* outputBuffer, unsigned long& outputNumBytes)
{
	assert(NULL != outputBuffer);

	//we assume that outputBuffer is big enough
	unsigned long iteratorIn = 0;
	unsigned long iteratorOut = 0;
	while(iteratorIn < inputNumBytes)
	{
		//这里处理的先后顺序很重要首先看是不是0xE0,如果是E0, 那么这个是转义字符
		//assert(iteratorOut < outputNumBytes);
		//20141205 zhanghr 边界超长检测，防止写越界
		//too big compress packet
        if(iteratorOut+15 >= outputNumBytes)
		{
            return;
        }

		if (inputBuffer[iteratorIn] == 0xE0)
		{
			outputBuffer[iteratorOut++] = inputBuffer[iteratorIn+1];
			iteratorIn += 2;
			continue;
		}

		//如果读到没有0xE0带头的E1,...,EF, 那么读出0的个数
		if ((inputBuffer[iteratorIn] > 0xE0) &&(inputBuffer[iteratorIn]<0xF0))
		{
			int tempChar = (int)(inputBuffer[iteratorIn] & 0x0F);
			for(int i=0;i<tempChar;i++)
			{
				outputBuffer[iteratorOut++] = 0x00;
			}
			iteratorIn ++;
			continue;
		}

		//其他字符拷贝即可
		outputBuffer[iteratorOut++] = inputBuffer[iteratorIn++];
	}

	outputNumBytes = iteratorOut;
}
