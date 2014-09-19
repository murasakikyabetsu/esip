#include "stdafx.h"
#include "ESInterpreter.h"

#include <wchar.h>
#include <sstream>


ESException::ESException(REASON reason, const wchar_t *pInformation, int line, int posInLine) : m_reason(reason), m_line(line), m_posInLine(posInLine)
{
	if (pInformation)
		m_information = pInformation;
}

ESException::~ESException()
{

}


///////////////////////////////////

Value::Value() : m_type(VT_UNDEFINED), m_pBase(nullptr)
{

}

Value::Value(double value) : m_type(VT_NUMBER), m_numberValue(value), m_pBase(nullptr)
{

}

Value::Value(bool value) : m_type(VT_BOOLEAN), m_booleanValue(value), m_pBase(nullptr)
{

}

Value::Value(const wchar_t *pValue) : m_type(VT_STRING), m_stringValue(pValue), m_pBase(nullptr)
{

}

Value::Value(Object *pValue) : m_type(VT_OBJECT), m_pObjectValue(pValue), m_pBase(nullptr)
{

}

Value::Value(const Value &value)
{
	m_type = value.m_type;
	switch (m_type)
	{
	case VT_INVALID:
		break;
	case VT_UNDEFINED:
		break;
	case VT_NUMBER:
		m_numberValue = value.m_numberValue;
		break;
	case VT_BOOLEAN:
		m_booleanValue = value.m_booleanValue;
		break;
	case VT_STRING:
		m_stringValue = value.m_stringValue;
		break;
	case VT_OBJECT:
		m_pObjectValue = value.m_pObjectValue;
		break;
	}

	m_pBase = value.m_pBase;
	m_referenceName = value.m_referenceName;
}

Value::~Value()
{

}

Value& Value::operator=(double value)
{
	m_type = VT_NUMBER;
	m_numberValue = value;
	return *this;
}

Value& Value::operator=(bool value)
{
	m_type = VT_BOOLEAN;
	m_booleanValue = value;
	return *this;
}

Value& Value::operator=(const wchar_t *pValue)
{
	m_type = VT_STRING;
	m_stringValue = pValue;
	return *this;
}

Value& Value::operator=(Object *pValue)
{
	m_type = VT_OBJECT;
	m_pObjectValue = pValue;
	return *this;
}

double Value::toNumber() const
{
	switch (m_type)
	{
	default:
	case VT_INVALID:
		throw ESException(ESException::R_REFERENCEERROR, m_referenceName.c_str());
	case VT_UNDEFINED:
		return 0;
	case VT_NUMBER:
		return m_numberValue;
	case VT_BOOLEAN:
		return m_booleanValue ? 1 : 0;
	case VT_STRING:
		return ::_wtof(m_stringValue.c_str());
	case VT_OBJECT:
		return 0;
	}
}

bool Value::toBoolean() const
{
	switch (m_type)
	{
	default:
	case VT_INVALID:
		throw ESException(ESException::R_REFERENCEERROR, m_referenceName.c_str());
	case VT_UNDEFINED:
		return false;
	case VT_NUMBER:
		return m_numberValue ? true : false;
	case VT_BOOLEAN:
		return m_booleanValue;
	case VT_STRING:
		return !m_stringValue.empty();
	case VT_OBJECT:
		return true;
	}
}

std::wstring Value::toString() const
{
	switch (m_type)
	{
	default:
	case VT_INVALID:
		throw ESException(ESException::R_REFERENCEERROR, m_referenceName.c_str());
	case VT_UNDEFINED:
		return L"undefined";
	case VT_NUMBER:
		{
			std::wstringstream buf;
			buf << std::fixed << m_numberValue;
			return buf.str();
		}
	case VT_BOOLEAN:
		return m_booleanValue ? L"true" : L"false";
	case VT_STRING:
		return m_stringValue;
	case VT_OBJECT:
		return L"Object";
	}
}

Object* Value::toObject() const
{
	switch (m_type)
	{
	default:
	case VT_INVALID:
		throw ESException(ESException::R_REFERENCEERROR, m_referenceName.c_str());
	case VT_UNDEFINED:
		return nullptr;
	case VT_NUMBER:
		return nullptr;
	case VT_BOOLEAN:
		return nullptr;
	case VT_STRING:
		return nullptr;
	case VT_OBJECT:
		return m_pObjectValue;
	}
}

Value Value::add(const Value &value) const
{
	if (m_type == VT_STRING || value.m_type == VT_STRING)
		return (toString() + value.toString()).c_str();

	return toNumber() + value.toNumber();
}

Value Value::sub(const Value &value) const
{
	return toNumber() - value.toNumber();
}

Value Value::mul(const Value &value) const
{
	return toNumber() * value.toNumber();
}

Value Value::div(const Value &value) const
{
	return toNumber() / value.toNumber();
}

Value Value::mod(const Value &value) const
{
	double n = toNumber();
	double d = value.toNumber();
	double q = ::floor(n / d);

	return n - d * q;
}

