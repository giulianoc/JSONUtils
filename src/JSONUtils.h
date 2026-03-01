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

#include "ThreadLogger.h"
#include "nlohmann/json.hpp"
#include <format>
#include <fstream>
#include <iostream>
#include <spdlog/fmt/bundled/ranges.h>

struct JsonFieldNotFound final : std::exception
{
	std::string _errorMessage;

	explicit JsonFieldNotFound(const std::string_view errorMessage):_errorMessage(errorMessage){};

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
	static T as(const J& root, std::string_view field, T defaultVal, std::initializer_list<T> allowedValues,
		const bool exceptionOnError = false)
	{
		return as<T>(root, field, std::move(defaultVal),
			std::span<const T>(allowedValues.begin(), allowedValues.size()), exceptionOnError);
	}

	template <typename T, typename J>
	static T as(const J& root, std::string_view field = {}, T defaultVal = {}, std::span<const T> allowedValues = {},
		const bool exceptionOnError = false)
	{
		try
		{
			if (root == nullptr)
			{
				const std::string errorMessage = std::format("Received a json nullptr"
					", field: {}", field);
				if (exceptionOnError)
				{
					LOG_ERROR(errorMessage);
					throw std::invalid_argument(errorMessage);
				}
				LOG_TRACE(errorMessage);
				return defaultVal;
			}
			if (field.empty())
			{
				// T value = root.template get<T>();
				T value = getJsonValue<T>(root);

				if (!allowedValues.empty())
				{
					if (std::ranges::find(allowedValues, value) != allowedValues.end())
						return value;

					std::string errorMessage;
					if constexpr (
						std::is_same_v<T, nlohmann::json> ||
						std::is_same_v<T, nlohmann::ordered_json>
					)
					{
						std::vector<std::string> tmp;
						tmp.reserve(allowedValues.size());
						for (const auto& v : allowedValues)
							tmp.push_back(JSONUtils::toString(v));
						std::string allowedValuesStr = fmt::format("{}", fmt::join(tmp, ", "));
						errorMessage = fmt::format(
						"Invalid value '{}' for '{}'. Allowed values are: {}",
							JSONUtils::toString(value), field, allowedValuesStr
						);
					}
					else
						errorMessage = fmt::format("Invalid value '{}' for '{}'. Allowed values are: {}",
						value, field, fmt::join(allowedValues, ", ")
						);
					if (exceptionOnError)
					{
						LOG_ERROR(errorMessage);
						throw std::invalid_argument(errorMessage);
					}
					LOG_TRACE(errorMessage);
					return defaultVal;
				}
				return value;
			}
			if (!JSONUtils::isPresent(root, field))
			{
				const std::string errorMessage = std::format("Field [{}] not found", field);
				if (exceptionOnError)
				{
					LOG_ERROR(errorMessage);
					throw JsonFieldNotFound(errorMessage);
				}
				LOG_TRACE(errorMessage);
				return defaultVal;
			}
			{
				// T value = root.at(field).template get<T>();
				T value = getJsonValue<T>(root.at(field));

				if (!allowedValues.empty())
				{
					if (std::ranges::find(allowedValues, value) != allowedValues.end())
						return value;

					std::string errorMessage;
					if constexpr (
						std::is_same_v<T, nlohmann::json> ||
						std::is_same_v<T, nlohmann::ordered_json>
					)
					{
						std::vector<std::string> tmp;
						tmp.reserve(allowedValues.size());
						for (const auto& v : allowedValues)
							tmp.push_back(JSONUtils::toString(v));
						std::string allowedValuesStr = fmt::format("{}", fmt::join(tmp, ", "));
						errorMessage = fmt::format(
						"Invalid value '{}' for '{}'. Allowed values are: {}",
							JSONUtils::toString(value), field, allowedValuesStr
						);
					}
					else
						errorMessage = fmt::format("Invalid value '{}' for '{}'. Allowed values are: {}",
						value, field, fmt::join(allowedValues, ", ")
						);
					if (exceptionOnError)
					{
						LOG_ERROR(errorMessage);
						throw std::invalid_argument(errorMessage);
					}
					LOG_TRACE(errorMessage);
					return defaultVal;
				}
				return value;
			}
		}
		catch (const std::exception& e)
		{
			// abbiamo una eccezione se ad es. chiediamo una stringa (as<string>) ma il valore è un numero
			const std::string errorMessage = field.empty() ?
				std::format("json: {}, exception: {}", JSONUtils::toString(root), e.what()) :
				std::format("json: {}, field: {}, exception: {}", JSONUtils::toString(root), field, e.what());
			if (exceptionOnError)
			{
				LOG_ERROR(errorMessage);
				throw;
			}
			LOG_TRACE(errorMessage);
			return defaultVal;
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
	static std::optional<T> asOpt(const J& root, std::string_view field, std::initializer_list<T> allowedValues,
		const bool exceptionOnError = false)
	{
		return asOpt<T>(root, field, std::span<const T>(allowedValues.begin(), allowedValues.size()),
			exceptionOnError);
	}

	template <typename T, typename J>
	static std::optional<T> asOpt(const J& root, std::string_view field = {}, std::span<const T> allowedValues = {},
		const bool exceptionOnError = false)
	{
		try
		{
			if (root == nullptr)
			{
				const std::string errorMessage = "Received a json nullptr";
				if (exceptionOnError)
				{
					LOG_ERROR(errorMessage);
					throw std::invalid_argument(errorMessage);
				}
				LOG_TRACE(errorMessage);
				return std::nullopt;
			}
			if (field.empty())
			{
				// T value = root.template get<T>();
				T value = getJsonValue<T>(root);

				if (!allowedValues.empty())
				{
					if (std::ranges::find(allowedValues, value) != allowedValues.end())
						return value;

					std::string errorMessage;
					if constexpr (
						std::is_same_v<T, nlohmann::json> ||
						std::is_same_v<T, nlohmann::ordered_json>
					)
					{
						std::vector<std::string> tmp;
						tmp.reserve(allowedValues.size());
						for (const auto& v : allowedValues)
							tmp.push_back(JSONUtils::toString(v));
						std::string allowedValuesStr = fmt::format("{}", fmt::join(tmp, ", "));
						errorMessage = fmt::format(
						"Invalid value '{}' for '{}'. Allowed values are: {}",
							JSONUtils::toString(value), field, allowedValuesStr
						);
					}
					else
						errorMessage = fmt::format("Invalid value '{}' for '{}'. Allowed values are: {}",
						value, field, fmt::join(allowedValues, ", ")
						);
					if (exceptionOnError)
					{
						LOG_ERROR(errorMessage);
						throw std::invalid_argument(errorMessage);
					}
					LOG_TRACE(errorMessage);
					return std::nullopt;
				}
				return value;
			}
			if (!JSONUtils::isPresent(root, field))
				return std::nullopt;
			{
				// T value = root.at(field).template get<T>();
				T value = getJsonValue<T>(root.at(field));

				if (!allowedValues.empty())
				{
					if (std::ranges::find(allowedValues, value) != allowedValues.end())
						return value;

					std::string errorMessage;
					if constexpr (
						std::is_same_v<T, nlohmann::json> ||
						std::is_same_v<T, nlohmann::ordered_json>
					)
					{
						std::vector<std::string> tmp;
						tmp.reserve(allowedValues.size());
						for (const auto& v : allowedValues)
							tmp.push_back(JSONUtils::toString(v));
						std::string allowedValuesStr = fmt::format("{}", fmt::join(tmp, ", "));
						errorMessage = fmt::format(
						"Invalid value '{}' for '{}'. Allowed values are: {}",
							JSONUtils::toString(value), field, allowedValuesStr
						);
					}
					else
						errorMessage = fmt::format("Invalid value '{}' for '{}'. Allowed values are: {}",
						value, field, fmt::join(allowedValues, ", ")
						);
					if (exceptionOnError)
					{
						LOG_ERROR(errorMessage);
						throw std::invalid_argument(errorMessage);
					}
					LOG_TRACE(errorMessage);
					return std::nullopt;
				}
				return value;
			}
		}
		catch (const std::exception& e)
		{
			// abbiamo una eccezione se ad es. chiediamo una stringa (as<string>) ma il valore è un numero
			const std::string errorMessage = field.empty() ?
				std::format("Json: {}, exception: {}", JSONUtils::toString(root), e.what()) :
				std::format("Field: {}, exception: {}", field, e.what());
			if (exceptionOnError)
			{
				LOG_ERROR(errorMessage);
				throw;
			}
			LOG_TRACE(errorMessage);
			return std::nullopt;
		}
	}

	template<typename T>
	static T getJsonValue(const nlohmann::json& fieldRoot)
	{
		if constexpr (std::is_same_v<T, std::string>)
		{
			if (fieldRoot.is_string())
				return fieldRoot.get<std::string>();
			if (fieldRoot.is_number())
				return fieldRoot.dump();   // converte 15.876 -> "15.876"
			if (fieldRoot.is_boolean())
				return fieldRoot.get<bool>() ? "true" : "false";

			const std::string errorMessage = std::format("getJsonValue failed"
				", fieldRoot: {}", toString(fieldRoot)
				);
			LOG_ERROR(errorMessage);
			throw std::invalid_argument(errorMessage);
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			if (fieldRoot.is_boolean())
				return fieldRoot.get<bool>();
			if (fieldRoot.is_number())
				return fieldRoot.get<double>() != 0.0;
			if (fieldRoot.is_string())
			{
				const auto& s = fieldRoot.get_ref<const std::string&>();
				if (s == "true" || s == "1") return true;
				if (s == "false" || s == "0") return false;
			}

			const std::string errorMessage = std::format("getJsonValue failed"
				", fieldRoot: {}", toString(fieldRoot)
				);
			LOG_ERROR(errorMessage);
			throw std::invalid_argument(errorMessage);
		}
		else if constexpr (std::is_arithmetic_v<T>) // NUMERIC TYPES REQUESTED
		{
			if (fieldRoot.is_number())
				return fieldRoot.get<T>();
			if (fieldRoot.is_string())
			{
				const auto& s = fieldRoot.get_ref<const std::string&>();
				T value{};
				auto [ptr, ec] = std::from_chars(
					s.data(),
					s.data() + s.size(),
					value
				);
				if (ec == std::errc() && ptr == s.data() + s.size())
					return value;
			}

			const std::string errorMessage = std::format("getJsonValue failed"
				", fieldRoot: {}", toString(fieldRoot)
				);
			LOG_ERROR(errorMessage);
			throw std::invalid_argument(errorMessage);
		}
		else
			return fieldRoot.get<T>();
	}

	template<typename J, typename N>
		requires requires(J) { (std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>); } &&
				 (std::is_integral_v<N> || std::is_floating_point_v<N>)
	static void setOrAdd(J &obj, const std::string_view &key, N value) {
		if (obj.contains(key) && obj[key].is_number()) {
			obj[key] = obj[key].template get<N>() + value;
		} else {
			obj[key] = value;
		}
	}

	template<typename J, typename V>
		requires (std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>) &&
				 (std::is_same_v<V, nlohmann::json> || std::is_same_v<V, nlohmann::ordered_json>)
	static void setOrAdd(J &obj, std::string_view key, const V &values) {
		if (!obj.contains(key)) {
			obj[key] = values;
			return;
		}

		if (values.is_array()) {
			if (obj[key].is_array()) {
				V temp = obj[key];
				for (const auto &val: values) {
					temp.push_back(val);
				}
				obj[key] = std::move(temp);
			} else {
				obj[key] = values;
			}
		} else if (values.is_object()) {
			if (obj[key].is_object()) {
				for (auto it = values.begin(); it != values.end(); ++it) {
					obj[key][it.key()] = it.value();
				}
			} else {
				obj[key] = values;
			}
		} else {
			obj[key] = values;
		}
	}

	template <typename J>
	requires std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>
	static J toJson(const std::string_view &j, const bool warningIfError = false)
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
				LOG_WARN(errorMessage);
			else
				LOG_ERROR(errorMessage);

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
