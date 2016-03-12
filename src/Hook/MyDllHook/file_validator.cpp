#include "StdAfx.h"
#include "file_validator.h"


fileValidator::fileValidator(string filePath)
{
	this->setFilePath(filePath);
}

void fileValidator::setFilePath(string filePath)
{
	//copy(filePath.begin(), filePath.end(), this->filePath); //  should also work.
	this->filePath = filePath;
}

bool fileValidator::isInPath(string basePath)
{
	char realFilePath[MAX_PATH], realBasePath[MAX_PATH];
	GetFullPathName((LPCTSTR)basePath.c_str(), MAX_PATH, (LPTSTR)realBasePath, NULL);
	GetFullPathName((LPCTSTR)(this->filePath.c_str()), MAX_PATH, (LPTSTR)realFilePath, NULL);
	return this->strValidator.beginsWith((string) realBasePath, (string) realFilePath);
}

fileValidator::~fileValidator()
{
}
