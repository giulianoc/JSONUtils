
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

int main()
{

	json j = R"({
        "key1": "value",
        "another": 42,
        "list": [1, 2, 3]
    })";

	// path not existing, default value returned
	cout << "value: " << JSONUtils::toString(JSONUtils::asJson(j, "nonExistingKey", json(nullptr))) << endl << endl;
	cout << "value: " << JSONUtils::asInt32(j, "nonExistingKey", 43) << endl << endl;
	if (j.contains("key") && j["key"].is_object()) {
		const json &k = j["key"];
		if (k.contains("key2") && k["key2"].is_object()) {
			cout << "value: " << JSONUtils::asInt32(k["key2"], "value", 42) << endl << endl;
		}
	}
}
