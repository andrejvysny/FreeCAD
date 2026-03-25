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

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "TokenManager.h"

namespace FcComponents::Tokens
{

enum class Operator : std::uint8_t
{
    Add,
    Subtract,
    Multiply,
    Divide
};

struct EvaluationContext
{
    const TokenManager* manager {};
    TokenManager::ResolveContext context;
};

// Abstract Syntax Tree (AST) Base
struct FCComponentLibExport Expr
{
    Expr() = default;

    Expr(Expr&&) = default;
    Expr& operator=(Expr&&) = default;
    Expr(const Expr&) = delete;
    Expr& operator=(const Expr&) = delete;

    virtual Value evaluate(const EvaluationContext& context) const = 0;
    virtual ~Expr() = default;
};

struct FCComponentLibExport ParameterReference: public Expr
{
    std::string name;

    explicit ParameterReference(std::string name)
        : name(std::move(name))
    {}

    Value evaluate(const EvaluationContext& context) const override;
};

struct FCComponentLibExport Number: public Expr
{
    Numeric value;

    Number(double value, std::string unit)
        : value({value, std::move(unit)})
    {}

    Value evaluate([[maybe_unused]] const EvaluationContext& context) const override;
};

struct FCComponentLibExport Color: public Expr
{
    Base::Color color;

    explicit Color(Base::Color color)
        : color(std::move(color))
    {}

    Value evaluate([[maybe_unused]] const EvaluationContext& context) const override;
};

struct FCComponentLibExport TupleLiteral: public Expr
{
    struct Element
    {
        std::optional<std::string> name;
        std::unique_ptr<Expr> expression;
    };

    std::vector<Element> elements;

    Value evaluate(const EvaluationContext& context) const override;
};

struct FCComponentLibExport FunctionCall: public Expr
{
    std::string functionName;
    TupleLiteral arguments;

    FunctionCall(std::string functionName, TupleLiteral arguments)
        : functionName(std::move(functionName))
        , arguments(std::move(arguments))
    {}

    Value evaluate(const EvaluationContext& context) const override;
};

struct FCComponentLibExport BinaryOp: public Expr
{
    std::unique_ptr<Expr> left, right;
    Operator op;

    BinaryOp(std::unique_ptr<Expr> left, Operator op, std::unique_ptr<Expr> right)
        : left(std::move(left))
        , right(std::move(right))
        , op(op)
    {}

    Value evaluate(const EvaluationContext& context) const override;
};

struct FCComponentLibExport UnaryOp: public Expr
{
    Operator op;
    std::unique_ptr<Expr> operand;

    UnaryOp(Operator op, std::unique_ptr<Expr> operand)
        : op(op)
        , operand(std::move(operand))
    {}

    Value evaluate(const EvaluationContext& context) const override;
};

struct FCComponentLibExport MemberAccess: public Expr
{
    std::unique_ptr<Expr> object;
    std::string member;

    MemberAccess(std::unique_ptr<Expr> object, std::string member)
        : object(std::move(object))
        , member(std::move(member))
    {}

    Value evaluate(const EvaluationContext& context) const override;
};

class FCComponentLibExport Parser
{
    static constexpr auto rgbFunction = "rgb(";
    static constexpr auto rgbaFunction = "rgba(";

    std::string input;
    size_t pos = 0;

public:
    explicit Parser(std::string input)
        : input(std::move(input))
    {}

    std::unique_ptr<Expr> parse();

private:
    bool peekString(const char* function) const;
    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseTerm();
    std::unique_ptr<Expr> parseFactor();
    bool peekColor();
    std::unique_ptr<Expr> parseColor();
    bool peekParameter();
    std::unique_ptr<Expr> parseParameter();
    bool peekFunction();
    std::unique_ptr<Expr> parseFunctionCall();
    int parseInt();
    std::unique_ptr<Expr> parseNumber();
    std::string parseUnit();
    std::string parseMember();
    bool peekNamedElement();
    std::unique_ptr<TupleLiteral> parseTuple(
        std::optional<TupleLiteral::Element> firstElement = std::nullopt
    );
    bool match(char expected);
    void skipWhitespace();
};

}  // namespace FcComponents::Tokens
