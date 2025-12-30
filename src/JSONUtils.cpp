#include "JSONUtils.h"
#include <format>
#include <fstream>
#include <regex>

#ifdef _WIN32
	extern char **_environ;
#else
extern char **environ;
#endif


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

// metodo aggiunto a JSONUtils solo perchè utilizzato da loadConfigurationFile.
// Avevo pensato di aggiungerlo a StringUtils creando una dipendenza tra le due librerie.
// Ho preferito evitare questa dipendenza.
string JSONUtils::applyEnvironmentToConfiguration(string configuration, const string_view &environmentPrefix)
{
#ifdef _WIN32
	char **s = _environ;
#else
	char **s = environ;
#endif

	int envNumber = 0;
	for (; *s; s++)
	{
		string envVariable = *s;
		if (envVariable.starts_with(environmentPrefix))
		{
			size_t endOfVarName = envVariable.find('=');
			if (endOfVarName == string::npos)
				continue;

			envNumber++;

			// sarebbe \$\{CATRAMMSGUIAPPS_PATH\}
			string envLabel = std::format(R"(\$\{{{}\}})", envVariable.substr(0, endOfVarName));
			string envValue = envVariable.substr(endOfVarName + 1);
			configuration = regex_replace(configuration, regex(envLabel), envValue);
		}
	}

	return configuration;
}
