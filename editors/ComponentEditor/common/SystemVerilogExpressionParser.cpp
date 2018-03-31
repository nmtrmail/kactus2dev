//-----------------------------------------------------------------------------
// File: SystemVerilogExpressionParser.cpp
//-----------------------------------------------------------------------------
// Project: Kactus2
// Author: Esko Pekkarinen
// Date: 25.11.2014
//
// Description:
// Parser for SystemVerilog expressions.
//-----------------------------------------------------------------------------

#include "SystemVerilogExpressionParser.h"
#include "SystemVerilogSyntax.h"

#include <QRegularExpression>
#include <QStringList>
#include <qmath.h>

#include <QMap>
#include <QDebug>

#include <algorithm>

namespace
{
    const QRegularExpression PRIMARY_LITERAL("(" + SystemVerilogSyntax::REAL_NUMBER + "|" +
        SystemVerilogSyntax::BOOLEAN_VALUE + "|" +
        SystemVerilogSyntax::INTEGRAL_NUMBER + ")");

    const QRegularExpression BINARY_OPERATOR("[/*][/*]|[-+/*//]|[<>]=?|[!=]=|[$]pow");

    const QRegularExpression UNARY_OPERATOR("[$]clog2|[$]exp|[$]sqrt");

    const QChar OPEN_PARENTHESIS('(');
    const QChar CLOSE_PARENTHESIS(')');
    const QChar OPEN_ARRAY('{');
    const QChar CLOSE_ARRAY('}');

    const QString OPEN_PARENTHESIS_STRING("(");
    const QString OPEN_ARRAY_STRING("{");
    const QString CLOSE_ARRAY_STRING("}");

    const QRegularExpression ANY_OPERATOR(BINARY_OPERATOR.pattern() + "|" + UNARY_OPERATOR.pattern());
}

QMap<QString, int> SystemVerilogExpressionParser::operator_precedence =
{
    {"<", 1},
    {">", 1},
    {"<=", 1},
    {">=", 1},
    {"==", 1},
    {"+", 2},
    {"-", 2},
    {"*", 3},
    {"/", 3},
    {"**", 4},
    {"$clog2", 5},
    {"$pow", 5},
    {"(", 6},
    {")", 6},
    {"{", 6},
    {"}", 6}
};