Value Value::lessThan(const Value &value) const
{
	if (m_type == Value::VT_STRING && value.m_type == Value::VT_STRING)
		return ::wcscmp(toString().c_str(), value.toString().c_str()) < 0;
	else
		return toNumber() < value.toNumber();
}

///////////////////////////////////////

std::list<std::unique_ptr<Object>> Object::m_objects;

Object::Object() : m_pNativeFunction(nullptr), m_pFunctionExpression(nullptr), m_pUserParam(nullptr), m_callable(false), m_pParentScope(nullptr)
{

}

Object::~Object()
{
	if (m_pDestroy)
		m_pDestroy(m_pUserParam);
}

Object* Object::create()
{
	std::unique_ptr<Object> pObject(new Object());
	Object::m_objects.push_back(std::move(pObject));

	return Object::m_objects.back().get();
}

Object* Object::create(std::function<Value(Object*, std::vector<Value>&, void*)> pNativeFunction, void *pUserParam)
{
	std::unique_ptr<Object> pObject(new Object());
	pObject->m_pNativeFunction = pNativeFunction;
	pObject->m_pUserParam = pUserParam;
	pObject->m_callable = true;

	Object::m_objects.push_back(std::move(pObject));

	return Object::m_objects.back().get();
}

Object& Object::operator = (const Object &obj)
{
	m_pNativeFunction = obj.m_pNativeFunction;
	m_pFunctionExpression = obj.m_pFunctionExpression;
	m_callable = obj.m_callable;
	m_pParentScope = obj.m_pParentScope;

	for (const auto& v : obj.m_variable)
	{
		setVariable(v.first.c_str(), v.second);
	}

	return *this;
}

void Object::setVariable(const wchar_t *pName, const Value &value)
{
	if (m_pSetVariable && m_pSetVariable(pName, value, m_pUserParam))
		return;

	m_variable[pName] = value;
	m_variable[pName].m_pBase = this;
	m_variable[pName].m_referenceName = pName;
}

Value Object::getVariable(const wchar_t *pName, bool isThis)
{
	Value value;
	value.m_pBase = this;
	value.m_referenceName = pName;

	if (m_pGetVariable && m_pGetVariable(pName, value, m_pUserParam))
		return value;

	auto it = m_variable.find(pName);
	if (it != m_variable.end())
		return it->second;

	if (isThis)
	{
		it = m_variable.find(L"__proto__");
		if (it != m_variable.end() && it->second.m_type == Value::VT_OBJECT)
			value = it->second.toObject()->getVariable(pName, true);

		value.m_pBase = this;
		value.m_referenceName = pName;
		return value;
	}

	if (m_pParentScope != nullptr)
		return m_pParentScope->getVariable(pName, false);

	value.m_type = Value::VT_INVALID;

	return value;
}

void Object::setNativeFunction(const wchar_t *pName, std::function<Value(Object*, std::vector<Value>&, void*)> pNativeFunction, void *pUserParam)
{
	setVariable(pName, Object::create(pNativeFunction, pUserParam));
}

void Object::setCapture(std::function<bool(const wchar_t *pName, const Value &value, void*)> pSetVariable, std::function<bool(const wchar_t *pName, Value &value, void*)> pGetVariable, std::function<void(void*)> pDestroy, void *pUserParam)
{
	m_pSetVariable = pSetVariable;
	m_pGetVariable = pGetVariable;
	m_pDestroy = pDestroy;
	m_pUserParam = pUserParam;
}

///////////////////////////////////////

Expression::Expression(EXPRESSIONTYPE type) : m_type(type)
{

}

Expression::~Expression()
{

}

