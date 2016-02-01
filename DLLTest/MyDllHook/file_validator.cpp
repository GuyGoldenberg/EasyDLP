#include "file_validator.h"


using namespace std;

bool fileValidator::endsWith(string str, string end)
{
	int strLen = str.length(), endLen = end.length();
	return (strLen >= endLen && str.compare(strLen - endLen, strLen, end) == 0);
}

bool fileValidator::endsWith(string str, vector<string> ending)
{
	int endLen, strLen = str.length();
	for (vector<string>::iterator i = ending.begin(); i != ending.end(); ++i)
	{
		endLen = i->length();
		if (strLen >= endLen)
		{
			if (str.compare(strLen - endLen, strLen, *i) == 0)
				return true;
		}
	}
	return false;

}

vector<string> fileValidator::checkForExpression(string str, string pattern)
{
	vector<string> results;
	const regex re(pattern.c_str());
	sregex_token_iterator iter(str.begin(), str.end(), re), end;
	for (; iter != end; ++iter)
	{
		results.push_back(*iter);
	}
	return results;
}