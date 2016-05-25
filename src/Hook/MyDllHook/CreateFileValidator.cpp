#include "stdafx.h"
#include "CreateFileValidator.h"
#include <fstream>

CreateFileValidator::CreateFileValidator()
{
	this->fValidator = new fileValidator();
	this->sValidator = new stringValidator();
}

extern bool release;



CreateFileValidator::~CreateFileValidator()
{
}

//void CreateFileValidator::setHookObj(Hook* pHook)
//{
////	this->pHook = pHook;
//}

void CreateFileValidator::setRules(string rules)
{
	Json::Reader reader;
	bool parsedSuccess = reader.parse(rules, this->rules);
	if (!parsedSuccess)
	{
		return;
	}
}

int CreateFileValidator::Validate(string filePath)
{
	TCHAR exepath[MAX_PATH + 1];
	GetModuleFileName(0, exepath, MAX_PATH + 1);
	wstring wideExePath(exepath);
	string exePath(wideExePath.begin(), wideExePath.end());
	int pos = exePath.find_last_of("\\");
	if (pos != string::npos)
		exePath = exePath.substr(pos + 1);

	if (this->rules.isNull() || !this->rules.isArray() )
		return true;
	int ruleType, actionToTake, id;
	string processName, ruleContent;
	Json::Value rule;
	Json::Reader reader;
	bool result = true;
	for (auto itr : this->rules)
	{
		processName = itr["processName"].asString();

		if (!(processName == exePath))
			continue;

		id = itr["id"].asInt();
		ruleType = itr["ruleType"].asInt(); // 1 - extension, 2 - md5, 3 - regex, 4 - content similarity, 5 - begins with, 6 - contains
		actionToTake = itr["actionToTake"].asInt();
		
		ruleContent = itr["ruleContent"].asString();
		reader.parse(ruleContent, rule);
		switch (ruleType)
		{
		case 1:
			result = result && this->validateExtension(filePath, rule);
			break;
		case 2:
			result = result && this->validateHash(filePath, rule);
			break;
		case 3:
			result = result && this->validateRegex(filePath, rule);
			break;
		case 4:
			result = result && this->validateSimilarity(filePath, rule);
			break;
		case 5:
			result = result && this->validatePrefix(filePath, rule);
			break;
		case 6:
			result = result && this->validateContentInclude(filePath, rule);
			break;
		}
		if (!result)
		{
			this->lastIncidentId = id; // set last incident rule hit id
			return (actionToTake == 1) ? 1 : 2 ;// 1 = block and notify, 2 = only notify; this line is to prevent errors;
		}
	}
	return 0; // File valid; 0 = Do nothing
}

bool CreateFileValidator::validateExtension(string filePath, Json::Value rule)
{
	return !this->sValidator->endsWith(filePath, rule["content"].asString());
}

bool CreateFileValidator::validateHash(string filePath, Json::Value rule)
{
	fValidator->setFilePath(filePath);
	bool res = this->fValidator->compareFileHash(rule["content"].asString());
	return res;
}

bool CreateFileValidator::validateSimilarity(string filePath, Json::Value rule)
{
	string fileContent;
	string temp;
	ifstream file;
	::release = true;
	file.open(filePath);
	std::string s((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	::release = false;
	float result = this->sValidator->stringSimilarity(fileContent, rule["content"].asString());
	return result * 100 < rule["similarity"].asFloat();
}

bool CreateFileValidator::validateContentInclude(string filePath, Json::Value rule)
{
	return true;
}

bool CreateFileValidator::validatePrefix(string filePath, Json::Value rule)
{
	return true;
}

bool CreateFileValidator::validateRegex(string filePath, Json::Value rule)
{
	return true;
}