Value Expression::run(Object *pScope, Object *pThis)
{
	switch (m_type)
	{
	case ET_NUMBER:
		return ::_wtof(m_expressionSets[0]->token.value.c_str());

	case ET_STRING:
		return m_expressionSets[0]->token.value.c_str();

	case ET_IDENTIFIER:
		return pScope->getVariable(m_expressionSets[0]->token.value.c_str(), false);

	case ET_THIS:
		return pThis;

	case ET_OBJECT:
		{
			Object *pObject = Object::create();
			for (size_t n = 0; n < m_expressionSets.size(); n += 2)
			{
				Value rValue = m_expressionSets[n + 1]->expression->run(pScope, pThis);
				if (m_expressionSets[n]->expression->m_type == ET_IDENTIFIER)
					pObject->setVariable(m_expressionSets[n]->expression->m_expressionSets[0]->token.value.c_str(), rValue);
				else
					pObject->setVariable(m_expressionSets[n]->expression->run(pScope, pThis).toString().c_str(), rValue);
			}
			return pObject;
		}

	case ET_NEW:
		{
			Value value = m_expressionSets[0]->expression->run(pScope, pThis);

			Object *pObject = Object::create();
			call(pScope, pThis, value, m_expressionSets[0].get(), pObject);

			pObject->setVariable(L"__proto__", value.toObject()->getVariable(L"prototype", true).toObject());

			return pObject;
		}
	case ET_LEFTHADSIDE:
		{
			Value value = m_expressionSets[0]->expression->run(pScope, pThis);

			for (size_t n = 1; n < m_expressionSets.size(); n++)
			{
				Object *pNewThis = value.toObject();
				if (pNewThis == nullptr)
					throw ESException(ESException::R_TYPEERROR, value.m_referenceName.c_str());

				if (m_expressionSets[n]->token.type == L'.')
				{
					value = pNewThis->getVariable(m_expressionSets[n]->expression->m_expressionSets[0]->token.value.c_str(), true);
				}
				else if (m_expressionSets[n]->token.type == L'[')
				{
					Value identifire = m_expressionSets[n]->expression->run(pScope, pNewThis);
					value = pNewThis->getVariable(identifire.toString().c_str(), true);
				}
				else if (m_expressionSets[n]->token.type == L'(')
				{
					value = call(pScope, pThis, value, m_expressionSets[n].get(), pThis);
				}

				pThis = pNewThis;
			}

			return value;
		}
	case ET_POSTFIX:
		{
			Value value = m_expressionSets[0]->expression->run(pScope, pThis);

			if (m_expressionSets[0]->token.type == ESInterpreter::TT_PLUSPLUS)
				value.m_pBase->setVariable(value.m_referenceName.c_str(), value.add(1.0));
			else if (m_expressionSets[0]->token.type == ESInterpreter::TT_MINUSMINUS)
				value.m_pBase->setVariable(value.m_referenceName.c_str(), value.sub(1.0));

			return value;
		}
	case ET_UNARY:
		{
			Value value = m_expressionSets[0]->expression->run(pScope, pThis);

			if (m_expressionSets[0]->token.type == L'-')
				return -value.toNumber();
			else if (m_expressionSets[0]->token.type == L'!')
				return !value.toBoolean();

			return value;
		}
	case ET_MULTIPLICATIVE:
	case ET_ADDITIVE:
	case ET_RELATIONAL:
		{
			Value value = m_expressionSets[0]->expression->run(pScope, pThis);
			for (size_t n = 1; n < m_expressionSets.size(); n++)
			{
				switch (m_expressionSets[n]->token.type)
				{
				case L'*':	value = value.mul(m_expressionSets[n]->expression->run(pScope, pThis));			break;
				case L'/':	value = value.div(m_expressionSets[n]->expression->run(pScope, pThis));			break;
				case L'%':	value = value.mod(m_expressionSets[n]->expression->run(pScope, pThis));			break;
				case L'+':	value = value.add(m_expressionSets[n]->expression->run(pScope, pThis));			break;
				case L'-':	value = value.sub(m_expressionSets[n]->expression->run(pScope, pThis));			break;
				case L'<':	value = value.lessThan(m_expressionSets[n]->expression->run(pScope, pThis));	break;
				}
			}
			return value;
		}
	case ET_ASSIGNMENT:
		{
			Value lValue = m_expressionSets[0]->expression->run(pScope, pThis);
			Value rValue = m_expressionSets[1]->expression->run(pScope, pThis);

			if (lValue.m_pBase == nullptr)
				throw ESException(ESException::R_TYPEERROR);

			Value value;
			switch (m_expressionSets[1]->token.type)
			{
			case L'=':							value = rValue;				break;
			case ESInterpreter::TT_PLUSEQUAL:	value = lValue.add(rValue);	break;
			}
			lValue.m_pBase->setVariable(lValue.m_referenceName.c_str(), value);

			return value;
		}
	}

	return Value();
}

Value Expression::call(Object *pScope, Object *pThis, Value &func, EXPRESSIONSET *pSet, Object *pNewThis)
{
	Object *pFunctionObject = func.toObject();
	if (pFunctionObject == nullptr || !pFunctionObject->m_callable)
		throw ESException(ESException::R_TYPEERROR, func.m_referenceName.c_str());

	std::vector<Value> arguments;
	for (size_t n = 0; n < pSet->arguments.size(); n++)
	{
		arguments.push_back(pSet->arguments[n]->run(pScope, pThis));
	}

	if (pFunctionObject->m_pNativeFunction)
		return pFunctionObject->m_pNativeFunction(pNewThis, arguments, pFunctionObject->m_pUserParam);

	// todo argumentsオブジェクト

	Object *pNewScope = Object::create();
	*pNewScope = *pFunctionObject->m_pFunctionExpression->m_pVariableEnvironment;
	pNewScope->m_pParentScope = pFunctionObject->m_pParentScope;
	for (size_t n = 0; n < pFunctionObject->m_pFunctionExpression->m_expressionSets[0]->arguments.size() && n < arguments.size(); n++)
	{
		pNewScope->setVariable(pFunctionObject->m_pFunctionExpression->m_expressionSets[0]->arguments[n]->m_expressionSets[0]->token.value.c_str(), arguments[n]);
	}

	pFunctionObject->m_pFunctionExpression->m_pFunctionBody->run(pNewScope, pNewThis);
	return pFunctionObject->m_pFunctionExpression->m_pFunctionBody->m_result.value;
}

///////////////////////////////////////

