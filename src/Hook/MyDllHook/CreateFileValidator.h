#pragma once
#include "JsonLib\json\json.h"

#include "string_validator.h"

#include "file_validator.h"

class CreateFileValidator
{
private:
	Json::Value rules;
	stringValidator* sValidator;
	fileValidator* fValidator;
public:
	CreateFileValidator();
	~CreateFileValidator();
	void setRules(string rules);
	int Validate(string filePath);
	bool validateExtension(string filePath, Json::Value rule);
	bool validateHash(string filePath, Json::Value rule);
	bool validateSimilarity(string filePath, Json::Value rule);
	bool validateContentInclude(string filePath, Json::Value rule);
	bool validatePrefix(string filePath, Json::Value rule);
	bool validateRegex(string filePath, Json::Value rule);



};

