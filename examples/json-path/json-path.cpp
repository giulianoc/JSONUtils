
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

#include "JsonPath.h"
#include <iostream>

using namespace std;
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

int main()
{

	const string j = R"({
        "key1": "value",
		"key2": {
			"key3": {
				"key4": 12
			}
		},
		"key5": [
			"aa",
			"bb"
		],
        "another": 42,
        "stringToInt": "42",
        "list": [1, 2, 3]
    })";

	const json root = JSONUtils::toJson<json>(j);

	cout << "key1: " << JsonPath(&root)["key1"].as<string>("") << endl << endl << endl;
	cout << "key2: " << JsonPath(&root)["key2"].as<json>(json()) << endl << endl << endl;
	cout << "key4: " << JsonPath(&root)["key2"]["key3"]["key4"].as<int32_t>(-1) << endl << endl << endl;
	cout << "field not existing (default): " << JsonPath(&root)["key2"]["key3"]["key5"].as<int32_t>(-1) << endl << endl << endl;
	cout << "field not existing (default): " << JsonPath(&root)["key999"]["key3"]["key5"].as<int32_t>(-1) << endl << endl << endl;
	try
	{
		cout << "field not existing (exception): " << JsonPath(&root).required()["key999"]["key3"]["key5"].as<int32_t>(-1) << endl << endl << endl;
	}
	catch (const std::exception &e)
	{
		// la lib logga già l'errore, inutile riloggarlo
		// cout << "field not existing (exception): " << e.what() << endl << endl;
	}

	try
	{
		cout << "key5: " << JsonPath(&root)["key5"][0].as<int32_t>(-1) << endl << endl << endl;
	}
	catch (const std::exception &e)
	{
		// la lib logga già l'errore, inutile riloggarlo
		// cout << "exception: " << e.what() << endl << endl;
	}

	for (auto& item : JsonPath(&root)["list"].as<json>(json::array()))
	{
		cout << "item: " << JsonPath(&item).as<int32_t>(-1) << endl;
	}
	/*
	cout << "key1" << endl;
	cout << "val: " << JSONUtils::as<string>(root, "key1", "ciao") << endl << endl;

	cout << "stringToInt" << endl;
	try
	{
		cout << "val: " << JSONUtils::as<int32_t>(root, "stringToInt", 10) << endl << endl;
	}
	catch (const std::exception &e)
	{
		cout << "exception: " << e.what() << endl << endl;
	}

	cout << "list" << endl;
	for (auto &item : JSONUtils::as<json>(root, "list", json(nullptr)))
	{
		cout << "item: " << JSONUtils::as<int32_t>(item) << endl;
	}
	*/
}