QMap<QString, int> SystemVerilogExpressionParser::base_formats =
{
    {"", 10},
    {"d", 10},
    {"D", 10},
    {"h", 16},
    {"H", 16},
    {"o", 8},
    {"O", 8},
    {"b", 2},
    {"B", 2},
};

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::SystemVerilogExpressionParser()
//-----------------------------------------------------------------------------
SystemVerilogExpressionParser::SystemVerilogExpressionParser()
{

}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::parseExpression()
//-----------------------------------------------------------------------------
QString SystemVerilogExpressionParser::parseExpression(QString const& expression, bool* validExpression) const
{
    return solveRPN(convertToRPN(expression), validExpression);
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::convertToRPN()
//-----------------------------------------------------------------------------
QStringList SystemVerilogExpressionParser::convertToRPN(QString const& expression) const
{
    // Convert expression to Reverse Polish Notation (RPN) using the Shunting Yard algorithm.
    QStringList output;
    QStringList stack;

    int openParenthesis = 0;
    bool lastWasOperator = true;
    for (int index = 0; index < expression.size(); /*index incremented inside loop*/)
    {
        if (expression.at(index).isSpace())
        {
            index++;
            continue;
        }

        QRegularExpressionMatch operatorMatch = ANY_OPERATOR.match(expression, index,
            QRegularExpression::NormalMatch, QRegularExpression::AnchoredMatchOption);
        QRegularExpressionMatch literalMatch = PRIMARY_LITERAL.match(expression, index,
            QRegularExpression::NormalMatch, QRegularExpression::AnchoredMatchOption);

        if (lastWasOperator && literalMatch.hasMatch())
        {
            output.append(literalMatch.captured());
            
            index = literalMatch.capturedEnd();
            lastWasOperator = false;
        }
        else if (operatorMatch.hasMatch())
        {
            while (stack.size() > 0 and
                operator_precedence.value(stack.last()) >= operator_precedence.value(operatorMatch.captured()) and
                stack.last() != OPEN_PARENTHESIS_STRING)
            {
                output.append(stack.takeLast());
            }

            stack.append(operatorMatch.captured());

            index = operatorMatch.capturedEnd();
            lastWasOperator = true;
        }
        else if (expression.at(index) == OPEN_ARRAY or expression.at(index) == OPEN_PARENTHESIS)
        {
            if (expression.at(index) == OPEN_ARRAY)
            {
                output.append(expression.at(index));
            }
            else
            {
                stack.append(expression.at(index));
            }

            index++;
            openParenthesis++;
            lastWasOperator = true;
        }
        else if (expression.at(index) == CLOSE_ARRAY or expression.at(index) == CLOSE_PARENTHESIS)
        {
            while (stack.size() > 0 and (stack.last() != expression.at(index).mirroredChar()))
            {
                output.append(stack.takeLast());
            }

            bool isArray = expression.at(index) == CLOSE_ARRAY;
            if (stack.size() != 0)
            {
                QString last(stack.takeLast());
                if (isArray)
                {
                    output.append(last);
                }
            }

            if (isArray)
            {
                stack.append(expression.at(index));
            }

            index++;
            openParenthesis--;
            lastWasOperator = isArray;
        }
        else if (expression.at(index) == ',')
        {
            while (stack.size() > 0 && (stack.last() != OPEN_ARRAY && stack.last() != OPEN_PARENTHESIS))
            {
                output.append(stack.takeLast());
            }

            index++;
            lastWasOperator = true;
        }
        else
        {
            QRegularExpression separator(ANY_OPERATOR.pattern() + "|[(){}]");
            QString unknown = expression.mid(index, separator.match(expression, index).capturedStart() - index);
            output.append(unknown.trimmed());

            index += unknown.length();
            lastWasOperator = false;
        }
    }

    if (openParenthesis != 0)
    {
        return QStringList("x");
    }

    while (stack.isEmpty() == false)
    {
        output.append(stack.takeLast());
    }

    return output;
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::solveRPN()
//-----------------------------------------------------------------------------
QString SystemVerilogExpressionParser::solveRPN(QStringList const& rpn, bool* validExpression) const
{
    //qDebug() << rpn;
    QStringList resultStack;
    bool isWellFormed = true;

    for (QString const& token : rpn)
    {
        QRegularExpressionMatch match = PRIMARY_LITERAL.match(token, 0, QRegularExpression::NormalMatch,
            QRegularExpression::AnchoredMatchOption);

        if (match.hasMatch())
        {
            resultStack.append(parseConstant(token));
        }
        else if (isBinaryOperator(token))
        {
            if (resultStack.size() < 2)
            {
                isWellFormed = false;
                break;
            }

            resultStack.append(solveBinary(token, resultStack.takeLast(), resultStack.takeLast()));
        }
        else if (isUnaryOperator(token))
        {
            if (resultStack.size() < 1)
            {
                isWellFormed = false;
                break;
            }

            resultStack.append(solveUnary(token, resultStack.takeLast()));
        }
        else if (token.compare(OPEN_ARRAY_STRING) == 0)
        {
            resultStack.append(token);
        }
        else if (token.compare(CLOSE_ARRAY_STRING) == 0)
        {
            QStringList items;
            while (resultStack.size() > 0 and resultStack.last().compare(OPEN_ARRAY_STRING) != 0)
            {
                items.prepend(resultStack.takeLast());
            }

            if (resultStack.size() == 0)
            {
                isWellFormed = false;
                break;
            }

            QString arrayItem(resultStack.takeLast());
            arrayItem.append(items.join(QLatin1Char(',')));
            arrayItem.append(token);

            resultStack.append(arrayItem);
        }
        else if (isSymbol(token))
        {
            resultStack.append(findSymbolValue(token));
        }
        else
        {
            isWellFormed = false;
            break;
        }
    }

    if (validExpression != nullptr)
    {
        *validExpression = isWellFormed and not resultStack.contains(QStringLiteral("x"));
    }

    return resultStack.join(QString());
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::isPlainValue()
//-----------------------------------------------------------------------------
bool SystemVerilogExpressionParser::isPlainValue(QString const& expression) const
{
    return expression.isEmpty() || isLiteral(expression) || isStringLiteral(expression);
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::baseForExpression()
//-----------------------------------------------------------------------------
int SystemVerilogExpressionParser::baseForExpression(QString const& expression) const
{
    int greatestBase = 0;

    const QStringList expressionList(convertToRPN(expression));
    for (QString const& token : expressionList)
    {
        if (isLiteral(token))
        {
            greatestBase = qMax(greatestBase, getBaseForNumber(token));
        }
        else if (isSymbol(token))
        {
            greatestBase = qMax(greatestBase, getBaseForNumber(findSymbolValue(token)));
        }
    }

    return greatestBase;
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::isStringLiteral()
//-----------------------------------------------------------------------------
bool SystemVerilogExpressionParser::isStringLiteral(QString const& expression) const
{
    static QRegularExpression stringExpression("^\\s*" + SystemVerilogSyntax::STRING_LITERAL + "\\s*$");
    return stringExpression.match(expression).hasMatch();
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::isArrayExpression()
//-----------------------------------------------------------------------------
bool SystemVerilogExpressionParser::isArrayExpression(QString const& expression) const
{
    return expression.contains(OPEN_ARRAY) && expression.contains(CLOSE_ARRAY);
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::isLiteral()
//-----------------------------------------------------------------------------
bool SystemVerilogExpressionParser::isLiteral(QString const& expression) const
{
    static QRegularExpression literalExpression("^\\s*(" + SystemVerilogSyntax::INTEGRAL_NUMBER + "|" +
        SystemVerilogSyntax::REAL_NUMBER + ")\\s*$");

    return literalExpression.match(expression).hasMatch();
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::isUnaryOperator()
//-----------------------------------------------------------------------------
bool SystemVerilogExpressionParser::isUnaryOperator(QString const& token) const
{
    return UNARY_OPERATOR.match(token, 0, QRegularExpression::NormalMatch,
        QRegularExpression::AnchoredMatchOption).hasMatch();
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::isBinaryOperator()
//-----------------------------------------------------------------------------
bool SystemVerilogExpressionParser::isBinaryOperator(QString const& token) const
{
    return BINARY_OPERATOR.match(token, 0, QRegularExpression::NormalMatch,
        QRegularExpression::AnchoredMatchOption).hasMatch();
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::solveBinary()
//-----------------------------------------------------------------------------
QString SystemVerilogExpressionParser::solveBinary(QString const& operation, QString const& leftTerm,
    QString const& rightTerm) const
{
    qreal leftOperand = leftTerm.toDouble();
    qreal rightOperand = rightTerm.toDouble();

    qreal result = 0;

    if (operation.compare(QLatin1String("+")) == 0)
    {
        result = leftOperand + rightOperand;
    }
    else if (operation.compare(QLatin1String("-")) == 0)
    {
        result = leftOperand - rightOperand;
    }
    else if (operation.compare(QLatin1String("*")) == 0)
    {
        result = leftOperand*rightOperand;
    }
    else if (operation.compare(QLatin1String("**")) == 0 or operation.compare(QLatin1String("$pow")) == 0)
    {
        if (leftOperand == 0 and rightOperand < 0)
        {
            return QStringLiteral("x");
        }

        result = qPow(leftOperand, rightOperand);
    }
    else if (operation.compare(QLatin1String("/")) == 0)
    {
        if (rightOperand == 0)
        {
            return QStringLiteral("x");
        }

        result = leftOperand/rightOperand;
    }

    if ((operation.compare(QLatin1String(">")) == 0 and leftOperand > rightOperand) ||
        (operation.compare(QLatin1String("<")) == 0 and leftOperand < rightOperand) ||
        (operation.compare(QLatin1String("==")) == 0 and leftOperand == rightOperand) ||
        (operation.compare(QLatin1String(">=")) == 0 and leftOperand >= rightOperand) ||
        (operation.compare(QLatin1String("<=")) == 0 and leftOperand <= rightOperand) ||
        (operation.compare(QLatin1String("!=")) == 0 and leftOperand != rightOperand))
    {
        return QStringLiteral("1");
    }

    if (!leftTerm.contains('.') and
        (operation.compare(QLatin1String("/")) == 0 or
        (operation.compare(QLatin1String("**")) == 0 and rightOperand < 0)))
    {
        int integerResult = result;
        return QString::number(integerResult);
    }
    else
    {
        return QString::number(result, 'f', qMax(getDecimalPrecision(leftTerm), getDecimalPrecision(rightTerm)));
    }
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::solveUnary()
//-----------------------------------------------------------------------------
QString SystemVerilogExpressionParser::solveUnary(QString const& operation, QString const& term) const
{
    if (operation.compare(QLatin1String("$clog2")) == 0)
    {
        return solveClog2(term);
    }
    else if (operation.compare(QLatin1String("$exp")) == 0)
    {
        return QString::number(qExp(term.toLongLong()));
    }
    else if (operation.compare(QLatin1String("$sqrt")) == 0)
    {
        return solveSqrt(term);
    }

    return QStringLiteral("x");
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::solveClog2()
//-----------------------------------------------------------------------------
QString SystemVerilogExpressionParser::solveClog2(QString const& value) const
{
    qreal quotient = value.toLongLong();

    if (quotient < 0)
    {
        return QStringLiteral("x");
    }
    else if (quotient == 1)
    {
        return QStringLiteral("1");
    }

    int answer = 0;
    while (quotient > 1)
    {
        quotient /= 2;
        answer++;
    }

    return QString::number(answer);
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::solveSqrt()
//-----------------------------------------------------------------------------
QString SystemVerilogExpressionParser::solveSqrt(QString const& value) const
{
    qreal radicand = value.toLongLong();
    if (radicand < 0)
    {
        return QStringLiteral("x");
    }
    return QString::number(qSqrt(radicand));
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::parseConstant()
//-----------------------------------------------------------------------------
QString SystemVerilogExpressionParser::parseConstant(QString const& constantNumber) const
{
    if (QRegularExpression("\\b(?i)true\\b").match(constantNumber).hasMatch())
    {
        return QStringLiteral("1");
    }
    else if (QRegularExpression("\\b(i)false\\b").match(constantNumber).hasMatch())
    {
        return QStringLiteral("0");
    }
    
    static QRegularExpression size("([1-9][0-9_]*)?(?=')");
    static QRegularExpression baseFormat("'" + SystemVerilogSyntax::SIGNED + "[dDbBoOhH]?");

    // Remove formating of the number.
    QString result = constantNumber;
    result.remove(size);
    result.remove(baseFormat);
    result.remove('_');

    // Number containing a dot is interpreted as a floating number. Otherwise it is seen as an integer.
    if (constantNumber.contains(QLatin1Char('.')))
    {
        return constantNumber;
    }
    else
    {
        return QString::number(result.toLongLong(0, getBaseForNumber(constantNumber)));
    }
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::isSymbol()
//-----------------------------------------------------------------------------
bool SystemVerilogExpressionParser::isSymbol(QString const& /*expression*/) const
{
    return false;
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::findSymbolValue()
//-----------------------------------------------------------------------------
QString SystemVerilogExpressionParser::findSymbolValue(QString const& symbol) const
{
    return symbol;
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::getDecimalPrecision()
//-----------------------------------------------------------------------------
int SystemVerilogExpressionParser::getDecimalPrecision(QString const& term) const
{
    int decimalLength = 0;
    int commaPosition = term.indexOf(QLatin1Char('.'));
    if  (commaPosition != -1)
    {
        decimalLength = term.length() - commaPosition - 1;
    }

    return decimalLength;
}

//-----------------------------------------------------------------------------
// Function: SystemVerilogExpressionParser::getBaseForNumber()
//-----------------------------------------------------------------------------
int SystemVerilogExpressionParser::getBaseForNumber(QString const& constantNumber) const
{
    static QRegularExpression baseFormat("'" + SystemVerilogSyntax::SIGNED + "([dDbBoOhH]?)");

    QString format = baseFormat.match(constantNumber).captured(1);
    return base_formats.value(format, 0);
}
