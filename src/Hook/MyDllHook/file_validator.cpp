#include "StdAfx.h"
#include "file_validator.h"
#include <fstream>
#include "md5.h"
#include "hook.h"

extern bool release;

fileValidator::fileValidator(string filePath)
{
	this->setFilePath(filePath);
}

void fileValidator::setFilePath(string filePath)
{
	this->filePath = filePath;
}

bool fileValidator::isInPath(string basePath)
{
	char realFilePath[MAX_PATH], realBasePath[MAX_PATH];
	GetFullPathName((LPCTSTR)basePath.c_str(), MAX_PATH, (LPTSTR)realBasePath, NULL);
	GetFullPathName((LPCTSTR)(this->filePath.c_str()), MAX_PATH, (LPTSTR)realFilePath, NULL);
	return this->strValidator.beginsWith((string) realBasePath, (string) realFilePath);
}

bool fileValidator::compareFileHash(string hash)
{
	ifstream file;
	int size = 0;
	char * data = 0;
	
	::release = true; // Disable hook file validation
	// Open file for reading in binary mode
	file.open(this->filePath.c_str(), ios::in | ios::binary | ios::ate);
	if (!file.is_open())
	{
		::release = false;
		return true;
	}
	size = file.tellg(); // Get file size
	file.seekg(0, ios::beg);
	data = new char[size + 1];
	file.read(data, size);
	::release = false;
	data[size] = '\0';
	
	MD5* hasher = new MD5();
	hasher->update(data, size);
	hasher->finalize();
	bool res = stricmp(hasher->hexdigest().c_str(), hash.c_str()) != 0;
	delete data; // A serious memory leak caused without deleting the data
	return res;

}


fileValidator::~fileValidator()
{
}
