#pragma once
#include "string_validator.h"
#include <vector>


class fileValidator
{
private:
	string filePath;
	stringValidator strValidator;

public:
	fileValidator(string filePath);
	fileValidator();
	~fileValidator();
	
	bool isInPath(string basePath);
	bool isInPath(vector<string> basePath);

	bool compareFileHash(string hash);
	bool compareFileHash(vector<string> hashes);

	void setFilePath(string filePath);

};

