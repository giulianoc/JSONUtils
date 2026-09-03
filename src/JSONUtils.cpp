#include "JSONUtils.h"
#include <format>
#include <fstream>
#include <regex>

#ifdef _WIN32
	extern char **_environ;
#else
extern char **environ;
#endif


// Copia in out la stringa che inizia in input[i] (apice singolo o doppio) normalizzandola
// in una stringa JSON. Restituisce la posizione del carattere successivo alla stringa.
size_t JSONUtils::json5_copyString(const std::string &input, size_t i, std::string &out)
{
	const char quote = input[i++];

	out += '"';
	while (i < input.size() && input[i] != quote)
	{
		if (input[i] == '\\' && i + 1 < input.size())
		{
			// \' non è un escape valido in JSON
			if (input[i + 1] == '\'')
				out += '\'';
			else
			{
				out += input[i];
				out += input[i + 1];
			}
			i += 2;
			continue;
		}

		// un apice doppio non escapato è lecito dentro una stringa con apici singoli
		if (input[i] == '"')
			out += '\\';
		out += input[i++];
	}
	out += '"';

	return i < input.size() ? i + 1 : i;
}

// JSON5 → JSON standard: rimuove i commenti, mette le virgolette attorno alle chiavi non
// quotate, converte le stringhe con apici singoli e toglie le virgole finali.
// A differenza di una serie di regex applicate all'intero testo, la scansione è unica e
// salta il contenuto delle stringhe: un "https://..." o un "chiave: valore" restano intatti.
// Non sono gestiti (non servono nei casi d'uso attuali): numeri esadecimali, numeri con
// segno o punto iniziale/finale, NaN/Infinity, stringhe su più righe.
std::string JSONUtils::json5ToJson(const std::string &json5)
{
	std::string json;
	json.reserve(json5.size());

	// posizione in json dell'ultima virgola emessa, npos se dopo di essa è già stato
	// emesso qualcosa che non sia uno spazio
	size_t lastComma = std::string::npos;

	size_t i = 0;
	while (i < json5.size())
	{
		const char c = json5[i];

		// commento su singola riga
		if (c == '/' && i + 1 < json5.size() && json5[i + 1] == '/')
		{
			i = json5.find('\n', i + 2);
			if (i == std::string::npos)
				break;
			continue;
		}

		// commento su più righe
		if (c == '/' && i + 1 < json5.size() && json5[i + 1] == '*')
		{
			i = json5.find("*/", i + 2);
			if (i == std::string::npos)
				break;
			i += 2;
			continue;
		}

		// gli spazi non invalidano l'eventuale virgola finale in sospeso
		if (std::isspace(static_cast<unsigned char>(c)))
		{
			json += c;
			i++;
			continue;
		}

		// la virgola viene emessa e poi rimossa se il primo elemento significativo che la
		// segue è una chiusura di oggetto o di array
		if (c == ',')
		{
			lastComma = json.size();
			json += c;
			i++;
			continue;
		}

		if ((c == '}' || c == ']') && lastComma != std::string::npos)
			json.erase(lastComma, 1);
		lastComma = std::string::npos;

		if (c == '"' || c == '\'')
		{
			i = json5_copyString(json5, i, json);
			continue;
		}

		// chiave non quotata: un identificatore seguito da ':'
		if (std::isalpha(static_cast<unsigned char>(c)) || c == '_' || c == '$')
		{
			size_t end = i;
			while (end < json5.size() &&
				   (std::isalnum(static_cast<unsigned char>(json5[end])) || json5[end] == '_' || json5[end] == '$'))
				end++;

			// true, false, null e gli altri valori non quotati restano tali
			if (const size_t next = json5.find_first_not_of(" \t\r\n", end);
				next != std::string::npos && json5[next] == ':')
				json += '"' + json5.substr(i, end - i) + '"';
			else
				json += json5.substr(i, end - i);
			i = end;
			continue;
		}

		json += c;
		i++;
	}

	return json;
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
