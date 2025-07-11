#include "JSONUtils.h"
#include "spdlog/spdlog.h"
#include <fstream>
#include <regex>

#ifdef _WIN32
extern char **_environ;
#else
extern char **environ;
#endif

bool JSONUtils::isMetadataPresent(json root, string field)
{
	if (root == nullptr)
		return false;
	else
		return root.contains(field);
}

bool JSONUtils::isNull(json root, string field)
{
	if (root == nullptr)
	{
		string errorMessage = std::format(
			"JSONUtils::isNull, root is null"
			", field: {}",
			field
		);
		SPDLOG_ERROR(errorMessage);

		throw runtime_error(errorMessage);
	}

	return root[field].is_null();
}

string JSONUtils::asString(json root, string field, string defaultValue, bool notFoundAsException)
{
	if (notFoundAsException && !isMetadataPresent(root, field))
	{
		string errorMessage = std::format(
			"Field not found"
			", field: {}",
			field
		);

		throw JsonFieldNotFound(errorMessage);
	}

	if (root == nullptr)
		return defaultValue;

	try
	{
		if (field == "")
		{
			if (root.type() == json::value_t::number_integer)
				return to_string(root.template get<int>());
			else if (root.type() == json::value_t::number_float)
				return to_string(root.template get<float>());
			else if (root.type() == json::value_t::object)
				return toString(root);
			else if (root.type() == json::value_t::array)
				return toString(root);
			else
				return root.template get<string>();
		}
		else
		{
			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return defaultValue;
			if (root.at(field).type() == json::value_t::number_integer || root.at(field).type() == json::value_t::number_float ||
				root.at(field).type() == json::value_t::boolean)
				return to_string(root.at(field));
			else
				return root.at(field);
		}
	}
	catch (json::out_of_range &e)
	{
		return defaultValue;
	}
}

int JSONUtils::asInt(json root, string field, int defaultValue, bool notFoundAsException)
{
	if (notFoundAsException && !isMetadataPresent(root, field))
	{
		string errorMessage = std::format(
			"Field not found"
			", field: {}",
			field
		);

		throw JsonFieldNotFound(errorMessage);
	}

	if (root == nullptr)
		return defaultValue;

	try
	{
		if (field == "")
		{
			if (root.type() == json::value_t::string)
			{
				try
				{
					return strtol(asString(root, "", "0").c_str(), nullptr, 10);
				}
				catch (exception &e)
				{
					return defaultValue;
				}
			}
			else
				return root.template get<int>();
		}
		else
		{
			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return defaultValue;
			if (root.at(field).type() == json::value_t::string)
			{
				try
				{
					return strtol(asString(root, field, "0").c_str(), nullptr, 10);
				}
				catch (exception &e)
				{
					return defaultValue;
				}
			}
			else
				return root.at(field);
		}
	}
	catch (json::out_of_range &e)
	{
		return defaultValue;
	}
}

int64_t JSONUtils::asInt64(json root, string field, int64_t defaultValue, bool notFoundAsException)
{
	if (notFoundAsException && !isMetadataPresent(root, field))
	{
		string errorMessage = std::format(
			"Field not found"
			", field: {}",
			field
		);

		throw JsonFieldNotFound(errorMessage);
	}

	if (root == nullptr)
		return defaultValue;

	try
	{
		if (field == "")
		{
			if (root.type() == json::value_t::string)
			{
				try
				{
					return strtoll(asString(root, "", "0").c_str(), nullptr, 10);
				}
				catch (exception &e)
				{
					return defaultValue;
				}
			}
			else
				return root.template get<int64_t>();
		}
		else
		{
			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return defaultValue;
			if (root.at(field).type() == json::value_t::string)
			{
				try
				{
					return strtoll(asString(root, field, "0").c_str(), nullptr, 10);
				}
				catch (exception &e)
				{
					return defaultValue;
				}
			}
			else
				return root.at(field);
		}
	}
	catch (json::out_of_range &e)
	{
		return defaultValue;
	}
}

