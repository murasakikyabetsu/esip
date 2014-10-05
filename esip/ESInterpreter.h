#pragma once

#include <string>
#include <memory>
#include <vector>
#include <list>
#include <unordered_map>
#include <functional>
#include <map>

typedef struct tagTOKEN
{
	int type;
	std::wstring value;

	int m_line;
	int m_posInLine;

} TOKEN;

class ESException
{
public:

	enum REASON
	{
		R_SYNTAXERROR,
		R_REFERENCEERROR,
		R_TYPEERROR,
	};

public:

	REASON m_reason;
	std::wstring m_information;

	int m_line;
	int m_posInLine;

public:

	ESException(REASON reason, const wchar_t *pInformation = nullptr, int line = 0, int posInLine = 0);
	virtual ~ESException();
};

class Object;

class ObjectPtr
{
private:

	Object *m_pObject;

public:

	ObjectPtr();
	ObjectPtr(Object *pObject);
	ObjectPtr(const ObjectPtr &pObject);
	virtual ~ObjectPtr();

	ObjectPtr& operator=(const ObjectPtr &pObject);
	ObjectPtr& operator=(Object *pObject);
	Object* operator->();
	operator bool();
	operator Object*();
};

class Value
{
public:

	enum VALUETYPE
	{
		VT_INVALID,
		VT_UNDEFINED,
		VT_NUMBER,
		VT_BOOLEAN,
		VT_STRING,
		VT_OBJECT,
	};

public:

	VALUETYPE m_type;

	Object *m_pBase;
	std::wstring m_referenceName;

private:

	union
	{
		double number;
		bool boolean;
		Object *pObject;

	} m_value;
	std::wstring m_stringValue;

public:

	Value();
	Value(double value);
	Value(bool value);
	Value(const wchar_t *pValue);
	Value(Object *pValue);
	Value(const Value &value);
	Value(Value && value);
	virtual ~Value();

	Value& operator=(double value);
	Value& operator=(bool value);
	Value& operator=(const wchar_t *pValue);
	Value& operator=(Object *pValue);
	Value& operator=(const Value &value);
	Value& operator=(Value && value);

	double toNumber() const;
	int toInt32() const;
	bool toBoolean() const;
	std::wstring toString() const;
	Object* toObject() const;

	bool isCallable() const;

	Value operator+(const Value &value) const;
	Value operator-(const Value &value) const;
	Value operator*(const Value &value) const;
	Value operator/(const Value &value) const;
	Value operator%(const Value &value) const;
	Value operator|(const Value &value) const;
	Value operator&(const Value &value) const;
	Value operator<(const Value &value) const;
	Value operator==(const Value &value) const;

};

class FunctionExpression;

class Object
{
public:

	static std::list<std::unique_ptr<Object>> m_objects;
	static std::list<Object*> m_roots;
	static Object* create();

private:

	std::function<bool(const wchar_t *pName, const Value &value, void*)> m_pSetVariable;
	std::function<bool(const wchar_t *pName, Value &value, void*)> m_pGetVariable;
	std::function<void(void*)> m_pDestroy;

public:

	std::unordered_map<std::wstring, Value> m_variable;

	void *m_pUserParam;

	std::wstring m_class;	// [[Class]] - Object
	ObjectPtr m_pPrototype;	// [[Prototype]] - Object

	std::function<Value(Object*, std::vector<Value>, void*)> m_pNativeCall;
	std::function<Value(Object*, std::vector<Value>, void*)> m_pNativeConstructor;

	FunctionExpression *m_pFunctionBody;											// [[Code]] - Function Object
	ObjectPtr m_pScope;																// [[Scope]] - Function Object

	bool m_marked;

private:

	Object();

public:

	virtual ~Object();

	Object& operator=(const Object &obj);

	Value call(Object *pThis, std::vector<Value> arguments, bool isConstruct);	// [[Call]] - Function Object
	Value construct(std::vector<Value> arguments);								// [[Construct]] - Function Object

	void setVariable(const wchar_t *pName, Value value);
	Value getVariable(const wchar_t *pName, bool isThis);

	void setCapture(std::function<bool(const wchar_t *pName, const Value &value, void*)> pSetVariable, std::function<bool(const wchar_t *pName, Value &value, void*)> pGetVariable, std::function<void(void*)> pDestroy, void *pUserParam);

	void mark();
};

class ESInterpreter;

class Expression
{
public:

	enum EXPRESSIONTYPE
	{
		ET_NUMBER,
		ET_STRING,
		ET_IDENTIFIER,
		ET_THIS,
		ET_BOOLEAN,
		ET_OBJECT,
		ET_ARRAY,
		ET_NEW,
		ET_LEFTHADSIDE,
		ET_FUNCTION,
		ET_POSTFIX,
		ET_UNARY,
		ET_MULTIPLICATIVE,
		ET_ADDITIVE,
		ET_RELATIONAL,
		ET_EQUALITY,
		ET_BITWISE,
		ET_BINARYLOGICAL,
		ET_ASSIGNMENT,
	};

protected:

	ESInterpreter *m_pInterpreter;

public:

	EXPRESSIONTYPE m_type;

	typedef struct tagEXPRESSIONSET
	{
		std::unique_ptr<Expression> expression;
		TOKEN token;
		double numberValue;
		std::vector<std::unique_ptr<Expression>> arguments;
	} EXPRESSIONSET;
	std::vector<std::unique_ptr<EXPRESSIONSET>> m_expressionSets;

public:

	Expression(ESInterpreter *pInterpeter, EXPRESSIONTYPE type);
	virtual ~Expression();

	virtual Value run(Object *pScope, Object *pThis);

};

class Statement;

class FunctionExpression : public Expression
{
public:

	std::unique_ptr<Statement> m_pFunctionBody;

	ObjectPtr m_pVariableEnvironment;

public:

	FunctionExpression(ESInterpreter *pInterpeter);
	virtual ~FunctionExpression();

	virtual Value run(Object *pScope, Object *pThis);

};

class Statement
{
public:

	enum STATEMENTTYPE
	{
		ST_BLOCK,
		ST_VAR,
		ST_FOR,
		ST_WHILE,
		ST_BREAK,
		ST_CONTINUE,
		ST_RETURN,
		ST_IF,
		ST_EXPRESSION,
		ST_SOURCEELEMENT,
	};

	enum RESULTTYPE
	{
		RT_NORMAL,
		RT_BREAK,
		RT_CONTINUE,
		RT_RETURN,
	};

private:

	ESInterpreter *m_pInterpreter;
	STATEMENTTYPE m_type;

	void (*m_pCallback)(int, int);
	int m_line;
	int m_posInLine;

public:

	std::vector<std::unique_ptr<Statement>> m_statements;
	std::vector<std::unique_ptr<Expression>> m_expressions;

	struct
	{
		RESULTTYPE type;
		Value value;
	} m_result;

public:

	Statement(ESInterpreter *pInterpeter, STATEMENTTYPE type, int line, int posInLine, void(*pCallback)(int, int));
	virtual ~Statement();

	void run(Object *pScope, Object *pThis);

};

class NativeObject
{
protected:

	ESInterpreter *m_pInterpreter;

public:

	static Value call(ESInterpreter *pInterpreter, Object *pThis, std::vector<Value> arguments)
	{
		// todo オブジェクトを作ってコンストラクタを呼ぶ

		return Value();
	}

public:

	NativeObject(ESInterpreter *pInterpreter) : m_pInterpreter(pInterpreter){}
	virtual ~NativeObject() {}

	virtual Value constructor(Object *pThis, std::vector<Value> arguments)
	{
		return Value();
	}

	virtual bool setVariable(Object *pThis, const wchar_t *pName, const Value &value)
	{
		return false;
	}

	virtual bool getVariable(Object *pThis, const wchar_t *pName, Value &value)
	{
		return false;
	}
};

class ESInterpreter
{
public:

	enum TOKENTYPE
	{
		TT_UNDEFINED = -1,
		TT_EOF = 0,

		TT_NUMBER = 256,
		TT_STRING,
		TT_IDENTIFIER,
		TT_TRUE,
		TT_FALSE,

		TT_PLUSPLUS,
		TT_MINUSMINUS,
		TT_PLUSEQUAL,
		TT_OREQUAL,
		TT_ANDEQUAL,
		TT_EQUALEQUAL,
		TT_ANDAND,
		TT_OROR,
		TT_LESSEQUAL,

		TT_FUNCTION,
		TT_VAR,
		TT_FOR,
		TT_WHILE,
		TT_BREAK,
		TT_CONTINUE,
		TT_RETURN,
		TT_IF,
		TT_ELSE,
		TT_THIS,
		TT_NEW,
	};

private:

	ObjectPtr m_pGlobalObject;

	const wchar_t *m_pSourceCode;
	int m_sourcePos;
	int m_line;
	int m_posInLine;

	void (*m_pCallback)(int, int);

	TOKEN m_token;

	std::vector<std::unique_ptr<Statement>> m_programs;

	ObjectPtr m_pStandardObject;
	ObjectPtr m_pStandardObjectPrototype;

	ObjectPtr m_pStandardFunctionObject;
	ObjectPtr m_pStandardFunctionObjectPrototype;

private:

	bool getNextToken(int type = TT_UNDEFINED, bool exception = false);