FunctionExpression::FunctionExpression() : Expression(ET_FUNCTION), m_pFunctionBody(nullptr)
{
	m_pVariableEnvironment = Object::create();
}

FunctionExpression::~FunctionExpression()
{

}

Value FunctionExpression::run(Object *pScope, Object *pThis)
{
	Object *pFunctionObject = Object::create();
	pFunctionObject->m_callable = true;
	pFunctionObject->m_pFunctionExpression = this;
	pFunctionObject->m_pParentScope = pScope;
	pFunctionObject->setVariable(L"prototype", Object::create());	// todo だめ

	return pFunctionObject;
}

///////////////////////////////////////

Statement::Statement(STATEMENTTYPE type, int line, int posInLine, void(*pCallback)(int, int)) : m_type(type), m_line(line), m_posInLine(posInLine), m_pCallback(pCallback)
{

}

Statement::~Statement()
{

}

void Statement::run(Object *pScope, Object *pThis)
{
	if (m_pCallback)
		m_pCallback(m_line, m_posInLine);

	try
	{
		switch (m_type)
		{
		case ST_EXPRESSION:
			m_result.type = RT_NORMAL;
			m_result.value = m_expressions[0]->run(pScope, pThis);
			break;

		case ST_BLOCK:
			for (auto &statement : m_statements)
			{
				statement->run(pScope, pThis);
				m_result = statement->m_result;
			}
			break;

		case ST_VAR:
			for (size_t n = 0; n < m_expressions.size(); n += 2)
			{
				if (m_expressions[n + 1])
				{
					Value value = m_expressions[n]->run(pScope, pThis);
					value.m_pBase->setVariable(value.m_referenceName.c_str(), m_expressions[n + 1]->run(pScope, pThis));
				}
			}
			break;

		case ST_FOR:
			if (m_expressions[0])
				m_expressions[0]->run(pScope, pThis);

			while (1)
			{
				if (m_expressions[1] && !m_expressions[1]->run(pScope, pThis).toBoolean())
					break;

				m_statements[0]->run(pScope, pThis);
				m_result = m_statements[0]->m_result;

				if (m_result.type == RT_BREAK)
				{
					m_result.type = RT_NORMAL;
					break;
				}

				if (m_expressions[2])
					m_expressions[2]->run(pScope, pThis);
			}
			break;

		case ST_BREAK:
			m_result.type = RT_BREAK;
			m_result.value = Value();
			break;

		case ST_IF:
			if (m_expressions[0]->run(pScope, pThis).toBoolean())
			{
				m_statements[0]->run(pScope, pThis);
				m_result = m_statements[0]->m_result;
			}
			else if (1 < m_statements.size())
			{
				m_statements[1]->run(pScope, pThis);
				m_result = m_statements[1]->m_result;
			}
			else
			{
				m_result.type = RT_NORMAL;
				m_result.value = Value();
			}
			break;

		case ST_SOURCEELEMENT:
			for (auto &statement : m_statements)
			{
				statement->run(pScope, pThis);
				m_result = statement->m_result;
			}
			break;
		}
	}
	catch (ESException &e)
	{
		if (e.m_line == 0 || e.m_posInLine == 0)
		{
			e.m_line = m_line;
			e.m_posInLine = m_posInLine;
		}
		throw;
	}
}

//////////////////////////////////////

ESInterpreter::ESInterpreter(Object *pRoot, void(*pCallback)(int, int)) : m_pRoot(pRoot), m_pCallback(pCallback), m_pSourceCode(nullptr), m_sourcePos(0), m_line(1), m_posInLine(1)
{
}


ESInterpreter::~ESInterpreter()
{
}

