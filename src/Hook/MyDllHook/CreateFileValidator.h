#pragma once
#include "JsonLib\json\json.h"
#include "string_validator.h"
#include "file_validator.h"
#include "hook.h"


class CreateFileValidator
{
private:
	Json::Value rules;
	stringValidator* sValidator;
	fileValidator* fValidator;
	int lastIncidentId;
	//Hook* pHook;
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
	int getLastIncidentId(){ return this->lastIncidentId; }
//	void setHookObj(Hook* hook);

};

