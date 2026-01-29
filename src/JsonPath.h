#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <cstddef>   // size_t
#include <format>

#include "JSONUtils.h"

template <typename J>
requires (std::is_same_v<J, nlohmann::json> || std::is_same_v<J, nlohmann::ordered_json>)
class JsonPath
{
public:
    enum class AccessMode
    {
        Required,
        Optional
    };

    explicit JsonPath(const J* j = nullptr, const AccessMode mode = AccessMode::Optional)
        : _root(j), _mode(mode)
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
			return jsonPathMissing(nextPath);

		if (!JSONUtils::isPresent(*_root, key))
			return jsonPathMissing(nextPath);

		auto it = _root->find(std::string(key));
		if (it == _root->end())
			return jsonPathMissing(nextPath);

		return JsonPath(&(*it), _mode, nextPath);
	}

	[[nodiscard]] JsonPath operator[](std::size_t index) const
    {
        std::string nextPath = _path.empty()
            ? std::format("[{}]", index)
            : std::format("{}[{}]", _path, index);

        if (!_root || !_root->is_array() || index >= _root->size())
            return jsonPathMissing(nextPath);

        return JsonPath(&((*_root)[index]), _mode, nextPath);
    }

    [[nodiscard]] bool exists() const noexcept
    {
        return _root != nullptr;
    }

	[[nodiscard]] bool empty() const
    {
    	if (!_root)
    		return true; // se il nodo non esiste è vuoto

    	if (_root->is_null())
    		return true;

    	if (_root->is_object())
    		return _root->empty();

    	if (_root->is_array())
    		return _root->empty();

    	return false;
    }

	template <typename T>
    [[nodiscard]] T as(T defaultValue = {}, std::span<const T> allowedValues = {}) const
    {
        if (!_root)
        {
            if (_mode == AccessMode::Required)
                throw JsonFieldNotFound(std::format("Missing required JSON field: {}", _path));
            return defaultValue;
        }
    	try
    	{
    		return JSONUtils::as<T>(*_root, "", defaultValue, allowedValues, false);
    	}
    	catch (const std::exception &e)
    	{
    		const std::string errorMessage = std::format("Error accessing JSON field '{}': {}", _path, e.what());
    		LOG_ERROR(errorMessage);
    		throw std::runtime_error(errorMessage);
    	}
    }

	template <typename T>
    [[nodiscard]] std::optional<T> asOpt(std::span<const T> allowedValues = {}) const
    {
        if (!_root) {
            if (_mode == AccessMode::Required)
                throw JsonFieldNotFound(std::format("Missing required JSON field: {}", _path));
            return std::nullopt;
        }
    	try
    	{
    		return JSONUtils::asOpt<T>(*_root, "", allowedValues, false);
    	}
    	catch (const std::exception &e)
    	{
    		const std::string errorMessage = std::format("Error accessing JSON field '{}': {}", _path, e.what());
    		LOG_ERROR(errorMessage);
    		throw std::runtime_error(errorMessage);
    	}
    }

    [[nodiscard]] const J* get() const noexcept { return _root; }
    [[nodiscard]] std::string path() const noexcept { return _path; }

private:
    const J* _root;
    AccessMode _mode;
    std::string _path;

	explicit JsonPath(const J* j, const AccessMode mode, std::string path)
		: _root(j), _mode(mode), _path(std::move(path))
	{}

    [[nodiscard]] JsonPath jsonPathMissing(const std::string& nextPath) const
    {
        if (_mode == AccessMode::Required)
            throw JsonFieldNotFound(std::format("Missing required JSON field: {}", nextPath));
        return JsonPath(nullptr, _mode, nextPath);
    }
};