bool ESInterpreter::getNextToken(int type, bool exception)
{
	if (type != TT_UNDEFINED && type != m_token.type)
	{
		if (exception)
			throw ESException(ESException::R_SYNTAXERROR, m_token.value.c_str(), m_token.m_line, m_token.m_posInLine);
		return false;
	}

	const wchar_t whiteSpcae[] = L"\x0009\x000B\x000C\x0020\x00A0\xFEFF";
	const wchar_t lineTerminators[] = L"\x000A\x000D\x2028\x2029";

	while (m_pSourceCode[m_sourcePos] != L'\0' && (::wcschr(whiteSpcae, m_pSourceCode[m_sourcePos]) != nullptr || ::wcschr(lineTerminators, m_pSourceCode[m_sourcePos]) != nullptr))
	{
		if (m_pSourceCode[m_sourcePos] == L'\n')
		{
			m_line++;
			m_posInLine = 1;
		}
		else
			m_posInLine++;

		m_sourcePos++;
	}

	if (m_pSourceCode[m_sourcePos] == L'/' && m_pSourceCode[m_sourcePos + 1] == L'/')
	{
		while (m_pSourceCode[m_sourcePos] != L'\0' && m_pSourceCode[m_sourcePos] != L'\n')
		{
			m_sourcePos++;
		}

		if (m_pSourceCode[m_sourcePos] != L'\0')
			m_sourcePos++;

		return getNextToken();
	}
	if (m_pSourceCode[m_sourcePos] == L'/' && m_pSourceCode[m_sourcePos + 1] == L'*')
	{
		while (m_pSourceCode[m_sourcePos] != L'\0' && (m_pSourceCode[m_sourcePos] != L'*' || m_pSourceCode[m_sourcePos + 1] != L'/'))
		{
			m_sourcePos++;
		}

		if (m_pSourceCode[m_sourcePos] != L'\0')
			m_sourcePos += 2;

		return getNextToken();
	}

	m_token.type = TT_EOF;
	m_token.value.clear();
	m_token.m_line = m_line;
	m_token.m_posInLine = m_posInLine;

	if (::iswdigit(m_pSourceCode[m_sourcePos]))
	{
		m_token.type = TT_NUMBER;

		while (::iswdigit(m_pSourceCode[m_sourcePos]))
		{
			m_token.value += m_pSourceCode[m_sourcePos];
			m_sourcePos++;
			m_posInLine++;
		}
	}
	else if (m_pSourceCode[m_sourcePos] == L'\"')
	{
		m_token.type = TT_STRING;

		m_sourcePos++;
		m_posInLine++;

		while (m_pSourceCode[m_sourcePos] != L'\"' && m_pSourceCode[m_sourcePos] != L'\0' && ::wcschr(lineTerminators, m_pSourceCode[m_sourcePos]) == nullptr)
		{
			if (m_pSourceCode[m_sourcePos] == L'\\')
			{
				m_sourcePos++;
				if (m_pSourceCode[m_sourcePos] == L'\0')
					break;
				switch (m_pSourceCode[m_sourcePos])
				{
				case L'n':
					m_token.value += L'\n';
					break;
				case L'r':
					m_token.value += L'\r';
					break;
				case L't':
					m_token.value += L'\t';
					break;
				default:
					m_token.value += m_pSourceCode[m_sourcePos];
					break;
				}
			}
			else
				m_token.value += m_pSourceCode[m_sourcePos];

			m_sourcePos++;
			m_posInLine++;
		}

		if (m_pSourceCode[m_sourcePos] != L'\"')
			throw ESException(ESException::R_SYNTAXERROR, L"ILLEGAL", m_line, m_posInLine);

		m_sourcePos++;
		m_posInLine++;
	}
	else if (::isalpha(m_pSourceCode[m_sourcePos]) || m_pSourceCode[m_sourcePos] == L'$' || m_pSourceCode[m_sourcePos] == L'_')
	{
		m_token.type = TT_IDENTIFIER;

		while (::isalnum(m_pSourceCode[m_sourcePos]) || m_pSourceCode[m_sourcePos] == L'$' || m_pSourceCode[m_sourcePos] == L'_')
		{
			m_token.value += m_pSourceCode[m_sourcePos];
			m_sourcePos++;
			m_posInLine++;
		}

		if (m_token.value == L"function")
			m_token.type = TT_FUNCTION;
		else if (m_token.value == L"var")
			m_token.type = TT_VAR;
		else if (m_token.value == L"for")
			m_token.type = TT_FOR;
		else if (m_token.value == L"break")
			m_token.type = TT_BREAK;
		else if (m_token.value == L"if")
			m_token.type = TT_IF;
		else if (m_token.value == L"else")
			m_token.type = TT_ELSE;
		else if (m_token.value == L"this")
			m_token.type = TT_THIS;
		else if (m_token.value == L"new")
			m_token.type = TT_NEW;
	}
	else
	{
		if (m_pSourceCode[m_sourcePos] == L'+' && m_pSourceCode[m_sourcePos + 1] == L'+')
		{
			m_token.type = TT_PLUSPLUS;
			m_token.value = L"++";
			m_sourcePos += 2;
			m_posInLine += 2;
		}
		else if (m_pSourceCode[m_sourcePos] == L'-' && m_pSourceCode[m_sourcePos + 1] == L'-')
		{
			m_token.type = TT_MINUSMINUS;
			m_token.value = L"--";
			m_sourcePos += 2;
			m_posInLine += 2;
		}
		else if (m_pSourceCode[m_sourcePos] == L'+' && m_pSourceCode[m_sourcePos + 1] == L'=')
		{
			m_token.type = TT_PLUSEQUAL;
			m_token.value = L"+=";
			m_sourcePos += 2;
			m_posInLine += 2;
		}
		else
		{
			m_token.type = m_pSourceCode[m_sourcePos];
			m_token.value = m_pSourceCode[m_sourcePos];
			m_sourcePos++;
			m_posInLine++;
		}
	}

	return true;
}

