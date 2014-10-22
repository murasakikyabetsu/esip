#include "stdafx.h"
#include "ESInterpreter.h"

#include "StandardObjects.h"

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

Value::Value(double value) : m_type(VT_NUMBER), m_pBase(nullptr)
{
	m_value.number = value;
}

Value::Value(bool value) : m_type(VT_BOOLEAN), m_pBase(nullptr)
{
	m_value.boolean = value;
}

Value::Value(const wchar_t *pValue) : m_type(VT_STRING), m_stringValue(pValue), m_pBase(nullptr)
{

}

Value::Value(Object *pValue) : m_type(VT_OBJECT), m_pBase(nullptr)
{
	m_value.pObject = pValue;
}

Value::Value(const Value &value)
{
	m_type = value.m_type;
	if (m_type == VT_STRING)
		m_stringValue = value.m_stringValue;
	else
		m_value = value.m_value;
	m_pBase = value.m_pBase;
	m_referenceName = value.m_referenceName;
}

Value::Value(Value && value)
{
	m_type = value.m_type;
	if (m_type == VT_STRING)
		m_stringValue = std::move(value.m_stringValue);
	else
		m_value = value.m_value;
	m_pBase = value.m_pBase;
	m_referenceName = std::move(value.m_referenceName);
}

Value::~Value()
{

}

Value& Value::operator=(double value)
{
	m_type = VT_NUMBER;
	m_value.number = value;
	return *this;
}

Value& Value::operator=(bool value)
{
	m_type = VT_BOOLEAN;
	m_value.boolean = value;
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
	m_value.pObject = pValue;
	return *this;
}

Value& Value::operator=(const Value &value)
{
	m_type = value.m_type;
	if (m_type == VT_STRING)
		m_stringValue = value.m_stringValue;
	else
		m_value = value.m_value;
	m_pBase = value.m_pBase;
	m_referenceName = value.m_referenceName;

	return *this;
}

Value& Value::operator=(Value && value)
{
	m_type = value.m_type;
	if (m_type == VT_STRING)
		m_stringValue = std::move(value.m_stringValue);
	else
		m_value = value.m_value;
	m_pBase = value.m_pBase;
	m_referenceName = std::move(value.m_referenceName);

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
		return m_value.number;
	case VT_BOOLEAN:
		return m_value.boolean ? 1 : 0;
	case VT_STRING:
		return ::_wtof(m_stringValue.c_str());
	case VT_OBJECT:
		return 0;
	}
}

int Value::toInt32() const
{
	return (int)toNumber();
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
		return m_value.number ? true : false;
	case VT_BOOLEAN:
		return m_value.boolean;
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
			if (m_value.number == (int)m_value.number)
				buf << (int)m_value.number;
			else
				buf << std::fixed << m_value.number;
			return buf.str();
		}
	case VT_BOOLEAN:
		return m_value.boolean ? L"true" : L"false";
	case VT_STRING:
		return m_stringValue;
	case VT_OBJECT:
		{
			Value value = m_value.pObject->getVariable(L"toString", true);
			if (!value.isCallable())
				throw ESException(ESException::R_REFERENCEERROR, m_referenceName.c_str());

			std::vector<Value> arguments;
			return value.toObject()->call(m_value.pObject, arguments, false).toString();
		}
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
		if (m_pBase)
		{
			ObjectPtr string = m_pBase->m_pInterpreter->getGlobalObject()->getVariable(L"String", true).toObject();
			return string->construct({ m_stringValue.c_str() }).toObject();
		}
		_ASSERT(0);	// todo checking
		return nullptr;
	case VT_OBJECT:
		return m_value.pObject;
	}
}

bool Value::isCallable() const
{
	switch (m_type)
	{
	default:
	case VT_INVALID:
	case VT_UNDEFINED:
	case VT_NUMBER:
	case VT_BOOLEAN:
	case VT_STRING:
		return false;
	case VT_OBJECT:
		return m_value.pObject->m_pNativeCall || m_value.pObject->m_pNativeConstructor || m_value.pObject->m_pFunctionBody;
	}
}

Value Value::operator+(const Value &value) const
{
	if (m_type == VT_STRING || value.m_type == VT_STRING)
		return (toString() + value.toString()).c_str();

	return toNumber() + value.toNumber();
}

Value Value::operator-(const Value &value) const
{
	return toNumber() - value.toNumber();
}

Value Value::operator*(const Value &value) const
{
	return toNumber() * value.toNumber();
}

Value Value::operator/(const Value &value) const
{
	return toNumber() / value.toNumber();
}

Value Value::operator%(const Value &value) const
{
	double n = toNumber();
	double d = value.toNumber();
	double q = ::floor(n / d);

	return n - d * q;
}

Value Value::operator<(const Value &value) const
{
	if (m_type == Value::VT_STRING && value.m_type == Value::VT_STRING)
		return ::wcscmp(toString().c_str(), value.toString().c_str()) < 0;
	else
		return toNumber() < value.toNumber();
}

Value Value::operator|(const Value &value) const
{
	return (double)(toInt32() | value.toInt32());
}

Value Value::operator&(const Value &value) const
{
	return (double)(toInt32() & value.toInt32());
}

