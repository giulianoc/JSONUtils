
/*
 Copyright (C) Giuliano Catrambone (giuliano.catrambone@catrasoftware.it)

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Commercial use other than under the terms of the GNU General Public
 License is allowed only after express negotiation of conditions
 with the authors.
*/

#include "JSONUtils.h"
#include <iostream>

using namespace std;
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

int main()
{

	string json5 = R"({
        // Questo è un commento
        key: "value", // commento in linea
        another: 42,
        list: [1, 2, 3,], /* trailing comma 
    aaaaa */
    })";

	// Normalizza JSON5 → JSON
	string json = JSONUtils::json5ToJson(json5);

	cout << "json5: " << json5 << endl << endl;
	cout << "json: " << json << endl << endl;

	// Ora puoi usare nlohmann::json
	// auto parsed = nlohmann::json::parse(clean_json);

	// std::cout << "Chiave key: " << parsed["key"] << std::endl;
}
