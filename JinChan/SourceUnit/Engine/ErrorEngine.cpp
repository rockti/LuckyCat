#include "stdafx.h"
#include "ErrorEngine.h"

/*
CErrorEngine::CErrorEngine(void)
:m_LastErrorID(0),m_LastErrorMsg("正确") {
}
CErrorEngine::~CErrorEngine(void) {
}

void CErrorEngine::registerErrorType(int errorID, char *errorMsg) {
	if (!insert(value_type(errorID,errorMsg)).second) {
		return;
	}
}
void CErrorEngine::registerErrorType(TErrorType pErrorType[]) {
	for (int i=0;;i++) {
		if (pErrorType[i].errorID==0) {
			break;
		}
		registerErrorType(pErrorType[i].errorID,pErrorType[i].errorMsg);
	}
}

void CErrorEngine::reportError(const int errorID) {
	iterator iter = find(errorID);
	if (iter == end()) {
		return;
	}
	m_LastErrorID = errorID;
	m_LastErrorMsg = iter->second;
}
char* CErrorEngine::getErrorMsg(const int errorID) {
	iterator iter = find(errorID);
	if (iter == end()) {
		return NULL;
	}
	return iter->second;
}

void CErrorEngine::getLastError(int* pLastErrorID, char** pLastErrorMsg) {
	if (NULL != pLastErrorID) {
		*pLastErrorID = m_LastErrorID;
	}
	if (NULL != pLastErrorMsg) {
		*pLastErrorMsg = m_LastErrorMsg;
	}
	m_LastErrorID = 0;
	m_LastErrorMsg = "正确";
}

void CErrorEngine::getLastErrorWithoutClear(int* pLastErrorID, char** pLastErrorMsg) {
	if (NULL != pLastErrorID) {
		*pLastErrorID = m_LastErrorID;
	}
	if (NULL != pLastErrorMsg) {
		*pLastErrorMsg = m_LastErrorMsg;
	}
}
*/