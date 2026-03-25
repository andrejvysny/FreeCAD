// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2025 Kacper Donat <kacper@kadet.net>                     *
 *   Copyright (c) 2025 FreeCAD Project Association                         *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/

#include "TokenManager.h"
#include "Parser.h"

#include <QFile>
#include <fstream>
#include <yaml-cpp/yaml.h>

#include <QRegularExpression>
#include <QString>
#include <ranges>
#include <utility>
#include <variant>

#include <QtDebug>

namespace FcComponents::Tokens
{

ParameterSource::ParameterSource(const Metadata& metadata)
    : metadata(metadata)
{}

InMemoryParameterSource::InMemoryParameterSource(
    const std::list<Parameter>& parameters,
    const Metadata& metadata
)
    : ParameterSource(metadata)
{
    for (const auto& parameter : parameters) {
        InMemoryParameterSource::define(parameter);
    }
}

std::list<Parameter> InMemoryParameterSource::all() const
{
    auto values = parameters | std::ranges::views::values;

    return std::list<Parameter>(values.begin(), values.end());
}

std::optional<Parameter> InMemoryParameterSource::get(const std::string& name) const
{
    if (parameters.contains(name)) {
        return parameters.at(name);
    }

    return std::nullopt;
}

void InMemoryParameterSource::define(const Parameter& parameter)
{
    parameters[parameter.name] = parameter;
}

void InMemoryParameterSource::remove(const std::string& name)
{
    parameters.erase(name);
}

YamlParameterSource::YamlParameterSource(const std::string& filePath, const Metadata& metadata)
    : ParameterSource(metadata)
{
    changeFilePath(filePath);
}

void YamlParameterSource::changeFilePath(const std::string& path)
{
    this->filePath = path;
    reload();
}

void YamlParameterSource::reload()
{
    QFile file(QString::fromStdString(filePath));

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug("FCComponentLib: Unable to open file %s", filePath.c_str());
        return;
    }

    if (filePath.starts_with(":/")) {
        this->metadata.options |= ReadOnly;
    }

    QTextStream in(&file);
    std::string content = in.readAll().toStdString();

    YAML::Node root = YAML::Load(content);
    parameters.clear();
    for (auto it = root.begin(); it != root.end(); ++it) {
        auto key = it->first.as<std::string>();
        auto value = it->second.as<std::string>();

        parameters[key] = Parameter {
            .name = key,
            .value = value,
        };
    }
}

std::list<Parameter> YamlParameterSource::all() const
{
    std::list<Parameter> result;
    for (const auto& param : parameters | std::views::values) {
        result.push_back(param);
    }
    return result;
}

std::optional<Parameter> YamlParameterSource::get(const std::string& name) const
{
    if (auto it = parameters.find(name); it != parameters.end()) {
        return it->second;
    }

    return std::nullopt;
}

void YamlParameterSource::define(const Parameter& param)
{
    parameters[param.name] = param;
}

void YamlParameterSource::remove(const std::string& name)
{
    parameters.erase(name);
}

void YamlParameterSource::flush()
{
    YAML::Node root;
    for (const auto& [name, param] : parameters) {
        root[name] = param.value;
    }

    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning("FCComponentLib: Unable to open file %s for writing", filePath.c_str());
        return;
    }

    QTextStream out(&file);
    out << QString::fromStdString(YAML::Dump(root));
}

TokenManager::TokenManager() = default;

void TokenManager::reload()
{
    _resolved.clear();
}

std::string TokenManager::replacePlaceholders(
    const std::string& expression,
    ResolveContext context
) const
{
    static const QRegularExpression regex(QStringLiteral("@(\\w+)"));

    auto substituteWithCallback =
        [](const QRegularExpression& regex,
           const QString& input,
           const std::function<QString(const QRegularExpressionMatch&)>& callback) {
            QRegularExpressionMatchIterator it = regex.globalMatch(input);

            QString result;
            qsizetype lastIndex = 0;

            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();

                qsizetype start = match.capturedStart();
                qsizetype end = match.capturedEnd();

                result += input.mid(lastIndex, start - lastIndex);
                result += callback(match);

                lastIndex = end;
            }

            // Append any remaining text after the last match
            result += input.mid(lastIndex);

            return result;
        };

    // clang-format off
    return substituteWithCallback(
        regex,
        QString::fromStdString(expression),
        [&](const QRegularExpressionMatch& match) {
            auto tokenName = match.captured(1).toStdString();
            auto tokenValue = resolve(tokenName, context);

            if (!tokenValue) {
                qWarning("Requested non-existent style parameter token '%s'.",
                         tokenName.c_str());
                return QStringLiteral("");
            }

            context.visited.erase(tokenName);
            return QString::fromStdString(tokenValue->toString());
    }
    ).toStdString();
    // clang-format on
}

std::list<Parameter> TokenManager::parameters() const
{
    std::set<Parameter, Parameter::NameComparator> result;

    // we need to traverse it in reverse order so more important tokens will take precedence
    for (const ParameterSource* source : _sources | std::views::reverse) {
        for (const Parameter& parameter : source->all()) {
            result.insert(parameter);
        }
    }

    return std::list(result.begin(), result.end());
}

std::optional<std::string> TokenManager::expression(const std::string& name) const
{
    if (auto param = parameter(name)) {
        return param->value;
    }

    return {};
}

std::optional<Value> TokenManager::resolve(const std::string& name, ResolveContext context) const
{
    std::optional<Parameter> maybeParameter = this->parameter(name);

    if (!maybeParameter) {
        return std::nullopt;
    }

    if (context.visited.contains(name)) {
        qWarning("The style parameter '%s' contains circular-reference.", name.c_str());
        return expression(name);
    }

    const Parameter& token = *maybeParameter;

    if (!_resolved.contains(token.name)) {
        context.visited.insert(token.name);
        try {
            _resolved[token.name] = evaluate(token.value, context);
        }
        catch (Base::Exception&) {
            // in case of being unable to parse it, we need to treat it as a generic value
            _resolved[token.name] = replacePlaceholders(token.value, context);
        }
        context.visited.erase(token.name);
    }

    return _resolved[token.name];
}

Value TokenManager::evaluate(const std::string& expression, ResolveContext context) const
{
    Parser parser(expression);
    return parser.parse()->evaluate({.manager = this, .context = std::move(context)});
}

std::optional<Parameter> TokenManager::parameter(const std::string& name) const
{
    for (const ParameterSource* source : _sources) {
        if (const auto& parameter = source->get(name)) {
            return parameter;
        }
    }

    return {};
}

void TokenManager::addSource(ParameterSource* source)
{
    _sources.push_front(source);
}

std::list<ParameterSource*> TokenManager::sources() const
{
    return _sources;
}

}  // namespace FcComponents::Tokens
