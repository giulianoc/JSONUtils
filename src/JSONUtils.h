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

#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include <format>
#include <utility>
#include <iostream>
#include <fstream>

struct JsonFieldNotFound final : public std::exception
{
	std::string _errorMessage;

	explicit JsonFieldNotFound(std::string_view errorMessage):_errorMessage(errorMessage){};

	[[nodiscard]] char const *what() const noexcept override { return _errorMessage.c_str(); };
};

class JSONUtils
{
  public:
	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static bool isPresent(const J &root, std::string_view field, const bool checksAlsoNotNull = false)
	{
		if (root == nullptr)
			return false;
		if (checksAlsoNotNull)
			return root.is_object() && root.contains(field) && !root.at(field).is_null();
		return root.is_object() && root.contains(field);

		// Questa implementazione potrebbe sostituire l'implementazione sopra
		// return root.is_object() && root.contains(std::string(field)) &&
		//   (!checksAlsoNotNull || !root.at(std::string(field)).is_null());
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static bool isNull(const J &root, std::string_view field)
	{
		return isPresent(root, field) && root[field].is_null();
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static J* jpath(J& root, const std::initializer_list<std::string_view> fields)
	{
		J* current = &root;

		for (auto field : fields)
		{
			if (!current->is_object())
				return nullptr;

			auto it = current->find(field);
			if (it == current->end())
				return nullptr;

			current = &(*it);
		}
		return current;
	}

	template <typename T, typename J>
	static T as(const J& root, std::string_view field = {}, T defaultVal = {}, const bool exceptionOnMissing = false)
	{
		try
		{
			if (root == nullptr)
			{
				if (exceptionOnMissing)
				{
					const std::string errorMessage = std::format("Field [{}] not found", field);
					SPDLOG_ERROR(errorMessage);
					throw JsonFieldNotFound(errorMessage);
				}
				return defaultVal;
			}
			if (field.empty())
				return root.template get<T>();
			if (!JSONUtils::isPresent(root, field))
			{
				if (exceptionOnMissing)
				{
					std::string errorMessage = std::format("Field [{}] not found", field);
					SPDLOG_ERROR(errorMessage);
					throw JsonFieldNotFound(errorMessage);
				}
				return defaultVal;
			}
			return root.at(field).template get<T>();
		}
		catch (const std::exception & e)
		{
			// abbiamo una eccezione se ad es. chiediamo una stringa (as<string>) ma il valore è un numero
			std::string errorMessage;
			if (field.empty())
				errorMessage = std::format("Json: {}, exception: {}", JSONUtils::toString(root), e.what());
			else
				errorMessage = std::format("Field: {}, exception: {}", field, e.what());
			SPDLOG_ERROR(errorMessage);
			throw;
		}
	}

	template <typename T, typename J>
	static T as(const J* root, std::string_view field = {}, T defaultVal = {}, const bool exceptionOnMissing = false)
	{
		if (!root)
			return defaultVal;
		return as(*root, field, defaultVal, exceptionOnMissing);
	}

	template <typename T, typename J>
	static std::optional<T> asOpt(const J& root, std::string_view field = {})
	{
		try
		{
			if (root == nullptr)
				return std::nullopt;
			if (field.empty())
				return root.template get<T>();
			if (!JSONUtils::isPresent(root, field))
				return std::nullopt;
			return root.at(field).template get<T>();
		}
		catch (const std::exception & e)
		{
			// abbiamo una eccezione se ad es. chiediamo una stringa (as<string>) ma il valore è un numero
			std::string errorMessage;
			if (field.empty())
				errorMessage = std::format("Json: {}, exception: {}", JSONUtils::toString(root), e.what());
			else
				errorMessage = std::format("Field: {}, exception: {}", field, e.what());
			SPDLOG_ERROR(errorMessage);
			throw;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	// [[deprecated("This method is deprecated. Use as<string>() instead.")]]
	static std::string asString(const J &root, std::string_view field = "", const std::string_view defaultValue = "", bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			const std::string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return std::string(defaultValue);

		try
		{
			if (field.empty())
			{
				switch (root.type())
				{
				case nlohmann::json::value_t::number_integer:
					return std::format("{}", root.template get<int>());
				case nlohmann::json::value_t::number_unsigned:
					return std::format("{}", root.template get<unsigned>());
				case nlohmann::json::value_t::boolean:
					return std::format("{}", root.template get<bool>());
				case nlohmann::json::value_t::number_float:
					return std::format("{}", root.template get<float>());
				case nlohmann::json::value_t::object:
					return toString(root);
				case nlohmann::json::value_t::array:
					return toString(root);
				case nlohmann::json::value_t::string:
					return root.template get<std::string>();
				default:
					SPDLOG_ERROR("asString, type not managed: {}", static_cast<int>(root.type()));
					return std::string(defaultValue);
				}
			}

			if (!isPresent(root, field) || JSONUtils::isNull(root, field))
				return std::string(defaultValue);
			if (root.at(field).type() == nlohmann::json::value_t::number_integer)
				return std::to_string(root.at(field).template get<int>());
			if (root.at(field).type() == nlohmann::json::value_t::number_float)
				return std::to_string(root.at(field).template get<double>());
			if (root.at(field).type() == nlohmann::json::value_t::boolean)
				return std::to_string(static_cast<int>(root.at(field).template get<bool>()));
			return root.at(field);
		}
		catch (nlohmann::json::out_of_range &e)
		{
			return std::string(defaultValue);
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::string asString(const J *root, std::string_view field = "", const std::string_view defaultValue = "",
		bool exceptionOnMissing = false)
	{
		if (!root)
			return std::string(defaultValue);
		return asString(*root, field, defaultValue, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<std::string> asOptString(const J &root, std::string_view field = "", bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			const std::string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return std::nullopt;

		try
		{
			if (field.empty())
			{
				switch (root.type())
				{
				case nlohmann::json::value_t::number_integer:
					return std::format("{}", root.template get<int>());
				case nlohmann::json::value_t::number_unsigned:
					return std::format("{}", root.template get<unsigned>());
				case nlohmann::json::value_t::boolean:
					return std::format("{}", root.template get<bool>());
				case nlohmann::json::value_t::number_float:
					return std::format("{}", root.template get<float>());
				case nlohmann::json::value_t::object:
					return toString(root);
				case nlohmann::json::value_t::array:
					return toString(root);
				case nlohmann::json::value_t::string:
					return root.template get<std::string>();
				default:
					SPDLOG_ERROR("asString, type not managed: {}", static_cast<int>(root.type()));
					return std::nullopt;
				}
			}

			if (!isPresent(root, field) || JSONUtils::isNull(root, field))
				return std::nullopt;
			if (root.at(field).type() == nlohmann::json::value_t::number_integer || root.at(field).type() == nlohmann::json::value_t::number_float ||
				root.at(field).type() == nlohmann::json::value_t::boolean)
				return std::format("{}", root.at(field));
			return root.at(field);
		}
		catch (nlohmann::json::out_of_range &e)
		{
			return std::nullopt;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<std::string> asOptString(const J *root, std::string_view field = "", bool exceptionOnMissing = false)
	{
		if (!root)
			return std::nullopt;
		return asOptString(*root, field, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static int32_t asInt32(const J &root, std::string_view field = "", const int32_t defaultValue = 0, bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			const std::string errorMessage = std::format(
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
				if (root.type() == nlohmann::json::value_t::string)
				{
					try
					{
						return strtol(asString(root, "", "0").c_str(), nullptr, 10);
					}
					catch (std::exception &e)
					{
						return defaultValue;
					}
				}
				return root.template get<int32_t>();
			}

			if (!isPresent(root, field) || JSONUtils::isNull(root, field))
				return defaultValue;
			if (root.at(field).type() == nlohmann::json::value_t::string)
			{
				try
				{
					return strtol(asString(root, field, "0").c_str(), nullptr, 10);
				}
				catch (std::exception &e)
				{
					return defaultValue;
				}
			}
			return root.at(field);
		}
		catch (nlohmann::json::out_of_range &e)
		{
			return defaultValue;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static int32_t asInt32(const J *root, std::string_view field = "", const int32_t defaultValue = 0, bool exceptionOnMissing = false)
	{
		if (!root)
			return defaultValue;
		return asInt32(*root, field, defaultValue, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<int32_t> asOptInt32(const J &root, std::string_view field = "", bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			const std::string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return std::nullopt;

		try
		{
			if (field.empty())
			{
				if (root.type() == nlohmann::json::value_t::string)
				{
					try
					{
						return strtol(asString(root, "", "0").c_str(), nullptr, 10);
					}
					catch (std::exception &e)
					{
						return std::nullopt;
					}
				}
				return root.template get<int32_t>();
			}

			if (!isPresent(root, field) || JSONUtils::isNull(root, field))
				return std::nullopt;
			if (root.at(field).type() == nlohmann::json::value_t::string)
			{
				try
				{
					return strtol(asString(root, field, "0").c_str(), nullptr, 10);
				}
				catch (std::exception &e)
				{
					return std::nullopt;
				}
			}
			return root.at(field);
		}
		catch (nlohmann::json::out_of_range &e)
		{
			return std::nullopt;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<int32_t> asOptInt32(const J *root, std::string_view field = "", bool exceptionOnMissing = false)
	{
		if (!root)
			return std::nullopt;
		return asOptInt32(*root, field, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static int64_t asInt64(const J &root, std::string_view field = "", const int64_t defaultValue = 0, bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			const std::string errorMessage = std::format(
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
				if (root.type() == nlohmann::json::value_t::string)
				{
					try
					{
						return std::stoll(asString(root, "", "0").c_str());
					}
					catch (std::exception &e)
					{
						return defaultValue;
					}
				}
				return root.template get<int64_t>();
			}

			if (!isPresent(root, field) || JSONUtils::isNull(root, field))
				return defaultValue;
			if (root.at(field).type() == nlohmann::json::value_t::string)
			{
				try
				{
					return std::stoll(asString(root, field, "0").c_str());
				}
				catch (std::exception &e)
				{
					return defaultValue;
				}
			}
			return root.at(field);
		}
		catch (nlohmann::json::out_of_range &e)
		{
			return defaultValue;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static int64_t asInt64(const J *root, std::string_view field = "", const int64_t defaultValue = 0, bool exceptionOnMissing = false)
	{
		if (!root)
			return defaultValue;
		return asInt64(*root, field, defaultValue, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<int64_t> asOptInt64(const J &root, std::string_view field = "", bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			const std::string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return std::nullopt;

		try
		{
			if (field.empty())
			{
				if (root.type() == nlohmann::json::value_t::string)
				{
					try
					{
						return std::stoll(asString(root, "", "0").c_str());
					}
					catch (std::exception &e)
					{
						return std::nullopt;
					}
				}
				return root.template get<int64_t>();
			}

			if (!isPresent(root, field) || JSONUtils::isNull(root, field))
				return std::nullopt;
			if (root.at(field).type() == nlohmann::json::value_t::string)
			{
				try
				{
					return std::stoll(asString(root, field, "0").c_str());
				}
				catch (std::exception &e)
				{
					return std::nullopt;
				}
			}
			return root.at(field);
		}
		catch (nlohmann::json::out_of_range &e)
		{
			return std::nullopt;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<int64_t> asOptInt64(const J *root, std::string_view field = "", bool exceptionOnMissing = false)
	{
		if (!root)
			return std::nullopt;
		return asOptInt64(*root, field, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static uint64_t asUint64(const J &root, std::string_view field = "", const uint64_t defaultValue = 0, bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			const std::string errorMessage = std::format(
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
				if (root.type() == nlohmann::json::value_t::string)
				{
					try
					{
						return std::stoull(asString(root, "", "0").c_str());
					}
					catch (std::exception &e)
					{
						return defaultValue;
					}
				}
				return root.template get<uint64_t>();
			}

			if (!isPresent(root, field) || JSONUtils::isNull(root, field))
				return defaultValue;
			if (root.at(field).type() == nlohmann::json::value_t::string)
			{
				try
				{
					return std::stoull(asString(root, field, "0").c_str());
				}
				catch (std::exception &e)
				{
					return defaultValue;
				}
			}
			return root.at(field);
		}
		catch (nlohmann::json::out_of_range &e)
		{
			return defaultValue;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static uint64_t asUint64(const J *root, std::string_view field = "", const uint64_t defaultValue = 0, bool exceptionOnMissing = false)
	{
		if (!root)
			return defaultValue;
		return asUint64(*root, field, defaultValue, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<uint64_t> asOptUint64(const J &root, std::string_view field = "", bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			std::string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return std::nullopt;

		try
		{
			if (field.empty())
			{
				if (root.type() == nlohmann::json::value_t::string)
				{
					try
					{
						return std::stoull(asString(root, "", "0").c_str());
					}
					catch (std::exception &e)
					{
						return std::nullopt;
					}
				}
				return root.template get<uint64_t>();
			}

			if (!isPresent(root, field) || JSONUtils::isNull(root, field))
				return std::nullopt;
			if (root.at(field).type() == nlohmann::json::value_t::string)
			{
				try
				{
					return std::stoull(asString(root, field, "0").c_str());
				}
				catch (std::exception &e)
				{
					return std::nullopt;
				}
			}
			return root.at(field);
		}
		catch (nlohmann::json::out_of_range &e)
		{
			return std::nullopt;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<uint64_t> asOptUint64(const J *root, std::string_view field = "", bool exceptionOnMissing = false)
	{
		if (!root)
			return std::nullopt;
		return asOptUint64(*root, field, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static double asDouble(const J &root, std::string_view field = "", const double defaultValue = 0.0, bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			const std::string errorMessage = std::format(
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
				if (root.type() == nlohmann::json::value_t::string)
				{
					try
					{
						return stod(asString(root, "", "0"), nullptr);
					}
					catch (std::exception &e)
					{
						return defaultValue;
					}
				}
				return root.template get<double>();
			}

			if (!isPresent(root, field) || JSONUtils::isNull(root, field))
				return defaultValue;
			if (root.at(field).type() == nlohmann::json::value_t::string)
			{
				try
				{
					return stod(asString(root, field, "0"), nullptr);
				}
				catch (std::exception &e)
				{
					return defaultValue;
				}
			}
			return root.at(field);
		}
		catch (nlohmann::json::out_of_range &e)
		{
			return defaultValue;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static double asDouble(const J *root, std::string_view field = "", const double defaultValue = 0.0, bool exceptionOnMissing = false)
	{
		if (!root)
			return defaultValue;
		return asDouble(*root, field, defaultValue, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<double> asOptDouble(const J &root, std::string_view field = "", bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			const std::string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return std::nullopt;

		try
		{
			if (field.empty())
			{
				if (root.type() == nlohmann::json::value_t::string)
				{
					try
					{
						return stod(asString(root, "", "0"), nullptr);
					}
					catch (std::exception &e)
					{
						return std::nullopt;
					}
				}
				return root.template get<double>();
			}

			if (!isPresent(root, field) || JSONUtils::isNull(root, field))
				return std::nullopt;
			if (root.at(field).type() == nlohmann::json::value_t::string)
			{
				try
				{
					return stod(asString(root, field, "0"), nullptr);
				}
				catch (std::exception &e)
				{
					return std::nullopt;
				}
			}
			return root.at(field);
		}
		catch (nlohmann::json::out_of_range &e)
		{
			return std::nullopt;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<double> asOptDouble(const J *root, std::string_view field = "", bool exceptionOnMissing = false)
	{
		if (!root)
			return std::nullopt;
		return asOptDouble(*root, field, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static bool asBool(const J &root, std::string_view field, const bool defaultValue = false, bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			const std::string errorMessage = std::format(
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
				if (root.type() == nlohmann::json::value_t::string)
				{
					std::string sTrue = "true";

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

			if (!isPresent(root, field) || JSONUtils::isNull(root, field))
				return defaultValue;
			if (root.at(field).type() == nlohmann::json::value_t::string)
			{
				std::string sTrue = "true";

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
		catch (nlohmann::json::out_of_range &e)
		{
			return defaultValue;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static bool asBool(const J *root, std::string_view field, const bool defaultValue = false, bool exceptionOnMissing = false)
	{
		if (!root)
			return defaultValue;
		return asBool(*root, field, defaultValue, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<bool> asOptBool(const J &root, std::string_view field, bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			const std::string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (root == nullptr)
			return std::nullopt;

		try
		{
			if (field.empty())
			{
				if (root.type() == nlohmann::json::value_t::string)
				{
					std::string sTrue = "true";

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

			if (!isPresent(root, field) || JSONUtils::isNull(root, field))
				return std::nullopt;
			if (root.at(field).type() == nlohmann::json::value_t::string)
			{
				std::string sTrue = "true";

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
		catch (nlohmann::json::out_of_range &e)
		{
			return std::nullopt;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<bool> asOptBool(const J *root, std::string_view field, bool exceptionOnMissing = false)
	{
		if (!root)
			return std::nullopt;
		return asOptBool(*root, field, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static J asJson(const J &root, std::string_view field, J defaultValue = J(), const bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			const std::string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (field.empty())
			return root;
		if (root == nullptr)
			return defaultValue;

		if (!isPresent(root, field) || JSONUtils::isNull(root, field))
			return defaultValue;
		return root[field];
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static J asJson(const J *root, std::string_view field, J defaultValue = J(), const bool exceptionOnMissing = false)
	{
		if (!root)
			return defaultValue;
		return asJson(*root, field, defaultValue, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<J> asOptJson(const J &root, std::string_view field, const bool exceptionOnMissing = false)
	{
		if (exceptionOnMissing && !field.empty() && !isPresent(root, field))
		{
			const std::string errorMessage = std::format(
				"Field not found"
				", field: {}",
				field
			);
			SPDLOG_ERROR(errorMessage);

			throw JsonFieldNotFound(errorMessage);
		}

		if (field.empty())
			return root;
		if (root == nullptr)
			return std::nullopt;

		if (!isPresent(root, field) || JSONUtils::isNull(root, field))
			return std::nullopt;
		return root[field];
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::optional<J> asOptJson(const J *root, std::string_view field, const bool exceptionOnMissing = false)
	{
		if (!root)
			return std::nullopt;
		return asOptJson(*root, field, exceptionOnMissing);
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static J toJson(const std::string_view &j, bool warningIfError = false)
	{
		try
		{
			if (j.empty())
				return {};
			return J::parse(j);
		}
		catch (nlohmann::json::parse_error &ex)
		{
			std::string errorMessage = std::format(
				"failed to parse the json"
				", json: '{}'"
				", at byte: {}"
				", exception: {}",
				j, ex.byte, ex.what()
			);
			if (warningIfError)
				SPDLOG_WARN(errorMessage);
			else
				SPDLOG_ERROR(errorMessage);

			throw std::runtime_error(errorMessage);
		}
	}

	template <typename J, typename T>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static J toJson(const std::vector<T> &v)
	{
		J root = J::array();
		for (const T& i : v)
			root.push_back(i);
		return root;
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static J loadConfigurationFile(const std::string_view &configurationPathName, const std::string_view &environmentPrefix = "")
	{

#ifdef BOOTSERVICE_DEBUG_LOG
#ifdef _WIN32
		ofstream of("C:\\bootservice.log", ofstream::app);
#else
		ofstream of("/tmp/bootservice.log", ofstream::app);
#endif
		of << "loadConfigurationFile..." << endl;
#endif

		std::string sConfigurationFile;
		try
		{
			std::ifstream configurationFile(std::string(configurationPathName), std::ifstream::binary);
			std::stringstream buffer;
			buffer << configurationFile.rdbuf();
			if (environmentPrefix.empty())
				sConfigurationFile = buffer.str();
			else
				sConfigurationFile = applyEnvironmentToConfiguration(buffer.str(), environmentPrefix);

#ifdef BOOTSERVICE_DEBUG_LOG
			of << "loadConfigurationFile "
				<< ", sConfigurationFile: " << sConfigurationFile;
#endif

			return J::parse(
				sConfigurationFile,
				nullptr, // callback
				true,	 // allow exceptions
				true	 // ignore_comments
			);
		}
		catch (std::exception &e)
		{
#ifdef BOOTSERVICE_DEBUG_LOG
			of << "loadConfigurationFile failed "
				<< ", configurationPathName: " << configurationPathName
				<< ", sConfigurationFile: " << sConfigurationFile
				<< ", exception: " << e.what() << endl;
#endif
			throw;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static std::string toString(const J &root, int indent = -1)
	{
		try
		{
			if (root == nullptr)
				return "null";
			else
				return root.dump(indent, ' ', true);
		}
		catch (const nlohmann::json::type_error &e)
		{
			throw std::runtime_error(e.what());
		}
	}

	static std::string json5ToJson(const std::string &json5);
	static std::string applyEnvironmentToConfiguration(std::string configuration, const std::string_view &environmentPrefix);

  private:
	static std::string json5_removeComments(const std::string &input);
	static std::string json5_removeTrailingCommas(const std::string &input);
	static std::string json5_quoteUnquotedKeys(const std::string &input);
};