double JSONUtils::asDouble(json root, string field, double defaultValue, bool notFoundAsException)
{
	if (notFoundAsException && !isMetadataPresent(root, field))
	{
		string errorMessage = std::format(
			"Field not found"
			", field: {}",
			field
		);

		throw JsonFieldNotFound(errorMessage);
	}

	if (root == nullptr)
		return defaultValue;

	try
	{
		if (field == "")
		{
			if (root.type() == json::value_t::string)
			{
				try
				{
					return stod(asString(root, "", "0"), nullptr);
				}
				catch (exception &e)
				{
					return defaultValue;
				}
			}
			else
				return root.template get<double>();
		}
		else
		{
			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return defaultValue;
			if (root.at(field).type() == json::value_t::string)
			{
				try
				{
					return stod(asString(root, field, "0"), nullptr);
				}
				catch (exception &e)
				{
					return defaultValue;
				}
			}
			else
				return root.at(field);
		}
	}
	catch (json::out_of_range &e)
	{
		return defaultValue;
	}
}

bool JSONUtils::asBool(json root, string field, bool defaultValue, bool notFoundAsException)
{
	if (notFoundAsException && !isMetadataPresent(root, field))
	{
		string errorMessage = std::format(
			"Field not found"
			", field: {}",
			field
		);

		throw JsonFieldNotFound(errorMessage);
	}

	if (root == nullptr)
		return defaultValue;

	try
	{
		if (field == "")
		{
			if (root.type() == json::value_t::string)
			{
				string sTrue = "true";

				bool isEqual = asString(root, "", "").length() != sTrue.length()
								   ? false
								   : equal(
										 asString(root, "", "").begin(), asString(root, "", "").end(), sTrue.begin(),
										 [](int c1, int c2) { return toupper(c1) == toupper(c2); }
									 );

				return isEqual ? true : false;
			}
			else
				return root.template get<bool>();
		}
		else
		{
			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return defaultValue;
			if (root.at(field).type() == json::value_t::string)
			{
				string sTrue = "true";

				bool isEqual = asString(root, field, "").length() != sTrue.length()
								   ? false
								   : equal(
										 asString(root, field, "").begin(), asString(root, field, "").end(), sTrue.begin(),
										 [](int c1, int c2) { return toupper(c1) == toupper(c2); }
									 );

				return isEqual ? true : false;
			}
			else
				return root.at(field);
		}
	}
	catch (json::out_of_range &e)
	{
		return defaultValue;
	}
}

json JSONUtils::asJson(json root, string field, json defaultValue, bool notFoundAsException)
{
	bool isPresent = isMetadataPresent(root, field);

	if (notFoundAsException && !isPresent)
	{
		string errorMessage = std::format(
			"Field not found"
			", field: {}",
			field
		);

		throw JsonFieldNotFound(errorMessage);
	}

	// è presente oppure non è presente ma non deve dare eccezione

	if (!isPresent)
		return defaultValue;
	else
		return root[field];
}

json JSONUtils::toJson(string j, bool warningIfError)
{
	try
	{
		if (j == "")
			return json();
		else
			return json::parse(j);
	}
	catch (json::parse_error &ex)
	{
		string errorMessage = std::format(
			"failed to parse the json"
			", json: {}"
			", at byte: {}",
			j, ex.byte
		);
		if (warningIfError)
			SPDLOG_WARN(errorMessage);
		else
			SPDLOG_ERROR(errorMessage);

		throw runtime_error(errorMessage);
	}
}

json JSONUtils::toJson(vector<int32_t> v)
{
	json root = json::array();
	for (int32_t i : v)
		root.push_back(i);
	return root;
}

json JSONUtils::toJson(vector<string> v)
{
	json root = json::array();
	for (string i : v)
		root.push_back(i);
	return root;
}

string JSONUtils::toString(json root)
{
	try
	{
		if (root == nullptr)
			return "null";
		else
			return root.dump(-1, ' ', true);
	}
	catch (const json::type_error &e)
	{
		throw runtime_error(e.what());
	}
	/*
		Json::StreamWriterBuilder wbuilder;
		wbuilder.settings_["emitUTF8"] = true;
		wbuilder.settings_["indentation"] = "";

		return Json::writeString(wbuilder, joValueRoot);
	*/
}