std::unique_ptr<Expression> ESInterpreter::parsePrimaryExpression(TOKENTYPE requested)
{
	if (requested != TT_UNDEFINED && m_token.type != requested)
		throw ESException(ESException::R_TYPEERROR, m_token.value.c_str(), m_token.m_line, m_token.m_posInLine);

	if (m_token.type == TT_NUMBER)
	{
		auto pExpression = std::make_unique<Expression>(Expression::ET_NUMBER);
		pExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pExpression->m_expressionSets.back()->token = m_token;
		getNextToken();

		return pExpression;
	}
	else if (m_token.type == TT_STRING)
	{
		auto pExpression = std::make_unique<Expression>(Expression::ET_STRING);
		pExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pExpression->m_expressionSets.back()->token = m_token;
		getNextToken();

		return pExpression;
	}
	else if (m_token.type == TT_IDENTIFIER)
	{
		auto pExpression = std::make_unique<Expression>(Expression::ET_IDENTIFIER);
		pExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pExpression->m_expressionSets.back()->token = m_token;
		getNextToken();

		return pExpression;
	}
	else if (getNextToken(TT_THIS))
	{
		return std::make_unique<Expression>(Expression::ET_THIS);
	}
	else if (getNextToken(L'('))
	{
		auto pExpression = parseExpression();
		getNextToken(L')', true);
		
		return pExpression;
	}
	else if (getNextToken(L'{'))
	{
		auto pExpression = std::make_unique<Expression>(Expression::ET_OBJECT);

		while (m_token.type != TT_EOF && !getNextToken(L'}'))
		{
			pExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
			pExpression->m_expressionSets.back()->expression = parsePrimaryExpression();
			if (pExpression->m_expressionSets.back()->expression->m_expressionSets[0]->token.type != TT_NUMBER &&
				pExpression->m_expressionSets.back()->expression->m_expressionSets[0]->token.type != TT_STRING &&
				pExpression->m_expressionSets.back()->expression->m_expressionSets[0]->token.type != TT_IDENTIFIER)
			{
				throw ESException(ESException::R_SYNTAXERROR, m_token.value.c_str(), m_token.m_line, m_token.m_posInLine);
			}

			getNextToken(L':', true);

			pExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
			pExpression->m_expressionSets.back()->expression = parseAssignmentExpression();

			getNextToken(L',');
		}

		return pExpression;
	}

	throw ESException(ESException::R_SYNTAXERROR, m_token.value.c_str(), m_token.m_line, m_token.m_posInLine);
}

std::unique_ptr<Expression> ESInterpreter::parseMemberExpression()
{
	std::unique_ptr<Expression> pExpression;

	if (getNextToken(TT_FUNCTION))
	{
		auto pFunctionExpression = std::make_unique<FunctionExpression>();
		pFunctionExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());

		getNextToken(L'(', true);
		if (m_token.type != L')')
		{
			do
			{
				pFunctionExpression->m_expressionSets.back()->arguments.push_back(parsePrimaryExpression(TT_IDENTIFIER));
				pFunctionExpression->m_pVariableEnvironment->setVariable(pFunctionExpression->m_expressionSets.back()->token.value.c_str(), Value());
			}
			while (getNextToken(L','));
		}
		getNextToken(L')', true);

		getNextToken(L'{', true);
		pFunctionExpression->m_pFunctionBody = parseSourceElements(pFunctionExpression->m_pVariableEnvironment);
		getNextToken(L'}', true);

		pExpression = std::move(pFunctionExpression);
	}
	else if (getNextToken(TT_NEW))
	{
		pExpression = std::make_unique<Expression>(Expression::ET_NEW);

		pExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pExpression->m_expressionSets.back()->expression = parseMemberExpression();

		if (!getNextToken(L'('))
			return pExpression;	// NewExpression

		if (!getNextToken(L')'))
		{
			do
			{
				pExpression->m_expressionSets.back()->arguments.push_back(parseAssignmentExpression());
			}
			while (getNextToken(L','));
			getNextToken(L')', true);
		}
	}
	else
	{
		pExpression = std::move(parsePrimaryExpression());
	}

	if (m_token.type != L'.' && m_token.type != L'(' && m_token.type != L'[')
		return pExpression;

	auto pNewExpression = std::make_unique<Expression>(Expression::ET_LEFTHADSIDE);
	pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
	pNewExpression->m_expressionSets.back()->expression = std::move(pExpression);

	while (m_token.type == L'.' || m_token.type == L'[')
	{
		pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pNewExpression->m_expressionSets.back()->token = m_token;

		if (getNextToken(L'.'))
		{
			pNewExpression->m_expressionSets.back()->expression = parsePrimaryExpression(TT_IDENTIFIER);
		}
		else if (getNextToken(L'['))
		{
			pNewExpression->m_expressionSets.back()->expression = parseExpression();
			getNextToken(L']', true);
		}
	}

	return pNewExpression;
}

