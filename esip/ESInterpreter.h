#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>

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

	double m_numberValue;
	bool m_booleanValue;
	std::wstring m_stringValue;
	Object *m_pObjectValue;

public:

	Value();
	Value(double value);
	Value(bool value);
	Value(const wchar_t *pValue);
	Value(Object *pValue);
	virtual ~Value();

	double toNumber() const;
	bool toBoolean() const;
	std::wstring toString() const;
	Object* toObject() const;

	Value add(const Value &value) const;
	Value sub(const Value &value) const;
	Value mul(const Value &value) const;
	Value div(const Value &value) const;
	Value mod(const Value &value) const;
	Value lessThan(const Value &value) const;

};

class FunctionExpression;

class Object
{
public:

	static std::vector<std::unique_ptr<Object>> m_objects;
	static Object* create();
	static Object* create(std::function<Value(Object*, std::vector<Value>&, void*)> pNativeFunction, void *pUserParam);

public:

	std::map<std::wstring, Value> m_variable;

	std::function<Value(Object*, std::vector<Value>&, void*)> m_pNativeFunction;
	std::function<bool(const wchar_t *pName, const Value &value, void*)> m_pSetVariable;
	std::function<bool(const wchar_t *pName, Value &value, void*)> m_pGetVariable;
	std::function<void(void*)> m_pDestroy;
	void *m_pUserParam;

	FunctionExpression *m_pFunctionExpression;
	bool m_callable;

	Object *m_pParentScope;

private:

	Object();

public:

	Object& operator=(const Object &obj);

	virtual ~Object();

	void setVariable(const wchar_t *pName, Value value);
	Value getVariable(const wchar_t *pName, bool isThis);
	void setNativeFunction(const wchar_t *pName, std::function<Value(Object*, std::vector<Value>&, void*)> pNativeFunction, void *pUserParam);

	void setCapture(std::function<bool(const wchar_t *pName, const Value &value, void*)> pSetVariable, std::function<bool(const wchar_t *pName, Value &value, void*)> pGetVariable, std::function<void(void*)> pDestroy, void *pUserParam);
};

class Expression
{
public:

	enum EXPRESSIONTYPE
	{
		ET_NUMBER,
		ET_STRING,
		ET_IDENTIFIER,
		ET_THIS,
		ET_NEW,
		ET_LEFTHADSIDE,
		ET_FUNCTION,
		ET_POSTFIX,
		ET_UNARY,
		ET_MULTIPLICATIVE,
		ET_ADDITIVE,
		ET_RELATIONAL,
		ET_ASSIGNMENT,
	};

public:

	EXPRESSIONTYPE m_type;

	typedef struct tagEXPRESSIONSET
	{
		std::unique_ptr<Expression> expression;
		TOKEN token;
		std::vector<std::unique_ptr<Expression>> arguments;
	} EXPRESSIONSET;
	std::vector<std::unique_ptr<EXPRESSIONSET>> m_expressionSets;

public:

	Expression(EXPRESSIONTYPE type);
	virtual ~Expression();

	virtual Value run(Object *pScope, Object *pThis);
	virtual Value call(Object *pScope, Object *pThis, Value &func, EXPRESSIONSET *pSet, Object *pNewThis);

};

class Statement;

class FunctionExpression : public Expression
{
public:

	std::unique_ptr<Statement> m_pFunctionBody;

	Object *m_pVariableEnvironment;

public:

	FunctionExpression();
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
		ST_BREAK,
		ST_IF,
		ST_EXPRESSION,
		ST_SOURCEELEMENT,
	};

	enum RESULTTYPE
	{
		RT_NORMAL,
		RT_BREAK,
	};

private:

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

	Statement(STATEMENTTYPE type, int line, int posInLine, void(*pCallback)(int,int));
	virtual ~Statement();

	void run(Object *pScope, Object *pThis);

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

		TT_PLUSPLUS,
		TT_MINUSMINUS,
		TT_PLUSEQUAL,

		TT_FUNCTION,
		TT_VAR,
		TT_FOR,
		TT_BREAK,
		TT_IF,
		TT_ELSE,
		TT_THIS,
		TT_NEW,
	};

private:

	Object *m_pRoot;
	const wchar_t *m_pSourceCode;
	int m_sourcePos;
	int m_line;
	int m_posInLine;

	void (*m_pCallback)(int, int);

	TOKEN m_token;

	std::vector<std::unique_ptr<Statement>> m_programs;

private:

	bool getNextToken(int type = TT_UNDEFINED, bool exception = false);

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

public:

	ESInterpreter(Object *pRoot, void (*pCallback)(int, int) = NULL);
	virtual ~ESInterpreter();

	Value run(const wchar_t *pSourceCode);

};