// Rimuove commenti singola riga e multi-riga
string JSONUtils::json5_removeComments(const string &input)
{
	// Rimuove i commenti "// ... \n"
	string no_single_line = regex_replace(input, regex(R"(\/\/[^\n]*)"), "");

	// Rimuove i commenti multi-riga "/* ... */" anche su più righe
	// string no_multi_line = regex_replace(no_single_line, regex(R"((?s)\/\*.*?\*\/)"), "");
	string no_multi_line = regex_replace(no_single_line, regex(R"(\/\*[\s\S]*?\*\/)"), "");

	return no_multi_line;
}

// Rimuove le virgole finali in oggetti e array
string JSONUtils::json5_removeTrailingCommas(const string &input) { return regex_replace(input, regex(R"(,\s*([\]}]))"), "$1"); }

// Mette le virgolette attorno a chiavi non quotate
string JSONUtils::json5_quoteUnquotedKeys(const string &input)
{
	// Cerca chiavi tipo: chiave: → "chiave":
	return regex_replace(input, regex(R"((\s*)([a-zA-Z_][a-zA-Z0-9_]*)(\s*):)"), "$1\"$2\"$3:");
}

// Funzione completa: JSON5 → JSON standard
string JSONUtils::json5ToJson(const string &json5)
{
	string cleaned = json5_removeComments(json5);
	cleaned = json5_removeTrailingCommas(cleaned);
	cleaned = json5_quoteUnquotedKeys(cleaned);

	return cleaned;
}

json JSONUtils::loadConfigurationFile(string configurationPathName, string environmentPrefix)
{

#ifdef BOOTSERVICE_DEBUG_LOG
#ifdef _WIN32
	ofstream of("C:\\bootservice.log", ofstream::app);
#else
	ofstream of("/tmp/bootservice.log", ofstream::app);
#endif
	of << "loadConfigurationFile..." << endl;
#endif

	string sConfigurationFile;
	try
	{
		ifstream configurationFile(configurationPathName, ifstream::binary);
		stringstream buffer;
		buffer << configurationFile.rdbuf();
		if (environmentPrefix == "")
			sConfigurationFile = buffer.str();
		else
			sConfigurationFile = applyEnvironmentToConfiguration(buffer.str(), environmentPrefix);

		json configurationRoot = json::parse(
			sConfigurationFile,
			nullptr, // callback
			true,	 // allow exceptions
			true	 // ignore_comments
		);

		return configurationRoot;
	}
	catch (exception &e)
	{
#ifdef BOOTSERVICE_DEBUG_LOG
#ifdef _WIN32
		ofstream of("C:\\bootservice.log", ofstream::app);
#else
		ofstream of("/tmp/bootservice.log", ofstream::app);
#endif
		of << "loadConfigurationFile failed, configurationPathName: " << configurationPathName << ", exception: " << e << endl;
#endif
		throw;
	}
}

string JSONUtils::applyEnvironmentToConfiguration(string configuration, string environmentPrefix)
{
#ifdef _WIN32
	char **s = _environ;
#else
	char **s = environ;
#endif

#ifdef BOOTSERVICE_DEBUG_LOG
#ifdef _WIN32
	ofstream of("C:\\bootservice.log", ofstream::app);
#else
	ofstream of("/tmp/bootservice.log", ofstream::app);
#endif
#endif

	int envNumber = 0;
	for (; *s; s++)
	{
		string envVariable = *s;
#ifdef BOOTSERVICE_DEBUG_LOG
//          of << "ENV " << *s << endl;
#endif
		if (envVariable.starts_with(environmentPrefix))
		{
			size_t endOfVarName = envVariable.find("=");
			if (endOfVarName == string::npos)
				continue;

			envNumber++;

			// sarebbe \$\{CATRAMMSGUIAPPS_PATH\}
			string envLabel = std::format("\\$\\{{{}\\}}", envVariable.substr(0, endOfVarName));
			string envValue = envVariable.substr(endOfVarName + 1);
#ifdef BOOTSERVICE_DEBUG_LOG
			of << "ENV " << envLabel << ": " << envValue << endl;
#endif
			configuration = regex_replace(configuration, regex(envLabel), envValue);
		}
	}

	return configuration;
}