std::unique_ptr<Expression> ESInterpreter::parseLeftHandSideExpression()
{
	auto pExpression = parseMemberExpression();

	if (pExpression->m_type == Expression::ET_NEW && pExpression->m_expressionSets.size() == 1 && pExpression->m_expressionSets[0]->arguments.size() == 0)
		return pExpression;	// NewExpression

	if (m_token.type != L'(')
		return pExpression;

	std::unique_ptr<Expression> pNewExpression;
	if (pExpression->m_type == Expression::ET_LEFTHADSIDE)
	{
		pNewExpression = std::move(pExpression);
	}
	else
	{
		pNewExpression = std::move(std::make_unique<Expression>(Expression::ET_LEFTHADSIDE));
		pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pNewExpression->m_expressionSets.back()->expression = std::move(pExpression);
	}

	while (m_token.type == L'.' || m_token.type == L'[' || m_token.type == L'(')
	{
		pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pNewExpression->m_expressionSets.back()->token = m_token;
		if (getNextToken(L'.'))
		{
			pNewExpression->m_expressionSets.back()->expression = parsePrimaryExpression(TT_IDENTIFIER);
		}
		else if (getNextToken(L'['))
		{
			pNewExpression->m_expressionSets.back()->expression = parseExpression();
			getNextToken(L']', true);
		}
		else if (getNextToken(L'('))
		{
			if (!getNextToken(L')'))
			{
				do
				{
					pNewExpression->m_expressionSets.back()->arguments.push_back(parseAssignmentExpression());
				}
				while (getNextToken(L','));
				getNextToken(L')', true);
			}
		}
	}

	return pNewExpression;
}

std::unique_ptr<Expression> ESInterpreter::parsePostfixExpression()
{
	auto pExpression = parseLeftHandSideExpression();

	if (m_token.type != TT_MINUSMINUS && m_token.type != TT_PLUSPLUS)
		return pExpression;

	auto pNewExpression = std::make_unique<Expression>(Expression::ET_POSTFIX);

	pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
	pNewExpression->m_expressionSets.back()->expression = std::move(pExpression);
	pNewExpression->m_expressionSets.back()->token = m_token;
	getNextToken();

	return pNewExpression;
}

std::unique_ptr<Expression> ESInterpreter::parseUnaryExpression()
{
	if (m_token.type == L'-' || m_token.type == L'!')
	{
		auto pExpression = std::make_unique<Expression>(Expression::ET_UNARY);

		pExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pExpression->m_expressionSets.back()->token = m_token;
		getNextToken();
		pExpression->m_expressionSets.back()->expression = parseUnaryExpression();

		return pExpression;
	}

	return parsePostfixExpression();
}

std::unique_ptr<Expression> ESInterpreter::parseMultiplicativeExpression()
{
	auto pExpression = parseUnaryExpression();

	if (m_token.type != L'*' && m_token.type != L'/' && m_token.type != L'%')
		return pExpression;

	auto pNewExpression = std::make_unique<Expression>(Expression::ET_MULTIPLICATIVE);
	pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
	pNewExpression->m_expressionSets.back()->expression = std::move(pExpression);

	while (m_token.type == L'*' || m_token.type == L'/' || m_token.type == L'%')
	{
		pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pNewExpression->m_expressionSets.back()->token = m_token;
		getNextToken();
		pNewExpression->m_expressionSets.back()->expression = parseMultiplicativeExpression();
	}

	return pNewExpression;
}

std::unique_ptr<Expression> ESInterpreter::parseAdditiveExpression()
{
	auto pExpression = parseMultiplicativeExpression();

	if (m_token.type != L'+' && m_token.type != L'-')
		return pExpression;

	auto pNewExpression = std::make_unique<Expression>(Expression::ET_ADDITIVE);

	pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
	pNewExpression->m_expressionSets.back()->expression = std::move(pExpression);

	while (m_token.type == L'+' || m_token.type == L'-')
	{
		pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pNewExpression->m_expressionSets.back()->token = m_token;
		getNextToken();
		pNewExpression->m_expressionSets.back()->expression = parseMultiplicativeExpression();
	}

	return pNewExpression;
}

std::unique_ptr<Expression> ESInterpreter::parseShiftExpression()
{
	return parseAdditiveExpression();
}

std::unique_ptr<Expression> ESInterpreter::parseRelationalExpression()
{
	auto pExpression = parseShiftExpression();

	if (m_token.type != L'<')
		return pExpression;

	auto pNewExpression = std::make_unique<Expression>(Expression::ET_RELATIONAL);

	pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
	pNewExpression->m_expressionSets.back()->expression = std::move(pExpression);

	while (m_token.type == L'<')
	{
		pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pNewExpression->m_expressionSets.back()->token = m_token;
		getNextToken();
		pNewExpression->m_expressionSets.back()->expression = parseShiftExpression();
	}

	return pNewExpression;
}

std::unique_ptr<Expression> ESInterpreter::parseEqualityExpression()
{
	return parseRelationalExpression();
}

std::unique_ptr<Expression> ESInterpreter::parseBinaryBitwiseExpression()
{
	return parseEqualityExpression();
}

std::unique_ptr<Expression> ESInterpreter::parseBinaryLogicalExpression()
{
	return parseBinaryBitwiseExpression();
}

std::unique_ptr<Expression> ESInterpreter::parseConditionalExpression()
{
	return parseBinaryLogicalExpression();
}

