/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   JSONUtils.h
 * Author: giuliano
 *
 * Created on March 29, 2018, 6:27 AM
 */

#pragma once

#include <utility>
#include <iostream>

#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"

using namespace std;

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;
using namespace nlohmann::literals;

struct JsonFieldNotFound : public exception
{
	string _errorMessage;

	explicit JsonFieldNotFound(string_view errorMessage):_errorMessage(errorMessage){};

	[[nodiscard]] char const *what() const noexcept override { return _errorMessage.c_str(); };
};

class JSONUtils
{
  public:
	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static bool isMetadataPresent(const J &root, string_view field)
	{
		if (root == nullptr)
			return false;
		return root.contains(field);
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static bool isNull(const J &root, string_view field)
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

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static string asString(const J &root, string_view field = "", const string_view defaultValue = "", bool notFoundAsException = false)
	{
		if (notFoundAsException && !isMetadataPresent(root, field))
		{
			const string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return string(defaultValue);

		try
		{
			if (field.empty())
			{
				switch (root.type())
				{
				case json::value_t::number_integer:
					return std::format("{}", root.template get<int>());
				case json::value_t::number_unsigned:
					return std::format("{}", root.template get<unsigned>());
				case json::value_t::boolean:
					return std::format("{}", root.template get<bool>());
				case json::value_t::number_float:
					return to_string(root.template get<float>());
				case json::value_t::object:
					return toString(root);
				case json::value_t::array:
					return toString(root);
				case json::value_t::string:
					return root.template get<string>();
				default:
					SPDLOG_ERROR("asString, type not managed: {}", static_cast<int>(root.type()));
					return string(defaultValue);
				}
			}

			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return string(defaultValue);
			if (root.at(field).type() == json::value_t::number_integer || root.at(field).type() == json::value_t::number_float ||
				root.at(field).type() == json::value_t::boolean)
				return to_string(root.at(field));
			return root.at(field);
		}
		catch (json::out_of_range &e)
		{
			return string(defaultValue);
		}
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static optional<string> asOptString(const J &root, string_view field = "", bool notFoundAsException = false)
	{
		if (notFoundAsException && !isMetadataPresent(root, field))
		{
			const string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return nullopt;

		try
		{
			if (field.empty())
			{
				switch (root.type())
				{
				case json::value_t::number_integer:
					return std::format("{}", root.template get<int>());
				case json::value_t::number_unsigned:
					return std::format("{}", root.template get<unsigned>());
				case json::value_t::boolean:
					return std::format("{}", root.template get<bool>());
				case json::value_t::number_float:
					return to_string(root.template get<float>());
				case json::value_t::object:
					return toString(root);
				case json::value_t::array:
					return toString(root);
				case json::value_t::string:
					return root.template get<string>();
				default:
					SPDLOG_ERROR("asString, type not managed: {}", static_cast<int>(root.type()));
					return nullopt;
				}
			}

			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return nullopt;
			if (root.at(field).type() == json::value_t::number_integer || root.at(field).type() == json::value_t::number_float ||
				root.at(field).type() == json::value_t::boolean)
				return to_string(root.at(field));
			return root.at(field);
		}
		catch (json::out_of_range &e)
		{
			return nullopt;
		}
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static int32_t asInt32(const J &root, string_view field = "", const int32_t defaultValue = 0, bool notFoundAsException = false)
	{
		if (notFoundAsException && !isMetadataPresent(root, field))
		{
			const string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return defaultValue;

		try
		{
			if (field.empty())
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
				return root.template get<int32_t>();
			}

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
			return root.at(field);
		}
		catch (json::out_of_range &e)
		{
			return defaultValue;
		}
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static optional<int32_t> asOptInt32(const J &root, string_view field = "", bool notFoundAsException = false)
	{
		if (notFoundAsException && !isMetadataPresent(root, field))
		{
			const string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return nullopt;

		try
		{
			if (field.empty())
			{
				if (root.type() == json::value_t::string)
				{
					try
					{
						return strtol(asString(root, "", "0").c_str(), nullptr, 10);
					}
					catch (exception &e)
					{
						return nullopt;
					}
				}
				return root.template get<int32_t>();
			}

			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return nullopt;
			if (root.at(field).type() == json::value_t::string)
			{
				try
				{
					return strtol(asString(root, field, "0").c_str(), nullptr, 10);
				}
				catch (exception &e)
				{
					return nullopt;
				}
			}
			return root.at(field);
		}
		catch (json::out_of_range &e)
		{
			return nullopt;
		}
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static int64_t asInt64(const J &root, string_view field = "", const int64_t defaultValue = 0, bool notFoundAsException = false)
	{
		if (notFoundAsException && !isMetadataPresent(root, field))
		{
			const string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return defaultValue;

		try
		{
			if (field.empty())
			{
				if (root.type() == json::value_t::string)
				{
					try
					{
						return stoll(asString(root, "", "0").c_str());
					}
					catch (exception &e)
					{
						return defaultValue;
					}
				}
				return root.template get<int64_t>();
			}

			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return defaultValue;
			if (root.at(field).type() == json::value_t::string)
			{
				try
				{
					return stoll(asString(root, field, "0").c_str());
				}
				catch (exception &e)
				{
					return defaultValue;
				}
			}
			return root.at(field);
		}
		catch (json::out_of_range &e)
		{
			return defaultValue;
		}
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static optional<int64_t> asOptInt64(const J &root, string_view field = "", bool notFoundAsException = false)
	{
		if (notFoundAsException && !isMetadataPresent(root, field))
		{
			const string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return nullopt;

		try
		{
			if (field.empty())
			{
				if (root.type() == json::value_t::string)
				{
					try
					{
						return stoll(asString(root, "", "0").c_str());
					}
					catch (exception &e)
					{
						return nullopt;
					}
				}
				return root.template get<int64_t>();
			}

			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return nullopt;
			if (root.at(field).type() == json::value_t::string)
			{
				try
				{
					return stoll(asString(root, field, "0").c_str());
				}
				catch (exception &e)
				{
					return nullopt;
				}
			}
			return root.at(field);
		}
		catch (json::out_of_range &e)
		{
			return nullopt;
		}
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static uint64_t asUint64(const J &root, string_view field = "", const uint64_t defaultValue = 0, bool notFoundAsException = false)
	{
		if (notFoundAsException && !isMetadataPresent(root, field))
		{
			const string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return defaultValue;

		try
		{
			if (field.empty())
			{
				if (root.type() == json::value_t::string)
				{
					try
					{
						return stoull(asString(root, "", "0").c_str());
					}
					catch (exception &e)
					{
						return defaultValue;
					}
				}
				return root.template get<uint64_t>();
			}

			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return defaultValue;
			if (root.at(field).type() == json::value_t::string)
			{
				try
				{
					return stoull(asString(root, field, "0").c_str());
				}
				catch (exception &e)
				{
					return defaultValue;
				}
			}
			return root.at(field);
		}
		catch (json::out_of_range &e)
		{
			return defaultValue;
		}
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static optional<uint64_t> asOptUint64(const J &root, string_view field = "", bool notFoundAsException = false)
	{
		if (notFoundAsException && !isMetadataPresent(root, field))
		{
			string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return nullopt;

		try
		{
			if (field.empty())
			{
				if (root.type() == json::value_t::string)
				{
					try
					{
						return stoull(asString(root, "", "0").c_str());
					}
					catch (exception &e)
					{
						return nullopt;
					}
				}
				return root.template get<uint64_t>();
			}

			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return nullopt;
			if (root.at(field).type() == json::value_t::string)
			{
				try
				{
					return stoull(asString(root, field, "0").c_str());
				}
				catch (exception &e)
				{
					return nullopt;
				}
			}
			return root.at(field);
		}
		catch (json::out_of_range &e)
		{
			return nullopt;
		}
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static double asDouble(const J &root, string_view field = "", const double defaultValue = 0.0, bool notFoundAsException = false)
	{
		if (notFoundAsException && !isMetadataPresent(root, field))
		{
			const string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return defaultValue;

		try
		{
			if (field.empty())
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
				return root.template get<double>();
			}

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
			return root.at(field);
		}
		catch (json::out_of_range &e)
		{
			return defaultValue;
		}
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static optional<double> asOptDouble(const J &root, string_view field = "", bool notFoundAsException = false)
	{
		if (notFoundAsException && !isMetadataPresent(root, field))
		{
			const string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return nullopt;

		try
		{
			if (field.empty())
			{
				if (root.type() == json::value_t::string)
				{
					try
					{
						return stod(asString(root, "", "0"), nullptr);
					}
					catch (exception &e)
					{
						return nullopt;
					}
				}
				return root.template get<double>();
			}

			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return nullopt;
			if (root.at(field).type() == json::value_t::string)
			{
				try
				{
					return stod(asString(root, field, "0"), nullptr);
				}
				catch (exception &e)
				{
					return nullopt;
				}
			}
			return root.at(field);
		}
		catch (json::out_of_range &e)
		{
			return nullopt;
		}
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static bool asBool(const J &root, string_view field, const bool defaultValue = false, bool notFoundAsException = false)
	{
		if (notFoundAsException && !isMetadataPresent(root, field))
		{
			const string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return defaultValue;

		try
		{
			if (field.empty())
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
				return root.template get<bool>();
			}

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
			return root.at(field);
		}
		catch (json::out_of_range &e)
		{
			return defaultValue;
		}
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static optional<bool> asOptBool(const J &root, string_view field, bool notFoundAsException = false)
	{
		if (notFoundAsException && !isMetadataPresent(root, field))
		{
			const string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return nullopt;

		try
		{
			if (field.empty())
			{
				if (root.type() == json::value_t::string)
				{
					string sTrue = "true";

					const bool isEqual = asString(root, "", "").length() != sTrue.length()
									   ? false
									   : equal(
											 asString(root, "", "").begin(), asString(root, "", "").end(), sTrue.begin(),
											 [](const int c1, const int c2) { return toupper(c1) == toupper(c2); }
										 );

					return isEqual ? true : false;
				}
				return root.template get<bool>();
			}

			if (!JSONUtils::isMetadataPresent(root, field) || JSONUtils::isNull(root, field))
				return nullopt;
			if (root.at(field).type() == json::value_t::string)
			{
				string sTrue = "true";

				const bool isEqual = asString(root, field, "").length() != sTrue.length()
								   ? false
								   : equal(
										 asString(root, field, "").begin(), asString(root, field, "").end(), sTrue.begin(),
										 [](const int c1, const int c2) { return toupper(c1) == toupper(c2); }
									 );

				return isEqual ? true : false;
			}
			return root.at(field);
		}
		catch (json::out_of_range &e)
		{
			return nullopt;
		}
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static J asJson(const J &root, string_view field, J defaultValue = J(), const bool notFoundAsException = false)
	{
		bool isPresent = isMetadataPresent(root, field);

		if (notFoundAsException && !isPresent)
		{
			const string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		// è presente oppure non è presente ma non deve dare eccezione
		if (!isPresent)
			return defaultValue;
		return root[field];
	}

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static optional<J> asOptJson(const J &root, string_view field, const bool notFoundAsException = false)
	{
		bool isPresent = isMetadataPresent(root, field);

		if (notFoundAsException && !isPresent)
		{
			const string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		// è presente oppure non è presente ma non deve dare eccezione
		if (!isPresent)
			return nullopt;
		return root[field];
	}

	static json toJson(const string_view &json, bool warningIfError = false);
	static ordered_json toOrderedJson(const string_view &json, bool warningIfError = false);

	template <typename J>
	requires is_same_v<J, json> || is_same_v<J, ordered_json>
	static string toString(const J &root, int indent = -1)
	{
		try
		{
			if (root == nullptr)
				return "null";
			else
				return root.dump(indent, ' ', true);
		}
		catch (const json::type_error &e)
		{
			throw runtime_error(e.what());
		}
	}

	static json toJson(const vector<int32_t> &v);
	static ordered_json toOrderedJson(const vector<int32_t> &v);
	static json toJson(const vector<string> &v);
	static ordered_json toOrderedJson(const vector<string> &v);

	static string json5ToJson(const string &json5);
	static json loadConfigurationFile(const string_view &configurationPathName, const string_view &environmentPrefix = "");
	static string applyEnvironmentToConfiguration(string configuration, const string_view &environmentPrefix);

  private:
	static string json5_removeComments(const string &input);
	static string json5_removeTrailingCommas(const string &input);
	static string json5_quoteUnquotedKeys(const string &input);
};