Value Value::operator==(const Value &value) const
{
	if (m_type == value.m_type)
	{
		switch (m_type)
		{
		case VT_INVALID:
			return true;
		case VT_UNDEFINED:
			return true;
		case VT_NUMBER:
			return toNumber() == value.toNumber();
		case VT_BOOLEAN:
			return toBoolean() == value.toBoolean();
			break;
		case VT_STRING:
			return toString() == value.toString();
		case VT_OBJECT:
			return toObject() == value.toObject();
		}
	}

	return toNumber() == value.toNumber();
}

///////////////////////////////////////

Object::Object(ESInterpreter *pInterpreter) : m_pInterpreter(pInterpreter), m_pNativeCall(nullptr), m_pNativeConstructor(nullptr), m_pFunctionBody(nullptr), m_pUserParam(nullptr), m_marked(false)
{
#ifdef _DEBUG
//	::wprintf_s(L"[Created : 0x%08X]\n", this);
#endif
}

Object::~Object()
{
	if (m_pDestroy)
		m_pDestroy(m_pUserParam);

#ifdef _DEBUG
//	::wprintf_s(L"[Destroyed : 0x%08X]\n", this);
#endif
}

Object& Object::operator = (const Object &obj)
{
	m_pNativeCall = obj.m_pNativeCall;
	m_pNativeConstructor = obj.m_pNativeConstructor;
	m_pFunctionBody = obj.m_pFunctionBody;
	m_pScope = obj.m_pScope;

	for (const auto& v : obj.m_variable)
	{
		setVariable(v.first.c_str(), v.second);
	}

	return *this;
}

Value Object::call(Object *pThis, std::vector<Value> arguments, bool isConstruct)
{
	if (isConstruct && m_pNativeConstructor)
		return m_pNativeConstructor(pThis, arguments, m_pUserParam);
	if (!isConstruct && m_pNativeCall)
		return m_pNativeCall(pThis, arguments, m_pUserParam);

	if (!m_pFunctionBody)
		throw ESException(ESException::R_TYPEERROR);

	// todo argumentsオブジェクト

	ObjectPtr pNewScope = m_pInterpreter->createObject();
	*pNewScope = *m_pFunctionBody->m_pVariableEnvironment;
	pNewScope->m_pScope = m_pScope;
	for (size_t n = 0; n < m_pFunctionBody->m_expressionSets[0]->arguments.size() && n < arguments.size(); n++)
	{
		pNewScope->setVariable(m_pFunctionBody->m_expressionSets[0]->arguments[n]->m_expressionSets[0]->token.value.c_str(), arguments[n]);
	}

	m_pFunctionBody->m_pFunctionBody->run(pNewScope, pThis);
	return m_pFunctionBody->m_pFunctionBody->m_result.value;
}

Value Object::construct(std::vector<Value> arguments)
{
	ObjectPtr pObject = m_pInterpreter->createObject();
	pObject->m_class = L"Object";
	pObject->m_pPrototype = getVariable(L"prototype", true).toObject();

	Value value = call(pObject, std::move(arguments), true);

	return value.m_type == Value::VT_OBJECT ? value : (Object*)pObject;
}

bool Object::hasInstance(Object *pObject)
{
	if (!pObject->m_pPrototype)
		throw ESException(ESException::R_TYPEERROR);

	ObjectPtr pPrototype = m_pPrototype;
	while (pPrototype)
	{
		if (pPrototype == pObject->m_pPrototype)
			return true;

		pPrototype = pPrototype->m_pPrototype;
	}

	return false;
}

void Object::setVariable(const wchar_t *pName, Value value)
{
	if (m_pSetVariable && m_pSetVariable(pName, value, m_pUserParam))
		return;

	m_variable[pName] = std::move(value);
	m_variable[pName].m_pBase = this;
	m_variable[pName].m_referenceName = pName;
}

Value Object::getVariable(const wchar_t *pName, bool isThis)
{
	Value value;

	if (m_pGetVariable && m_pGetVariable(pName, value, m_pUserParam))
	{
		value.m_pBase = this;
		value.m_referenceName = pName;
		return value;
	}

	auto it = m_variable.find(pName);
	if (it != m_variable.end())
		return it->second;

	if (isThis)
	{
		if (m_pPrototype)
			value = m_pPrototype->getVariable(pName, true);

		value.m_pBase = this;
		value.m_referenceName = pName;
		return value;
	}

	if (m_pScope)
		return m_pScope->getVariable(pName, false);

	value.m_type = Value::VT_INVALID;
	value.m_pBase = this;
	value.m_referenceName = pName;

	return value;
}

void Object::setCapture(std::function<bool(const wchar_t *pName, const Value &value, void*)> pSetVariable, std::function<bool(const wchar_t *pName, Value &value, void*)> pGetVariable, std::function<void(void*)> pDestroy, void *pUserParam)
{
	m_pSetVariable = pSetVariable;
	m_pGetVariable = pGetVariable;
	m_pDestroy = pDestroy;
	m_pUserParam = pUserParam;
}

void Object::mark()
{
	if (m_marked)
		return;

	m_marked = true;

	for (const auto &v : m_variable)
	{
		if (v.second.m_type == Value::VT_OBJECT)
			v.second.toObject()->mark();
	}
}

///////////////////////////////////////

ObjectPtr::ObjectPtr() : m_pObject(nullptr), m_pInterpreter(nullptr)
{

}

ObjectPtr::ObjectPtr(Object *pObject) : m_pObject(pObject)
{
	if (m_pObject)
	{
		m_pInterpreter = m_pObject->m_pInterpreter;
		m_pInterpreter->m_roots.push_back(m_pObject);
	}
}