std::unique_ptr<Expression> ESInterpreter::parseAssignmentExpression()
{
	auto pExpression = parseConditionalExpression();
	if (Expression::ET_LEFTHADSIDE < pExpression->m_type || (m_token.type != L'=' && m_token.type != TT_PLUSEQUAL))
		return pExpression;

	auto pNewExpression = std::make_unique<Expression>(Expression::ET_ASSIGNMENT);

	pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
	pNewExpression->m_expressionSets.back()->expression = std::move(pExpression);

	pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
	pNewExpression->m_expressionSets.back()->token = m_token;
	getNextToken();
	pNewExpression->m_expressionSets.back()->expression = parseAssignmentExpression(); 
	
	return pNewExpression;
}

std::unique_ptr<Expression> ESInterpreter::parseExpression()
{
	return parseAssignmentExpression();
}

std::unique_ptr<Statement> ESInterpreter::parseStatement(Object *pVariableEnvironment)
{
	try
	{
		if (getNextToken(L'{'))
		{
			auto pStatement = std::make_unique<Statement>(Statement::ST_BLOCK, m_line, m_posInLine, m_pCallback);
			while (m_token.type != TT_EOF && m_token.type != L'}')
			{
				pStatement->m_statements.push_back(parseStatement(pVariableEnvironment));
			}
			getNextToken(L'}', true);

			return pStatement;
		}
		else if (getNextToken(TT_VAR))
		{
			auto pStatement = std::make_unique<Statement>(Statement::ST_VAR, m_line, m_posInLine, m_pCallback);

			do
			{
				auto pExpression = parsePrimaryExpression(TT_IDENTIFIER);
				pVariableEnvironment->setVariable(pExpression->m_expressionSets[0]->token.value.c_str(), Value());
				pStatement->m_expressions.push_back(std::move(pExpression));

				if (getNextToken(L'='))
					pStatement->m_expressions.push_back(parseAssignmentExpression());
				else
					pStatement->m_expressions.push_back(nullptr);
			}
			while (getNextToken(L','));

			getNextToken(L';');

			return pStatement;
		}
		else if (getNextToken(TT_FOR))
		{
			auto pStatement = std::make_unique<Statement>(Statement::ST_FOR, m_line, m_posInLine, m_pCallback);

			getNextToken(L'(', true);
			pStatement->m_expressions.push_back(m_token.type == L';' ? nullptr : parseExpression());
			getNextToken(L';', true);
			pStatement->m_expressions.push_back(m_token.type == L';' ? nullptr : parseExpression());
			getNextToken(L';', true);
			pStatement->m_expressions.push_back(m_token.type == L')' ? nullptr : parseExpression());
			getNextToken(L')', true);

			pStatement->m_statements.push_back(parseStatement(pVariableEnvironment));

			return pStatement;
		}
		else if (getNextToken(TT_BREAK))
		{
			getNextToken(L';');
			return std::make_unique<Statement>(Statement::ST_BREAK, m_line, m_posInLine, m_pCallback);
		}
		else if (getNextToken(TT_IF))
		{
			auto pStatement = std::make_unique<Statement>(Statement::ST_IF, m_line, m_posInLine, m_pCallback);

			getNextToken(L'(', true);
			pStatement->m_expressions.push_back(parseExpression());
			getNextToken(L')', true);

			pStatement->m_statements.push_back(parseStatement(pVariableEnvironment));

			if (getNextToken(TT_ELSE))
				pStatement->m_statements.push_back(parseStatement(pVariableEnvironment));

			return pStatement;
		}

		auto pStatement = std::make_unique<Statement>(Statement::ST_EXPRESSION, m_line, m_posInLine, m_pCallback);
		pStatement->m_expressions.push_back(parseExpression());
		getNextToken(L';');

		return pStatement;
	}
	catch (ESException &e)
	{
		if (e.m_line == 0 || e.m_posInLine == 0)
		{
			e.m_line = m_line;
			e.m_posInLine = m_posInLine;
		}
		throw;
	}
}

std::unique_ptr<Statement> ESInterpreter::parseSourceElements(Object *pVariableEnvironment)
{
	auto pStatement = std::make_unique<Statement>(Statement::ST_SOURCEELEMENT, m_line, m_posInLine, m_pCallback);
	while (m_token.type != TT_EOF && m_token.type != L'}')
	{
		pStatement->m_statements.push_back(parseStatement(pVariableEnvironment));
	}
	return pStatement;
}

std::unique_ptr<Statement> ESInterpreter::parseProgram()
{
	try
	{
		getNextToken();
		return parseSourceElements(m_pRoot);
	}
	catch (ESException &e)
	{
		if (e.m_line == 0 || e.m_posInLine == 0)
		{
			e.m_line = m_line;
			e.m_posInLine = m_posInLine;
		}
		throw;
	}
}

Value ESInterpreter::run(const wchar_t *pSourceCode)
{
	m_pSourceCode = pSourceCode;
	m_sourcePos = 0;
	m_line = 1;
	m_posInLine = 1;

	m_programs.push_back(parseProgram());
	m_programs.back()->run(m_pRoot, m_pRoot);

	return m_programs.back()->m_result.value;
}
