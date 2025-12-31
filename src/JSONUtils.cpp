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
std::string JSONUtils::json5_removeComments(const std::string &input)
{
	// Rimuove i commenti "// ... \n"
	std::string no_single_line = regex_replace(input, std::regex(R"(\/\/[^\n]*)"), "");

	// Rimuove i commenti multi-riga "/* ... */" anche su più righe
	// std::string no_multi_line = regex_replace(no_single_line, regex(R"((?s)\/\*.*?\*\/)"), "");
	std::string no_multi_line = regex_replace(no_single_line, std::regex(R"(\/\*[\s\S]*?\*\/)"), "");

	return no_multi_line;
}

// Rimuove le virgole finali in oggetti e array
std::string JSONUtils::json5_removeTrailingCommas(const std::string &input) { return regex_replace(input, std::regex(R"(,\s*([\]}]))"), "$1"); }

// Mette le virgolette attorno a chiavi non quotate
std::string JSONUtils::json5_quoteUnquotedKeys(const std::string &input)
{
	// Cerca chiavi tipo: chiave: → "chiave":
	return regex_replace(input, std::regex(R"((\s*)([a-zA-Z_][a-zA-Z0-9_]*)(\s*):)"), "$1\"$2\"$3:");
}

// Funzione completa: JSON5 → JSON standard
std::string JSONUtils::json5ToJson(const std::string &json5)
{
	std::string cleaned = json5_removeComments(json5);
	cleaned = json5_removeTrailingCommas(cleaned);
	cleaned = json5_quoteUnquotedKeys(cleaned);

	return cleaned;
}

// metodo aggiunto a JSONUtils solo perchè utilizzato da loadConfigurationFile.
// Avevo pensato di aggiungerlo a StringUtils creando una dipendenza tra le due librerie.
// Ho preferito evitare questa dipendenza.
std::string JSONUtils::applyEnvironmentToConfiguration(std::string configuration, const std::string_view &environmentPrefix)
{
#ifdef _WIN32
	char **s = _environ;
#else
	char **s = environ;
#endif

	int envNumber = 0;
	for (; *s; s++)
	{
		std::string envVariable = *s;
		if (envVariable.starts_with(environmentPrefix))
		{
			size_t endOfVarName = envVariable.find('=');
			if (endOfVarName == std::string::npos)
				continue;

			envNumber++;

			// sarebbe \$\{CATRAMMSGUIAPPS_PATH\}
			std::string envLabel = std::format(R"(\$\{{{}\}})", envVariable.substr(0, endOfVarName));
			std::string envValue = envVariable.substr(endOfVarName + 1);
			configuration = regex_replace(configuration, std::regex(envLabel), envValue);
		}
	}

	return configuration;
}