ObjectPtr::ObjectPtr(const ObjectPtr &pObject)
{
	m_pObject = pObject.m_pObject;

	if (m_pObject)
	{
		m_pInterpreter = m_pObject->m_pInterpreter;
		m_pInterpreter->m_roots.push_back(m_pObject);
	}
}

ObjectPtr::~ObjectPtr()
{
	if (m_pObject && m_pInterpreter && m_pInterpreter->m_roots.size())
	{
		auto it = std::find(m_pInterpreter->m_roots.begin(), m_pInterpreter->m_roots.end(), m_pObject);
		if (it != m_pInterpreter->m_roots.end())
		{
			m_pInterpreter->m_roots.erase(it);
		}
	}
}

ObjectPtr& ObjectPtr::operator=(const ObjectPtr &pObject)
{
	if (m_pObject && m_pInterpreter && m_pInterpreter->m_roots.size())
	{
		auto it = std::find(m_pInterpreter->m_roots.begin(), m_pInterpreter->m_roots.end(), m_pObject);
		if (it != m_pInterpreter->m_roots.end())
		{
			m_pInterpreter->m_roots.erase(it);
		}
	}

	m_pObject = pObject.m_pObject;

	if (m_pObject)
	{
		m_pInterpreter = m_pObject->m_pInterpreter;
		m_pInterpreter->m_roots.push_back(m_pObject);
	}

	return *this;
}

ObjectPtr& ObjectPtr::operator = (Object *pObject)
{
	if (m_pObject && m_pInterpreter && m_pInterpreter->m_roots.size())
	{
		auto it = std::find(m_pInterpreter->m_roots.begin(), m_pInterpreter->m_roots.end(), m_pObject);
		if (it != m_pInterpreter->m_roots.end())
		{
			m_pInterpreter->m_roots.erase(it);
		}
	}

	m_pObject = pObject;

	if (m_pObject)
	{
		m_pInterpreter = m_pObject->m_pInterpreter;
		m_pInterpreter->m_roots.push_back(m_pObject);
	}

	return *this;
}

Object* ObjectPtr::operator->()
{
	return m_pObject;
}

bool ObjectPtr::operator==(const ObjectPtr &pObject)
{
	return m_pObject == pObject.m_pObject;
}

ObjectPtr::operator bool()
{
	return m_pObject != nullptr;
}

ObjectPtr::operator Object*()
{
	return m_pObject;
}

///////////////////////////////////////


