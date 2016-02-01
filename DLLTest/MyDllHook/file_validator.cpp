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

bool stringContains(string str, string substr)
{
	return str.find(substr) != string::npos;
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


unsigned int fileValidator::levenshteinDistance(const std::string& s1, const std::string& s2)
{
	const std::size_t len1 = s1.size(), len2 = s2.size();
	std::vector<unsigned int> col(len2+1), prevCol(len2+1);
	
	for (unsigned int i = 0; i < prevCol.size(); i++)
		prevCol[i] = i;
	for (unsigned int i = 0; i < len1; i++) {
		col[0] = i+1;
		for (unsigned int j = 0; j < len2; j++)
                        
			col[j+1] = min(prevCol[1 + j] + 1, col[j] + 1, prevCol[j] + (s1[i]==s2[j] ? 0 : 1));
		col.swap(prevCol);
	}
	return prevCol[len2];
}

float fileValidator::stringSimilarity(string s1, string s2)
{

	return ((s1.length() - this->levenshteinDistance(s1, s2)) / (float)s1.length()) * 100;

}