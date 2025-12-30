#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <cstddef>   // size_t
#include <format>

#include "JSONUtils.h"

/* Esempi di uso:

// simile a javascript
auto first = jpath(root)["items"][0]["name"].as<string>();

// Config safe con default
_apiProtocol = jpath(configurationRoot)
		.optional()["mms"]["api"]["protocol"]
		.as<string>("https");

// Campo obbligatorio
int64_t port = jpath(configurationRoot)
		.required()["server"]["port"]
		.as<int64_t>();

// Optional typed
auto avgBandwidth = jpath(root)
		.optional()["stats"]["avgBandwidth"]
		.as<uint64_t>();

// check presenza
if (jpath(root)["mms"]["api"].exists())
	...
*/

template <typename J>
requires (std::is_same_v<J, json> || std::is_same_v<J, ordered_json>)
class JsonPath
{
public:
    enum class AccessMode
    {
        Required,
        Optional
    };

    explicit JsonPath(const J* j = nullptr, const AccessMode mode = AccessMode::Optional, std::string path = {})
        : _root(j), _mode(mode), _path(std::move(path))
    {}

    [[nodiscard]] JsonPath required() const
    {
        return JsonPath(_root, AccessMode::Required, _path);
    }

    [[nodiscard]] JsonPath optional() const
    {
        return JsonPath(_root, AccessMode::Optional, _path);
    }

	[[nodiscard]] JsonPath operator[](const std::string& key) const
	{
		// Path tipo "a.b.c"
		std::string nextPath = _path.empty()
			? std::string(key)
			: std::format("{}.{}", _path, key);

		if (!_root || !_root->is_object())
			return missing(nextPath);

		if (!JSONUtils::isPresent(*_root, key))
			return missing(nextPath);

		auto it = _root->find(std::string(key));
		if (it == _root->end())
			return missing(nextPath);

		return JsonPath(&(*it), _mode, nextPath);
	}

	[[nodiscard]] JsonPath operator[](std::size_t index) const
    {
        std::string nextPath = _path.empty()
            ? std::format("[{}]", index)
            : std::format("{}[{}]", _path, index);

        if (!_root || !_root->is_array() || index >= _root->size())
            return missing(nextPath);

        return JsonPath(&((*_root)[index]), _mode, nextPath);
    }

    [[nodiscard]] bool exists() const noexcept
    {
        return _root != nullptr;
    }

	template <typename T>
    [[nodiscard]] T as(T defaultValue, const bool exceptionOnMissing = false) const
    {
        if (!_root)
        {
            if (_mode == AccessMode::Required || exceptionOnMissing)
                throw JsonFieldNotFound(missingMessage());
            return defaultValue;
        }
    	try
    	{
    		return JSONUtils::as<T>(*_root, "", defaultValue, false);
    	}
    	catch (const std::exception &e)
    	{
    		const string errorMessage = std::format("Error accessing JSON field '{}': {}", _path, e.what());
    		SPDLOG_ERROR(errorMessage);
    		throw std::runtime_error(errorMessage);
    	}
    }

	template <typename T>
    [[nodiscard]] std::optional<T> asOpt() const
    {
        if (!_root) {
            if (_mode == AccessMode::Required)
                throw JsonFieldNotFound(missingMessage());
            return std::nullopt;
        }
    	try
    	{
    		return JSONUtils::asOpt<T>(*_root, "", false);
    	}
    	catch (const std::exception &e)
    	{
    		const string errorMessage = std::format("Error accessing JSON field '{}': {}", _path, e.what());
    		SPDLOG_ERROR(errorMessage);
    		throw std::runtime_error(errorMessage);
    	}
    }

    [[nodiscard]] const J* get() const noexcept { return _root; }
    [[nodiscard]] std::string path() const noexcept { return _path; }

private:
    const J* _root;
    AccessMode _mode;
    std::string _path;

    [[nodiscard]] std::string missingMessage() const
    {
        return std::format("Missing required JSON field: {}", _path);
    }

    [[nodiscard]] JsonPath missing(const std::string& nextPath) const
    {
        if (_mode == AccessMode::Required)
            throw JsonFieldNotFound(std::format("Missing required JSON field: {}", nextPath));
        return JsonPath(nullptr, _mode, nextPath);
    }
};
