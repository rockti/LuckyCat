#include "stdafx.h"
#include "ServiceName.h"

CServiceName::CServiceName(char const* location)
{
	if ((NULL==location) || (*location=='\0'))
	{
		RAISE_RUNTIME_ERROR("Empty location for service name");
	}

	m_pLocation = new char[strlen(location)+1];
	strcpy(m_pLocation, location);
	m_pBuffer = new char[strlen(location)+1];
	strcpy(m_pBuffer, location);
	char* buffer = m_pBuffer;
	m_pChannel = buffer;

	m_pHost = "";
	m_pServiceName = "";
	m_nPort = 0;
	char* startPort = "";
	char* position = strchr(buffer, ':');
	if (NULL == position)	return;
	*position = '\0';
	position++;

	//check for // after semicolon
	if ((*position!='/') || (*(position+1)!='/'))
		RAISE_RUNTIME_ERROR("Invalid location for service name");
	position += 2;
	m_pHost = position;
	
	//loop until the end of hostname
	while ((*position) && (*position!=':') && (*position!='/'))
		position++;
	if (*position == ':')
	{
		*position = '\0';
		position++;
		startPort = position;
		//loop until the end of port
		while ((*position) && (*position!='/'))
			position++;
	}
	if (*position == '/')
	{
		*position = '\0';
		position++;
		m_pServiceName = position;
	}

	m_nPort = atoi(startPort);
	if(0 == m_nPort)
	{
		printf("Invalid port! ServiceName string: %s\n", location);
	}	
}

CServiceName::~CServiceName()
{
	delete[] m_pLocation;
	delete[] m_pBuffer;
}