Expression::Expression(ESInterpreter *pInterpeter, EXPRESSIONTYPE type) : m_pInterpreter(pInterpeter), m_type(type)
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
		return m_expressionSets[0]->numberValue;

	case ET_STRING:
		return m_expressionSets[0]->token.value.c_str();

	case ET_IDENTIFIER:
		return pScope->getVariable(m_expressionSets[0]->token.value.c_str(), false);

	case ET_THIS:
		return pThis;

	case ET_BOOLEAN:
		return m_expressionSets[0]->token.type == ESInterpreter::TT_TRUE ? true : false;

	case ET_OBJECT:
		{
			ObjectPtr pObject = m_pInterpreter->createObject();
			for (size_t n = 0; n < m_expressionSets.size(); n += 2)
			{
				Value rValue = m_expressionSets[n + 1]->expression->run(pScope, pThis);
				if (m_expressionSets[n]->expression->m_type == ET_IDENTIFIER)
					pObject->setVariable(m_expressionSets[n]->expression->m_expressionSets[0]->token.value.c_str(), rValue);
				else
					pObject->setVariable(m_expressionSets[n]->expression->run(pScope, pThis).toString().c_str(), rValue);
			}
			return (Object*)pObject;
		}
	case ET_ARRAY:
		{
			Value value = m_pInterpreter->getGlobalObject()->getVariable(L"Array", true);

			value = value.toObject()->construct({ Value((double)m_expressionSets.size()) });

			ObjectPtr pArray = value.toObject();
			for (size_t n = 0; n < m_expressionSets.size(); n++)
			{
				std::wstringstream buf;
				buf << n;
				if (m_expressionSets[n])
					pArray->setVariable(buf.str().c_str(), m_expressionSets[n]->expression->run(pScope, pThis));
				else
					pArray->setVariable(buf.str().c_str(), Value());
			}

			return value;
		}
	case ET_NEW:
		{
			Value value = m_expressionSets[0]->expression->run(pScope, pThis);

			if (!value.isCallable())
				throw ESException(ESException::R_TYPEERROR, value.m_referenceName.c_str());

			std::vector<ObjectPtr> holder;
			std::vector<Value> arguments;
			for (size_t n = 0; n < m_expressionSets[0]->arguments.size(); n++)
			{
				arguments.push_back(m_expressionSets[0]->arguments[n]->run(pScope, pThis));
				if (arguments.back().m_type == Value::VT_OBJECT)
					holder.push_back(arguments.back().toObject());
			}

			ObjectPtr pObject = value.toObject();
			return pObject->construct(arguments);
		}
	case ET_LEFTHADSIDE:
		{
			Value value = m_expressionSets[0]->expression->run(pScope, pThis);

			for (size_t n = 1; n < m_expressionSets.size(); n++)
			{
				ObjectPtr pNewThis = value.toObject();
				if (!pNewThis)
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
					if (!value.isCallable())
						throw ESException(ESException::R_TYPEERROR, value.m_referenceName.c_str());

					std::vector<ObjectPtr> holder;
					std::vector<Value> arguments;
					for (size_t a = 0; a < m_expressionSets[n]->arguments.size(); a++)
					{
						arguments.push_back(m_expressionSets[n]->arguments[a]->run(pScope, pThis));
						if (arguments.back().m_type == Value::VT_OBJECT)
							holder.push_back(arguments.back().toObject());
					}

					ObjectPtr pObject = value.toObject();
					value = pObject->call(pThis, std::move(arguments), false);
				}

				pThis = pNewThis;
			}

			return value;
		}
	case ET_POSTFIX:
		{
			Value value = m_expressionSets[0]->expression->run(pScope, pThis);

			if (m_expressionSets[0]->token.type == ESInterpreter::TT_PLUSPLUS)
				value.m_pBase->setVariable(value.m_referenceName.c_str(), value + 1.0);
			else if (m_expressionSets[0]->token.type == ESInterpreter::TT_MINUSMINUS)
				value.m_pBase->setVariable(value.m_referenceName.c_str(), value - 1.0);

			return value;
		}
	case ET_UNARY:
		{
			Value value = m_expressionSets[0]->expression->run(pScope, pThis);

			switch (m_expressionSets[0]->token.type)
			{
			case L'-':	return -value.toNumber();
			case L'!':	return !value.toBoolean();
			case L'~':	return (double)~value.toInt32();
			}

			return value;
		}
	case ET_MULTIPLICATIVE:
	case ET_ADDITIVE:
	case ET_RELATIONAL:
	case ET_BITWISE:
	case ET_EQUALITY:
		{
			Value value = m_expressionSets[0]->expression->run(pScope, pThis);
			for (size_t n = 1; n < m_expressionSets.size(); n++)
			{
				switch (m_expressionSets[n]->token.type)
				{
				case L'*':	value = value * m_expressionSets[n]->expression->run(pScope, pThis);	break;
				case L'/':	value = value / m_expressionSets[n]->expression->run(pScope, pThis);	break;
				case L'%':	value = value % m_expressionSets[n]->expression->run(pScope, pThis);	break;
				case L'+':	value = value + m_expressionSets[n]->expression->run(pScope, pThis);	break;
				case L'-':	value = value - m_expressionSets[n]->expression->run(pScope, pThis);	break;
				case L'<':	value = value < m_expressionSets[n]->expression->run(pScope, pThis);	break;
				case ESInterpreter::TT_LESSEQUAL:	value = (m_expressionSets[n]->expression->run(pScope, pThis) < value).toBoolean() ? false : true; break;
				case L'|':	value = value | m_expressionSets[n]->expression->run(pScope, pThis);	break;
				case L'^':	value = (double)(value.toInt32() ^ m_expressionSets[n]->expression->run(pScope, pThis).toInt32());	break;
				case L'&':	value = value & m_expressionSets[n]->expression->run(pScope, pThis);	break;
				case ESInterpreter::TT_EQUALEQUAL:	value = value == m_expressionSets[n]->expression->run(pScope, pThis);	break;
				}
			}
			return value;
		}
	case ET_BINARYLOGICAL:
		{
			Value value = m_expressionSets[0]->expression->run(pScope, pThis);
			for (size_t n = 1; n < m_expressionSets.size(); n++)
			{
				switch (m_expressionSets[n]->token.type)
				{
				case ESInterpreter::TT_ANDAND:
					if (!value.toBoolean())
						return false;
					value = m_expressionSets[n]->expression->run(pScope, pThis);
					break;
				case ESInterpreter::TT_OROR:
					if (value.toBoolean())
						return true;
					value = m_expressionSets[n]->expression->run(pScope, pThis);
					break;
				}
			}
			return value;
		}
	case ET_ASSIGNMENT:
		{
			Value lValue = m_expressionSets[0]->expression->run(pScope, pThis);
			Value rValue = m_expressionSets[1]->expression->run(pScope, pThis);

			if (!lValue.m_pBase)
				throw ESException(ESException::R_TYPEERROR);

			Value value;
			switch (m_expressionSets[1]->token.type)
			{
			case L'=':							value = rValue;				break;
			case ESInterpreter::TT_PLUSEQUAL:	value = lValue + rValue;	break;
			case ESInterpreter::TT_OREQUAL:		value = lValue | rValue;	break;
			case ESInterpreter::TT_ANDEQUAL:	value = lValue & rValue;	break;
			}
			lValue.m_pBase->setVariable(lValue.m_referenceName.c_str(), value);

			return value;
		}
	}

	return Value();
}

///////////////////////////////////////

FunctionExpression::FunctionExpression(ESInterpreter *pInterpeter) : Expression(pInterpeter, ET_FUNCTION), m_pFunctionBody(nullptr)
{
	m_pVariableEnvironment = pInterpeter->createObject();
}

FunctionExpression::~FunctionExpression()
{

}

Value FunctionExpression::run(Object *pScope, Object *pThis)
{
	return m_pInterpreter->createFunctionObject(this, pScope);
}

///////////////////////////////////////