	std::unique_ptr<Expression> parseExpressionBase(std::vector<int> separators, Expression::EXPRESSIONTYPE newType, std::function<std::unique_ptr<Expression>()> nextParser);

	std::unique_ptr<Expression> parsePrimaryExpression(TOKENTYPE requested = TT_UNDEFINED);
	std::unique_ptr<Expression> parseMemberExpression();
	std::unique_ptr<Expression> parseLeftHandSideExpression();
	std::unique_ptr<Expression> parsePostfixExpression();
	std::unique_ptr<Expression> parseUnaryExpression();
	std::unique_ptr<Expression> parseMultiplicativeExpression();
	std::unique_ptr<Expression> parseAdditiveExpression();
	std::unique_ptr<Expression> parseShiftExpression();
	std::unique_ptr<Expression> parseRelationalExpression();
	std::unique_ptr<Expression> parseEqualityExpression();
	std::unique_ptr<Expression> parseBinaryBitwiseExpression();
	std::unique_ptr<Expression> parseBinaryLogicalExpression();
	std::unique_ptr<Expression> parseConditionalExpression();
	std::unique_ptr<Expression> parseAssignmentExpression();
	std::unique_ptr<Expression> parseExpression();

	std::unique_ptr<Statement> parseStatement(Object *pVariableEnvironment);
	std::unique_ptr<Statement> parseSourceElements(Object *pVariableEnvironment);
	std::unique_ptr<Statement> parseProgram();

	void createStandardObject();
	void createStandardFunctionObject();

	Object* createFunctionObject();

	static Value parseInt(Object *pThis, std::vector<Value> arguments, void *pUserParam);

public:

	ESInterpreter(void (*pCallback)(int, int) = NULL);
	virtual ~ESInterpreter();

	Value run(const wchar_t *pSourceCode);

	void sweep();

	Object* getGlobalObject();
	Object* createObject();
	Object* createFunctionObject(std::function<Value(Object*, std::vector<Value>, void*)> pNativeCall, std::function<Value(Object*, std::vector<Value>, void*)> pNativeConstructor, void *pUserParam);
	Object* createFunctionObject(FunctionExpression *pExpression, Object *pScope);

	template <class T>
	Object* createNativeObject(
		std::wstring className,
		std::map<std::wstring, Value(*)(ESInterpreter*, Object*, std::vector<Value>)> functions,
		std::map<std::wstring, Value(T::*)(Object*, std::vector<Value>)> prototypeFunctions,
		Value(T::*pConstructor)(Object*, std::vector<Value>) = &T::constructor,
		Value(*pCall)(ESInterpreter*, Object*, std::vector<Value>) = &NativeObject::call)
	{
		ObjectPtr pObject = createFunctionObject([=](Object *pThis, std::vector<Value> arguments, void *pUserParam)
		{
			ESInterpreter *pInterpreter = (ESInterpreter*)pUserParam;

			return pCall(pInterpreter, pThis, arguments);

		}, [=](Object *pThis, std::vector<Value>& arguments, void *pUserParam)
		{
			ESInterpreter *pInterpreter = (ESInterpreter*)pUserParam;

			T *p = new T(pInterpreter);

			pThis->setCapture([=](const wchar_t *pName, const Value &value, void *pUserParam)
			{
				T *p = static_cast<T*>(pUserParam);
				return p->setVariable(pThis, pName, value);
			}, [=](const wchar_t *pName, Value &value, void *pUserParam)
			{
				T *p = static_cast<T*>(pUserParam);
				return p->getVariable(pThis, pName, value);
			}, [](void *pUserParam)
			{
				T *p = static_cast<T*>(pUserParam);
				delete p;
			}, p);
			pThis->m_class = className;

			return (p->*pConstructor)(pThis, arguments);

		}, this);

		pObject->m_class = className;

		for (const auto &v : functions)
		{
			pObject->setVariable(v.first.c_str(), createFunctionObject([=](Object *pThis, std::vector<Value> arguments, void *pUserParam)
			{
				ESInterpreter *p = static_cast<ESInterpreter*>(pUserParam);
				return v.second(p, pThis, arguments);
			}, nullptr, this));
		}

		ObjectPtr pPrototype = createObject();
		pObject->setVariable(L"prototype", (Object*)pPrototype);

		for (const auto &v : prototypeFunctions)
		{
			pPrototype->setVariable(v.first.c_str(), createFunctionObject([=](Object *pThis, std::vector<Value> arguments, void *pUserParam)
			{
				T *p = static_cast<T*>(pThis->m_pUserParam);

				if (pThis->m_class != className || !p)
					throw ESException(ESException::R_TYPEERROR, (L"not " + className).c_str());

				return (p->*v.second)(pThis, arguments);
			}, nullptr, nullptr));
		}

		return pObject;
	}
};
