#ifndef TOJSON_HPP_
#define TOJSON_HPP_

#include <exception>
#include <fstream>
#include <iostream>

#include <yaml-cpp/yaml.h>
#include <Nlohmann/json.hpp>
using json = nlohmann::json;

#if __has_cpp_attribute(nodiscard)
	#define TOJSON_NODISCARD [[nodiscard]]
#else
	#define TOJSON_NODISCARD
#endif

namespace tojson {

class ParseError : public std::exception
{
	virtual const char* what() const throw()
	{
		return "parse error";
	}
};

namespace detail {

inline json yaml2json(YAML::Node root)
{
	json j{};

	switch (root.Type()) {
	case YAML::NodeType::Null:
		break;
	case YAML::NodeType::Scalar:
		return root.as<std::string>();
	case YAML::NodeType::Sequence:
		for (auto node : root)
			j.emplace_back(yaml2json(node));
		break;
	case YAML::NodeType::Map:
		for (const auto &it : root) {
			YAML::Node key   = it.first;
			YAML::Node value = it.second;
			j[key.as<std::string>()] = yaml2json(value);
		}
		break;
	default:
		break;
	}
	return j;
}

}


/// \brief Convert YAML string to JSON.
TOJSON_NODISCARD inline json yaml2json(const std::string &str)
{
	YAML::Node root;

	try {
		root = YAML::Load(str);
	} catch (const YAML::ParserException &e) {
		throw ParseError{};
	} catch (const std::exception& e) {
		return json{};
	}

	return detail::yaml2json(root);
}

/// \brief Load a YAML file to JSON.
TOJSON_NODISCARD inline json loadyaml(const std::string &filepath)
{
	YAML::Node root;

	try {
		root = YAML::LoadFile(filepath);
	} catch (const YAML::ParserException &e) {
		throw ParseError{};
	} catch (const std::exception& e) {
		return json{};
	}

	return detail::yaml2json(root);
}

}

#endif /* TOJSON_HPP_ */
