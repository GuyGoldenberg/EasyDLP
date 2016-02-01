#include <string>
#include <vector>
#include <regex>
#include "stdafx.h"

using namespace std;

class fileValidator
{
public:	
	fileValidator(){};

	/// <summary>
	/// Checks weather a string ends with a specified suffix.
	/// </summary>
	/// <param name="str">The base string.</param>
	/// <param name="end">The suffix to check for in the base string.</param>
	/// <returns></returns>
	bool endsWith(string str, string end);
	
	/// <summary>
	/// Checks weather a string ends with a specified vector(list) of suffixes.
	/// </summary>
	/// <param name="str">The string.</param>
	/// <param name="ending">List of suffixes to check for in the base string</param>
	/// <returns></returns>
	bool endsWith(string str, vector<string> ending);
	
	/// <summary>
	/// Finds all matches for a regex in a string.
	/// </summary>
	/// <param name="str">The string to find matches in.</param>
	/// <param name="pattern">The regex pattern(as string).</param>
	/// <returns></returns>
	vector<string> checkForExpression(string str, string pattern);

};