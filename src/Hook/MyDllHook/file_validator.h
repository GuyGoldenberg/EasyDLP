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
	fileValidator(){};
	~fileValidator();
	
	bool isInPath(string basePath);
	bool isInPath(vector<string> basePath);

	bool compareFileHash(string hash){ throw exception(); return true; };
	bool compareFileHash(vector<string> hashes){ throw exception(); return true; };

	void setFilePath(string filePath);

};