Statement::Statement(ESInterpreter *pInterpeter, STATEMENTTYPE type, int line, int posInLine, void(*pCallback)(int, int)) : m_pInterpreter(pInterpeter), m_type(type), m_line(line), m_posInLine(posInLine), m_pCallback(pCallback)
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
				if (m_result.type == RT_BREAK || m_result.type == RT_CONTINUE || m_result.type == RT_RETURN)
					break;
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
			else if (m_statements[0])
				m_statements[0]->run(pScope, pThis);

			while (1)
			{
				if (m_expressions[1] && !m_expressions[1]->run(pScope, pThis).toBoolean())
					break;

				m_statements[1]->run(pScope, pThis);
				m_result = m_statements[1]->m_result;

				if (m_result.type == RT_BREAK)
				{
					m_result.type = RT_NORMAL;
					break;
				}
				if (m_result.type == RT_CONTINUE)
					m_result.type = RT_NORMAL;
				if (m_result.type == RT_RETURN)
					break;

				if (m_expressions[2])
					m_expressions[2]->run(pScope, pThis);
			}
			break;

		case ST_WHILE:
			{
				while (1)
				{
					if (!m_expressions[0]->run(pScope, pThis).toBoolean())
						break;

					m_statements[0]->run(pScope, pThis);
					m_result = m_statements[0]->m_result;

					if (m_result.type == RT_BREAK)
					{
						m_result.type = RT_NORMAL;
						break;
					}
					if (m_result.type == RT_CONTINUE)
						m_result.type = RT_NORMAL;
					if (m_result.type == RT_RETURN)
						break;
				}

				break;
			}
		case ST_BREAK:
			m_result.type = RT_BREAK;
			m_result.value = Value();
			break;

		case ST_CONTINUE:
			m_result.type = RT_CONTINUE;
			m_result.value = Value();
			break;

		case ST_RETURN:
			if (0 < m_expressions.size())
				m_result.value = m_expressions[0]->run(pScope, pThis);
			m_result.type = RT_RETURN;
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
			for (const auto &statement : m_statements)
			{
				statement->run(pScope, pThis);
				m_result = statement->m_result;
				if (m_result.type == RT_RETURN)
					break;
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

ESInterpreter::ESInterpreter(void(*pCallback)(int, int))
	: m_pCallback(pCallback), m_pSourceCode(nullptr), m_sourcePos(0), m_line(1), m_posInLine(1)
{
	createStandardObject();
	createStandardFunctionObject();

	m_pGlobalObject = createObject();
	m_pGlobalObject->setVariable(L"Object", (Object*)m_pStandardObject);
	m_pGlobalObject->setVariable(L"Function", (Object*)m_pStandardFunctionObject);
	m_pGlobalObject->setVariable(L"parseInt", createFunctionObject(ESInterpreter::parseInt, nullptr, nullptr));
	m_pGlobalObject->setVariable(L"Date", Date::createObject(this));
	m_pGlobalObject->setVariable(L"Math", Math::createObject(this));
	m_pGlobalObject->setVariable(L"Array", Array::createObject(this));
	m_pGlobalObject->setVariable(L"String", String::createObject(this));
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
		{
			m_line++;
			m_posInLine = 1;

			m_sourcePos++;
		}

		return getNextToken();
	}
	if (m_pSourceCode[m_sourcePos] == L'/' && m_pSourceCode[m_sourcePos + 1] == L'*')
	{
		while (m_pSourceCode[m_sourcePos] != L'\0' && (m_pSourceCode[m_sourcePos] != L'*' || m_pSourceCode[m_sourcePos + 1] != L'/'))
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

		if (m_pSourceCode[m_sourcePos] != L'\0')
		{
			m_posInLine += 2;

			m_sourcePos += 2;
		}

		return getNextToken();
	}

	m_token.type = TT_EOF;
	m_token.value.clear();
	m_token.m_line = m_line;
	m_token.m_posInLine = m_posInLine;

	if (::iswdigit(m_pSourceCode[m_sourcePos]))
	{
		m_token.type = TT_NUMBER;

		if (m_pSourceCode[m_sourcePos + 1] == L'x')
		{
			m_token.value = L"x";
			m_sourcePos += 2;
			m_posInLine += 2;

			while (::isxdigit(m_pSourceCode[m_sourcePos]))
			{
				m_token.value += m_pSourceCode[m_sourcePos];
				m_sourcePos++;
				m_posInLine++;
			}
		}
		else
		{
			while (::iswdigit(m_pSourceCode[m_sourcePos]) || m_pSourceCode[m_sourcePos] == L'.')
			{
				m_token.value += m_pSourceCode[m_sourcePos];
				m_sourcePos++;
				m_posInLine++;
			}
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
		else if (m_token.value == L"while")
			m_token.type = TT_WHILE;
		else if (m_token.value == L"break")
			m_token.type = TT_BREAK;
		else if (m_token.value == L"continue")
			m_token.type = TT_CONTINUE;
		else if (m_token.value == L"return")
			m_token.type = TT_RETURN;
		else if (m_token.value == L"if")
			m_token.type = TT_IF;
		else if (m_token.value == L"else")
			m_token.type = TT_ELSE;
		else if (m_token.value == L"this")
			m_token.type = TT_THIS;
		else if (m_token.value == L"new")
			m_token.type = TT_NEW;
		else if (m_token.value == L"true")
			m_token.type = TT_TRUE;
		else if (m_token.value == L"false")
			m_token.type = TT_FALSE;
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
		else if (m_pSourceCode[m_sourcePos] == L'|' && m_pSourceCode[m_sourcePos + 1] == L'=')
		{
			m_token.type = TT_OREQUAL;
			m_token.value = L"|=";
			m_sourcePos += 2;
			m_posInLine += 2;
		}
		else if (m_pSourceCode[m_sourcePos] == L'&' && m_pSourceCode[m_sourcePos + 1] == L'=')
		{
			m_token.type = TT_ANDEQUAL;
			m_token.value = L"&=";
			m_sourcePos += 2;
			m_posInLine += 2;
		}
		else if (m_pSourceCode[m_sourcePos] == L'=' && m_pSourceCode[m_sourcePos + 1] == L'=')
		{
			m_token.type = TT_EQUALEQUAL;
			m_token.value = L"==";
			m_sourcePos += 2;
			m_posInLine += 2;
		}
		else if (m_pSourceCode[m_sourcePos] == L'&' && m_pSourceCode[m_sourcePos + 1] == L'&')
		{
			m_token.type = TT_ANDAND;
			m_token.value = L"&&";
			m_sourcePos += 2;
			m_posInLine += 2;
		}
		else if (m_pSourceCode[m_sourcePos] == L'|' && m_pSourceCode[m_sourcePos + 1] == L'|')
		{
			m_token.type = TT_OROR;
			m_token.value = L"||";
			m_sourcePos += 2;
			m_posInLine += 2;
		}
		else if (m_pSourceCode[m_sourcePos] == L'<' && m_pSourceCode[m_sourcePos + 1] == L'=')
		{
			m_token.type = TT_LESSEQUAL;
			m_token.value = L"<=";
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

std::unique_ptr<Expression> ESInterpreter::parseExpressionBase(std::vector<int> separators, Expression::EXPRESSIONTYPE newType, std::function<std::unique_ptr<Expression>()> nextParser)
{
	auto pExpression = nextParser();

	if (std::find(separators.begin(), separators.end(), m_token.type) == separators.end())
		return pExpression;

	auto pNewExpression = std::make_unique<Expression>(this, newType);
	pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
	pNewExpression->m_expressionSets.back()->expression = std::move(pExpression);

	do
	{
		pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pNewExpression->m_expressionSets.back()->token = m_token;
		getNextToken();
		pNewExpression->m_expressionSets.back()->expression = nextParser();
	}
	while (std::find(separators.begin(), separators.end(), m_token.type) != separators.end());

	return pNewExpression;
}

std::unique_ptr<Expression> ESInterpreter::parsePrimaryExpression(TOKENTYPE requested)
{
	if (requested != TT_UNDEFINED && m_token.type != requested)
		throw ESException(ESException::R_TYPEERROR, m_token.value.c_str(), m_token.m_line, m_token.m_posInLine);

	if (m_token.type == TT_NUMBER)
	{
		auto pExpression = std::make_unique<Expression>(this, Expression::ET_NUMBER);
		pExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pExpression->m_expressionSets.back()->token = m_token;
		if (m_token.value[0] == L'x')
			pExpression->m_expressionSets.back()->numberValue = ::wcstol(m_token.value.c_str() + 1, NULL, 16);
		else
			pExpression->m_expressionSets.back()->numberValue = ::_wtof(m_token.value.c_str());
		getNextToken();

		return pExpression;
	}
	else if (m_token.type == TT_STRING)
	{
		auto pExpression = std::make_unique<Expression>(this, Expression::ET_STRING);
		pExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pExpression->m_expressionSets.back()->token = m_token;
		getNextToken();

		return pExpression;
	}
	else if (m_token.type == TT_IDENTIFIER)
	{
		auto pExpression = std::make_unique<Expression>(this, Expression::ET_IDENTIFIER);
		pExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pExpression->m_expressionSets.back()->token = m_token;
		getNextToken();

		return pExpression;
	}
	else if (getNextToken(TT_THIS))
	{
		return std::make_unique<Expression>(this, Expression::ET_THIS);
	}
	else if (m_token.type == TT_TRUE)
	{
		auto pExpression = std::make_unique<Expression>(this, Expression::ET_BOOLEAN);
		pExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pExpression->m_expressionSets.back()->token = m_token;
		getNextToken();

		return pExpression;
	}
	else if (m_token.type == TT_FALSE)
	{
		auto pExpression = std::make_unique<Expression>(this, Expression::ET_BOOLEAN);
		pExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
		pExpression->m_expressionSets.back()->token = m_token;
		getNextToken();

		return pExpression;
	}
	else if (getNextToken(L'('))
	{
		auto pExpression = parseExpression();
		getNextToken(L')', true);
		
		return pExpression;
	}
	else if (getNextToken(L'{'))
	{
		auto pExpression = std::make_unique<Expression>(this, Expression::ET_OBJECT);

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
	else if (getNextToken(L'['))
	{
		auto pExpression = std::make_unique<Expression>(this, Expression::ET_ARRAY);

		if (!getNextToken(L']'))
		{
			do
			{
				pExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
				if (m_token.type != (L','))
					pExpression->m_expressionSets.back()->expression = parseAssignmentExpression();
			} while (getNextToken(L','));
			getNextToken(L']', true);
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
		auto pFunctionExpression = std::make_unique<FunctionExpression>(this);
		pFunctionExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());

		getNextToken(L'(', true);
		if (m_token.type != L')')
		{
			do
			{
				pFunctionExpression->m_expressionSets.back()->arguments.push_back(parsePrimaryExpression(TT_IDENTIFIER));
				pFunctionExpression->m_pVariableEnvironment->setVariable(pFunctionExpression->m_expressionSets.back()->arguments.back()->m_expressionSets[0]->token.value.c_str(), Value());
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
		pExpression = std::make_unique<Expression>(this, Expression::ET_NEW);

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

	auto pNewExpression = std::make_unique<Expression>(this, Expression::ET_LEFTHADSIDE);
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
		pNewExpression = std::move(std::make_unique<Expression>(this, Expression::ET_LEFTHADSIDE));
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

	auto pNewExpression = std::make_unique<Expression>(this, Expression::ET_POSTFIX);

	pNewExpression->m_expressionSets.push_back(std::make_unique<Expression::EXPRESSIONSET>());
	pNewExpression->m_expressionSets.back()->expression = std::move(pExpression);
	pNewExpression->m_expressionSets.back()->token = m_token;
	getNextToken();

	return pNewExpression;
}

std::unique_ptr<Expression> ESInterpreter::parseUnaryExpression()
{
	if (m_token.type == L'-' || m_token.type == L'!' || m_token.type == L'~')
	{
		auto pExpression = std::make_unique<Expression>(this, Expression::ET_UNARY);

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
	return parseExpressionBase({ L'*', L'/', L'%' }, Expression::ET_MULTIPLICATIVE, std::bind(&ESInterpreter::parseUnaryExpression, this));
}

std::unique_ptr<Expression> ESInterpreter::parseAdditiveExpression()
{
	return parseExpressionBase({ L'+', L'-' }, Expression::ET_ADDITIVE, std::bind(&ESInterpreter::parseMultiplicativeExpression, this));
}

std::unique_ptr<Expression> ESInterpreter::parseShiftExpression()
{
	return parseAdditiveExpression();
}

std::unique_ptr<Expression> ESInterpreter::parseRelationalExpression()
{
	return parseExpressionBase({ L'<', TT_LESSEQUAL }, Expression::ET_RELATIONAL, std::bind(&ESInterpreter::parseShiftExpression, this));
}

std::unique_ptr<Expression> ESInterpreter::parseEqualityExpression()
{
	return parseExpressionBase({ TT_EQUALEQUAL }, Expression::ET_EQUALITY, std::bind(&ESInterpreter::parseRelationalExpression, this));
}

std::unique_ptr<Expression> ESInterpreter::parseBinaryBitwiseExpression()
{
	return parseExpressionBase({ L'|' }, Expression::ET_BITWISE, [&]()
	{
		return parseExpressionBase({ L'^' }, Expression::ET_BITWISE, [&]()
		{
			return parseExpressionBase({ L'&' }, Expression::ET_BITWISE, std::bind(&ESInterpreter::parseEqualityExpression, this));
		});
	});
}

std::unique_ptr<Expression> ESInterpreter::parseBinaryLogicalExpression()
{
	return parseExpressionBase({ TT_OROR }, Expression::ET_BINARYLOGICAL, [&]()
	{
		return parseExpressionBase({ TT_ANDAND }, Expression::ET_BINARYLOGICAL, std::bind(&ESInterpreter::parseBinaryBitwiseExpression, this));
	});
}

std::unique_ptr<Expression> ESInterpreter::parseConditionalExpression()
{
	return parseBinaryLogicalExpression();
}

std::unique_ptr<Expression> ESInterpreter::parseAssignmentExpression()
{
	auto pExpression = parseConditionalExpression();
	if (Expression::ET_LEFTHADSIDE < pExpression->m_type || (m_token.type != L'=' && m_token.type != TT_PLUSEQUAL && m_token.type != TT_OREQUAL && m_token.type != TT_ANDEQUAL))
		return pExpression;

	auto pNewExpression = std::make_unique<Expression>(this, Expression::ET_ASSIGNMENT);

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
			auto pStatement = std::make_unique<Statement>(this, Statement::ST_BLOCK, m_line, m_posInLine, m_pCallback);
			while (m_token.type != TT_EOF && m_token.type != L'}')
			{
				pStatement->m_statements.push_back(parseStatement(pVariableEnvironment));
			}
			getNextToken(L'}', true);

			return pStatement;
		}
		else if (getNextToken(TT_VAR))
		{
			auto pStatement = std::make_unique<Statement>(this, Statement::ST_VAR, m_line, m_posInLine, m_pCallback);

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
			auto pStatement = std::make_unique<Statement>(this, Statement::ST_FOR, m_line, m_posInLine, m_pCallback);

			getNextToken(L'(', true);
			if (m_token.type == TT_VAR)
			{
				pStatement->m_statements.push_back(parseStatement(pVariableEnvironment));
				pStatement->m_expressions.push_back(nullptr);
			}
			else
			{
				pStatement->m_statements.push_back(nullptr);
				pStatement->m_expressions.push_back(m_token.type == L';' ? nullptr : parseExpression());
				getNextToken(L';', true);
			}
			pStatement->m_expressions.push_back(m_token.type == L';' ? nullptr : parseExpression());
			getNextToken(L';', true);
			pStatement->m_expressions.push_back(m_token.type == L')' ? nullptr : parseExpression());
			getNextToken(L')', true);

			pStatement->m_statements.push_back(parseStatement(pVariableEnvironment));

			return pStatement;
		}
		else if (getNextToken(TT_WHILE))
		{
			auto pStatement = std::make_unique <Statement>(this, Statement::ST_WHILE, m_line, m_posInLine, m_pCallback);

			getNextToken(L'(', true);
			pStatement->m_expressions.push_back(parseExpression());
			getNextToken(L')', true);

			pStatement->m_statements.push_back(parseStatement(pVariableEnvironment));

			return pStatement;

		}
		else if (getNextToken(TT_BREAK))
		{
			getNextToken(L';');
			return std::make_unique<Statement>(this, Statement::ST_BREAK, m_line, m_posInLine, m_pCallback);
		}
		else if (getNextToken(TT_CONTINUE))
		{
			getNextToken(L';');
			return std::make_unique<Statement>(this, Statement::ST_CONTINUE, m_line, m_posInLine, m_pCallback);
		}
		else if (getNextToken(TT_RETURN))
		{
			auto pStatement = std::make_unique<Statement>(this, Statement::ST_RETURN, m_line, m_posInLine, m_pCallback);
			if (m_token.type != L';')	// todo no line terminate
			{
				pStatement->m_expressions.push_back(parseExpression());
			}
			getNextToken(L';');
			return pStatement;
		}
		else if (getNextToken(TT_IF))
		{
			auto pStatement = std::make_unique<Statement>(this, Statement::ST_IF, m_line, m_posInLine, m_pCallback);

			getNextToken(L'(', true);
			pStatement->m_expressions.push_back(parseExpression());
			getNextToken(L')', true);

			pStatement->m_statements.push_back(parseStatement(pVariableEnvironment));

			if (getNextToken(TT_ELSE))
				pStatement->m_statements.push_back(parseStatement(pVariableEnvironment));

			return pStatement;
		}

		auto pStatement = std::make_unique<Statement>(this, Statement::ST_EXPRESSION, m_line, m_posInLine, m_pCallback);
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
	auto pStatement = std::make_unique<Statement>(this, Statement::ST_SOURCEELEMENT, m_line, m_posInLine, m_pCallback);
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
		return parseSourceElements(m_pGlobalObject);
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
	m_programs.back()->run(m_pGlobalObject, m_pGlobalObject);

	return m_programs.back()->m_result.value;
}

void ESInterpreter::sweep()
{
	for (auto it = m_roots.begin(); it != m_roots.end(); it++)
	{
		(*it)->mark();
	}

	for (auto it = m_objects.begin(); it != m_objects.end();)
	{
		if ((*it)->m_marked)
		{
			(*it)->m_marked = false;
			it++;
		}
		else
		{
			it = m_objects.erase(it);
		}
	}
}

Object* ESInterpreter::getGlobalObject()
{
	return m_pGlobalObject;
}

Value ESInterpreter::parseInt(Object *pThis, std::vector<Value> arguments, void *pUserParam)
{
	if (1 <= arguments.size())
		return (double)arguments[0].toInt32();
	return Value();
}

void ESInterpreter::createStandardObject()
{
	m_pStandardObject = createObject();
	m_pStandardObject->m_pNativeCall = [](Object *pThis, std::vector<Value>&, void*)
	{
		// todo
		return Value();
	};
	m_pStandardObject->m_pNativeConstructor = [](Object *pThis, std::vector<Value>&, void*)
	{
		// todo
		return Value();
	};
	m_pStandardObject->m_class = L"Object";

	m_pStandardObjectPrototype = createObject();
	m_pStandardObject->setVariable(L"prototype", (Object*)m_pStandardObjectPrototype);

	m_pStandardObjectPrototype->setVariable(L"toString", createFunctionObject([](Object *pThis, std::vector<Value>&, void*)
	{
		return Value((L"[object " + pThis->m_class + L"]").c_str());
	}, nullptr, nullptr));
}

void ESInterpreter::createStandardFunctionObject()
{
	m_pStandardFunctionObject = createObject();
	m_pStandardFunctionObject->m_pNativeCall = [](Object *pThis, std::vector<Value>&, void*)
	{
		// todo
		return Value();
	};
	m_pStandardFunctionObject->m_pNativeConstructor = [](Object *pThis, std::vector<Value>&, void*)
	{
		// todo
		return Value();
	};
	m_pStandardFunctionObject->m_class = L"Function";

	m_pStandardFunctionObjectPrototype = createObject();
	m_pStandardFunctionObject->setVariable(L"prototype", (Object*)m_pStandardFunctionObjectPrototype);
}

Object* ESInterpreter::createObject()
{
	sweep();	// todo

	std::unique_ptr<Object> pTempObject(new Object(this));
	m_objects.push_back(std::move(pTempObject));

	ObjectPtr pObject = m_objects.back().get();
	pObject->m_class = L"Object";
	pObject->m_pPrototype = m_pStandardObjectPrototype;
	return pObject;
}

Object* ESInterpreter::createFunctionObject()
{
	ObjectPtr pObject = createObject();
	pObject->m_class = L"Function";
	pObject->m_pPrototype = m_pStandardFunctionObjectPrototype;
	pObject->setVariable(L"prototype", createObject());
	return pObject;
}

Object* ESInterpreter::createFunctionObject(std::function<Value(Object*, std::vector<Value>, void*)> pNativeCall, std::function<Value(Object*, std::vector<Value>, void*)> pNativeConstructor, void *pUserParam)
{
	ObjectPtr pObject = createFunctionObject();
	pObject->m_pNativeCall = pNativeCall;
	pObject->m_pNativeConstructor = pNativeConstructor;
	pObject->m_pUserParam = pUserParam;
	return pObject;
}

Object* ESInterpreter::createFunctionObject(FunctionExpression *pExpression, Object *pScope)
{
	ObjectPtr pObject = createFunctionObject();
	pObject->m_pFunctionBody = pExpression;
	pObject->m_pScope = pScope;
	return pObject;
}
