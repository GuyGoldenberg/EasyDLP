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
	/// <returns>If the base string ends with the specified suffix.</returns>
	bool endsWith(string str, string end);
	
	/// <summary>
	/// Checks weather a string ends with a specified vector(list) of suffixes.
	/// </summary>
	/// <param name="str">The string.</param>
	/// <param name="ending">List of suffixes to check for in the base string</param>
	/// <returns>If the base string ends with the specified suffix.</returns>
	bool endsWith(string str, vector<string> ending);
		
	/// <summary>
	/// Checks if a base string contains a sub-string
	/// </summary>
	/// <param name="str">The base string.</param>
	/// <param name="substr">The sub-string.</param>
	/// <returns>If the base string contains the sub-string.</returns>
	bool stringContains(string str, string substr);
	
	/// <summary>
	/// Finds all matches for a regex in a string.
	/// </summary>
	/// <param name="str">The string to find matches in.</param>
	/// <param name="pattern">The regex pattern(as string).</param>
	/// <returns>A list(vector) of regex matches results.</returns>
	vector<string> checkForExpression(string str, string pattern);

	unsigned int levenshteinDistance(const std::string& s1, const std::string& s2);

	
	/// <summary>
	/// Checks for strings similarity.
	/// </summary>
	/// <param name="s1">Source string</param>
	/// <param name="s2">Targer string</param>
	/// <returns>The similatiry rate between s1 and s2</returns>
	float stringSimilarity(string s1, string s2);

};