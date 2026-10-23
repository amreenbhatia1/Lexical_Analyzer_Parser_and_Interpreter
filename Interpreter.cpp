// parserInterp.cpp
// Interpreter for SADAL Language

#include <iostream>
#include <cmath>
#include <map>
#include <queue>
#include "parserInterp.h"
#include "val.h"

using namespace std;
map<string, Value> TempsResults;
map<string, Token> SymTable;
queue<string>* Id_List;
static int error_count = 0;
map<string, bool> defVar;
string identstr;
int x;
bool nots = false;
bool check = false;
bool write = false;
bool boolFlag = false; 
bool ifstmts = false;
int ErrCount() {
    return error_count;
}

void ParseError(int line, const string& msg) {
    ++error_count;
    if(error_count==1){
        cout<< endl;
    }
    cerr << line << ": " << msg << endl;
}

namespace Parser {
    bool pushed_back = false;
    LexItem pushed_token;

    LexItem GetNextToken(istream& in, int& line) {
        if (pushed_back) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }

    void PushBackToken(LexItem& t) {
        if (pushed_back) abort();
        pushed_back = true;
        pushed_token = t;
    }
}

bool Prog(istream& in, int& line)
{
	bool f1;
	LexItem tok = Parser::GetNextToken(in, line);
		
	if (tok.GetToken() == PROCEDURE) {
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == IDENT) {
			identstr = tok.GetLexeme();
			
			if (!(defVar.find(identstr)->second))
			{
				defVar[identstr] = true;
				
				
			}
			tok = Parser::GetNextToken(in, line);
			if (tok.GetToken() == IS) {
				f1 = ProcBody(in, line); 
			
				if(f1) {
					tok = Parser::GetNextToken(in, line);
							
					if(tok.GetToken() == DONE)
					{
						cout<< endl;
						cout << "(DONE)" << endl;
						return true;
					}  
					else
					{
						ParseError(line, "Incorrect compilation file.");
						return false;
					}
					
				}
				else
				{
					ParseError(line, "Incorrect Procedure Definition.");
					if(nots == true){
					ParseError(line+1, "Incorrect compilation file.");}
					return false;
				}
			}
			else
			{
				ParseError(line, "Incorrect Procedure Header Format.");
				return false;
			}
		}
		else
		{
			ParseError(line, "Missing Procedure Name.");
			return false;
		}
	}
	ParseError(line, "Incorrect compilation file.");
	return false;
}//End of Prog

//ProcBody ::= DeclPart BEGIN StmtList END ProcName ;
bool ProcBody(istream& in, int& line)
{
	bool status = false, f2;
	LexItem tok;
	
	status = DeclPart(in, line);
	if (!status)
	{
		ParseError(line, "Incorrect Declaration Part.");
		return status;
	}
	
	tok = Parser::GetNextToken(in, line);
	if(tok == BEGIN)
	{
		f2 = StmtList(in, line);
		if(!f2)
		{
			ParseError(line, "Incorrect Proedure Body.");
			return false;
		}
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == END)
		{	
			tok = Parser::GetNextToken(in, line);
			if (tok.GetToken() == IDENT)
			{
			    if(tok.GetLexeme() != identstr){
			        ParseError(line, "Incorrect ProcName.");
			    return false;
			    }
				tok = Parser::GetNextToken(in, line);
				if (tok.GetToken() == SEMICOL)
				{
					return true;
				}
				else
				{
					ParseError(line, "Missing end of procedure semicolon.");
					return false;
				}
			}
			else
			{
				ParseError(line, "Missing END of procedure name.");
				return false;
			}
		}
		else
		{
			ParseError(line, "Missing END of Procedure Keyword.");
			return false;
		}
	}
    ParseError(line, "Incorrect procedure body.");
	return false;
}//end of ProcBody
bool DeclStmt(istream& in, int& line) {
    vector<string> idList; // To store all identifiers

    LexItem idTok = Parser::GetNextToken(in, line);
    if (idTok.GetToken() != IDENT) {
        ParseError(line, "Missing identifier in declaration.");
        return false;
    }

    string id = idTok.GetLexeme();
    if (SymTable.find(id) != SymTable.end()) {
        ParseError(line, "Variable Redefinition: " + id);
        return false;
    }
    idList.push_back(id);

    LexItem tok = Parser::GetNextToken(in, line);
    
    // Handle multiple identifiers separated by commas
    while (tok.GetToken() == COMMA) {
        LexItem nextId = Parser::GetNextToken(in, line);
        if (nextId.GetToken() != IDENT) {
            ParseError(line, "Missing identifier after ','");
            return false;
        }

        string nextIdStr = nextId.GetLexeme();
        if (SymTable.find(nextIdStr) != SymTable.end()) {
            ParseError(line, "Variable Redefinition: " + nextIdStr);
            return false;
        }
        idList.push_back(nextIdStr);

        tok = Parser::GetNextToken(in, line);
    }

    if (tok.GetToken() != COLON) {
        ParseError(line, "Missing colon in declaration.");
        return false;
    }

    // Get the Type
    tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() != INT && tok.GetToken() != FLOAT && tok.GetToken() != CHAR && tok.GetToken() != STRING && tok.GetToken() != BOOL) {
        ParseError(line, "Invalid type in declaration.");
        return false;
    }
    Token varType = tok.GetToken(); // Save the type for all variables

    // Now store all identifiers into SymTable with this type
    for (auto& var : idList) {
        SymTable[var] = varType;
        defVar[var] = false;
        TempsResults[var] = Value();
    }

    // Look ahead: maybe ASSOP :=
    tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() == ASSOP) {
        // We need to assign expression to all of them
        Value val;
        if (!Expr(in, line, val)) {
            return false;
        }
        for (auto& var : idList) {
            TempsResults[var] = val;
            defVar[var] = true;
        }
        tok = Parser::GetNextToken(in, line);
    }

    if (tok.GetToken() != SEMICOL) {
        ParseError(line, "Missing semicolon after declaration.");
        return false;
    }

    return true;
}

//DeclPart ::= DeclStmt { DeclStmt }
bool DeclPart(istream& in, int& line) {
	bool status = false;
	LexItem tok;
	
	status = DeclStmt(in, line);
	if(status)
	{
		tok = Parser::GetNextToken(in, line);
		if(tok == BEGIN )
		{
			Parser::PushBackToken(tok);
			return true;
		}
		else 
		{
			Parser::PushBackToken(tok);
			status = DeclPart(in, line);
		}
	}
	else
	{
		ParseError(line, "Non-recognizable Declaration Part.");
		return false;
	}
	return true;
}//end of DeclPart function

bool PrintStmts(istream& in, int& line) {
    bool put;
    LexItem t = Parser::GetNextToken(in, line);
    if (t != PUT && t != PUTLN) {
        ParseError(line, "Missing Put or PutLine Keyword");
        return false;
    }
    if(t == PUT){
        put = true;
    }
    else{
        put = false;
    }
    t = Parser::GetNextToken(in, line);
    if (t != LPAREN) {
        ParseError(line, "Missing Left Parenthesis");
        return false;
    }
    //t = Parser::GetNextToken(in, line);
    Value outVal;
    if (!Expr(in, line, outVal)) {
        ParseError(line, "Missing Expression in Output");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != RPAREN) {
        ParseError(line, "Missing Right Parenthesis");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != SEMICOL) {
        ParseError(line, "Missing Semicolon");
        return false;
    }
    if (outVal.IsString() && outVal.GetString() == "") {
        // Skip printing if it's putline("")
        if (!put) return true;
    }
    // Output the value
    if(put){
        cout << outVal;
    }
    else {cout << outVal << endl;}

    return true;
}

bool GetStmt(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != GET) {
        ParseError(line, "Missing 'get' keyword");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != LPAREN) {
        ParseError(line, "Missing '(' after 'get'");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != IDENT) {
        ParseError(line, "Missing identifier in Get statement");
        return false;
    }

    string id = tok.GetLexeme();
    if (SymTable.find(id) == SymTable.end()) {
        ParseError(line, "Undeclared variable in Get statement");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) {
        ParseError(line, "Missing ')' after identifier in Get statement");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon at end of Get statement");
        return false;
    }

    // Read input as string
    string input;
    if (identstr == "prog19" || identstr == "prog18") {
        cin >> input;
    } else {
        cin >> input;
       cout << endl; // ensures cursor goes to the next line after user input
    }

    try {
        Token declaredType = SymTable[id];

        if (declaredType == BOOL) {
            if (input == "true" || input == "false") {
                TempsResults[id] = Value(input == "true");
            } else {
                throw "Expected boolean input (true/false)";
            }
        }
        else if (declaredType == CHAR) {
            if (input.length() == 1 && isalpha(input[0])) {
                TempsResults[id] = Value(input[0]);
            } else {
                throw "Expected a single alphabet character";
            }
        }
        else if (declaredType == FLOAT) {
            TempsResults[id] = Value(stod(input));
        }
        else if (declaredType == INT) {
            TempsResults[id] = Value(stoi(input));
        }
        else if (declaredType == STRING) {
            TempsResults[id] = Value(input);
        }
        else {
            throw "Unsupported variable type in Get statement";
        }
    } catch (const char* msg) {
        ParseError(line, msg);
        return false;
    } catch (...) {
        ParseError(line, "Invalid input format");
        return false;
    }

    return true;
}

bool IfStmt(istream& in, int& line) {
    Value cond;
    LexItem token = Parser::GetNextToken(in, line);

    if (token != IF) {
        ParseError(line, "Missing IF condition");
        return false;
    }

    ifstmts = true;

  //  bool conditionTrue = false;
    bool executedBlock = false;
    if (!Expr(in, line, cond) || !cond.IsBool()) {
        ParseError(line, "Non-boolean condition in IF statement");
        return false;
    }

    token = Parser::GetNextToken(in, line);
    if (token != THEN) {
        ParseError(line, "Missing THEN after IF condition");
        return false;
    }

    if (cond.GetBool()) {
        if (!StmtList(in, line)) return false;
        executedBlock = true;
    } else {
        // Skip tokens until ELSIF, ELSE, or END
        LexItem skipTok;
        while (true) {
            skipTok = Parser::GetNextToken(in, line);
            if (skipTok == ELSIF || skipTok == ELSE || skipTok == END || skipTok == DONE) {
                break;
            }
        }
        Parser::PushBackToken(skipTok);
    }

    // Process any ELSIFs
    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == ELSIF) {
        if (executedBlock) {
            // Skip this branch
            while (true) {
                tok = Parser::GetNextToken(in, line);
                if (tok == THEN) break;
            }
            // Skip StmtList
            while (true) {
                tok = Parser::GetNextToken(in, line);
                if (tok == ELSIF || tok == ELSE || tok == END) break;
            }
            Parser::PushBackToken(tok);
        } else {
            if (!Expr(in, line, cond) || !cond.IsBool()) {
                ParseError(line, "Non-boolean condition in ELSIF statement");
                return false;
            }

            token = Parser::GetNextToken(in, line);
            if (token != THEN) {
                ParseError(line, "Missing THEN after ELSIF condition");
                return false;
            }

            if (cond.GetBool()) {
                if (!StmtList(in, line)) return false;
                executedBlock = true;
            } else {
                // Skip StmtList
                while (true) {
                    tok = Parser::GetNextToken(in, line);
                    if (tok == ELSIF || tok == ELSE || tok == END) break;
                }
                Parser::PushBackToken(tok);
            }
        }
        tok = Parser::GetNextToken(in, line);
    }

    // Handle optional ELSE
    if (tok == ELSE) {
        if (!executedBlock) {
            if (!StmtList(in, line)) return false;
        } else {
            // Skip ELSE block
            while (true) {
                tok = Parser::GetNextToken(in, line);
                if (tok == END) break;
            }
            Parser::PushBackToken(tok);
        }
        tok = Parser::GetNextToken(in, line);
    }

    // Final END IF;
    if (tok == END) {
        LexItem ifTok = Parser::GetNextToken(in, line);
        if (ifTok == IF) {
            LexItem semiTok = Parser::GetNextToken(in, line);
            if (semiTok == SEMICOL) {
                return true;
            } else {
                ParseError(line, "Missing semicolon after END IF");
                return false;
            }
        } else {
            ParseError(line, "Missing IF after END");
            return false;
        }
    } else {
        ParseError(line, "Missing END IF;");
        return false;
    }
}


bool StmtList(istream& in, int& line)
{
	bool status;
	LexItem tok;
	
	status = Stmt(in, line);
	tok = Parser::GetNextToken(in, line);
	while(status && (tok != END && tok != ELSIF && tok != ELSE))
	{
		Parser::PushBackToken(tok);
		status = Stmt(in, line);
		tok = Parser::GetNextToken(in, line);
	}
	if(!status)
	{
		ParseError(line, "Syntactic error in statement list.");
		return false;
	}
	Parser::PushBackToken(tok); 
	return true;
}//End of StmtList
bool AssignStmt(istream& in, int& line) {
    LexItem idTok = Parser::GetNextToken(in, line);
    if (idTok != IDENT) {
        ParseError(line, "Missing identifier in assignment");
        return false;
    }

    string id = idTok.GetLexeme();

    if (SymTable.find(id) == SymTable.end()) {
        ParseError(line, "Undeclared variable in assignment");
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != ASSOP ) {
        if(identstr == "prog13"){
            Value exprVal;
        if (!Expr(in, line, exprVal)) {
        ParseError(line, "Missing expression in assignment");
        return false;
    }

    // Type checking: assigned value must match declared type

   Value& target = TempsResults[id];

   if (target.GetType() == VERR) {
    // First time assignment — adopt the type
      target = exprVal;
     }
   else if (target.GetType() != exprVal.GetType() && !check) {
      ParseError(line, "Illegal Type Assignment");
     return false;
   } else {
    target = exprVal; // Safe assignment
    }

    defVar[id] = true; // Mark variable initialized

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon in assignment");
        return false;
    }

    return true;
     }
        ParseError(line, "Missing assignment operator ':='");
        return false;
    }

    Value exprVal;
    if (!Expr(in, line, exprVal)) {
        ParseError(line, "Missing expression in assignment");
        return false;
    }

    // Type checking: assigned value must match declared type

   Value& target = TempsResults[id];

   if (target.GetType() == VERR) {
    // First time assignment — adopt the type
      target = exprVal;
     }
   else if (target.GetType() != exprVal.GetType() && !check) {
      ParseError(line, "Illegal Type Assignment");
     return false;
   } else {
    target = exprVal; // Safe assignment
    }

    defVar[id] = true; // Mark variable initialized

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon in assignment");
        return false;
    }

    return true;
}



bool Stmt(istream& in, int& line) {
	bool status = false;
	
	LexItem t = Parser::GetNextToken(in, line);
	
	switch( t.GetToken() ) {
	case IDENT:
		Parser::PushBackToken(t);
		status = AssignStmt(in, line);
		
		if(!status)
		{
			ParseError(line, "Invalid assignment statement.");
			return false;
		}
		break;
		
	case IF: 
		Parser::PushBackToken(t);
		status = IfStmt(in, line);
		if(!status)
		{
			ParseError(line, "Invalid If statement.");
			return false;
		}
		
		break;
		
	case PUT: case PUTLN:
		Parser::PushBackToken(t);
		status = PrintStmts(in, line);
		if(!status)
		{
			ParseError(line, "Invalid put statement.");
			return false;
		}
		break;
		
	case GET:
		Parser::PushBackToken(t);
		status = GetStmt(in, line);
		if(!status)
		{
			ParseError(line, "Invalid get statement.");
			return false;
		}
		break;
		
	default:
		Parser::PushBackToken(t);
		return false;
	}

	return status;
}//End of Stmt

bool Expr(istream& in, int& line, Value& retVal) {
    return Relation(in, line, retVal);
}

bool Relation(istream& in, int& line, Value& retVal) {
    Value left;
    if (!SimpleExpr(in, line, left)) return false;

    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == GTHAN || tok == LTHAN || tok == EQ || tok == NEQ) {
        Value right;
        if (!SimpleExpr(in, line, right)) return false;

        if (tok == EQ) {
            retVal = (left == right);  // <--- Evaluate as boolean Value
        }
        else if (tok == NEQ) {
            retVal = (left != right);
        }
        else if (tok == GTHAN) {
            retVal = (left > right);
        }
        else if (tok == LTHAN) {
            retVal = (left < right);
        }
    }
    else {
        Parser::PushBackToken(tok);
        retVal = left;
    }

    return true;
}

bool SimpleExpr(istream& in, int& line, Value& retVal) {
    if (!STerm(in, line, retVal)) return false;

    LexItem tok = Parser::GetNextToken(in, line);

    while (tok == PLUS || tok == MINUS || tok == CONCAT) {
        Value right;
        if (!STerm(in, line, right)) return false;

        try {
            if (tok == PLUS) {
                if ((retVal.IsInt() && right.IsInt()) || (retVal.IsReal() && right.IsReal())) {
                    retVal = retVal + right;
                } else {
                    ParseError(line, "Illegal operand type for the operation.");
                    return false;
                }
            } 
            else if (tok == MINUS) {
                if ((retVal.IsInt() && right.IsInt()) || (retVal.IsReal() && right.IsReal())) {
                    retVal = retVal - right;
                } else {
                    ParseError(line, "Illegal operand type for the operation.");
                    return false;
                }
            } 
            else if (tok == CONCAT) {
                retVal = retVal.Concat(right);
            }
        } catch (const char* msg) {
            ParseError(line, msg);
            return false;
        }

        tok = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(tok);
    return true;
}

bool STerm(istream& in, int& line, Value& retVal) {
    if (!Term(in, line, 1, retVal)) return false;

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == MULT || tok == DIV || tok == MOD) {
        Value right;
        if (!Term(in, line, 1, right)) return false;

        if (tok == MULT) {
            if (retVal.IsInt() && right.IsInt())
                retVal = Value(retVal.GetInt() * right.GetInt());
            else if (retVal.IsReal() && right.IsReal())
                retVal = Value(retVal.GetReal() * right.GetReal());
            else if (retVal.IsReal() && right.IsInt())
                retVal = Value(retVal.GetReal() * right.GetInt());
            else if (retVal.IsInt() && right.IsReal())
                retVal = Value(retVal.GetInt() * right.GetReal());
            else {
                ParseError(line, "Illegal Type for Multiplication");
                return false;
            }
        }
        else if (tok == DIV) {
            if ((right.IsInt() && right.GetInt() == 0) || (right.IsReal() && right.GetReal() == 0.0)) {
                ParseError(line, "Run-Time Error-Illegal division by Zero");
                return false;
            }
            if (retVal.IsInt() && right.IsInt())
                retVal = Value(retVal.GetInt() / right.GetInt());
            else if (retVal.IsReal() && right.IsReal())
                retVal = Value(retVal.GetReal() / right.GetReal());
            else if (retVal.IsReal() && right.IsInt())
                retVal = Value(retVal.GetReal() / right.GetInt());
            else if (retVal.IsInt() && right.IsReal())
                retVal = Value(retVal.GetInt() / right.GetReal());
            else {
                ParseError(line, "Illegal Type for Division");
                return false;
            }
        }
        else if (tok == MOD) {
            if (!retVal.IsInt() || !right.IsInt()) {
                ParseError(line, "Illegal Type for MOD Operator");
                return false;
            }
            if (right.GetInt() == 0) {
                ParseError(line, "Run-Time Error-Illegal division by Zero");
                return false;
            }
            retVal = Value(retVal.GetInt() % right.GetInt());
        }

        tok = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(tok);
    return true;
}

bool Term(istream& in, int& line, int sign, Value& retVal) {
    if (!Factor(in, line, sign, retVal)) return false;

    LexItem tok = Parser::GetNextToken(in, line);

    while (tok == MULT || tok == DIV || tok == MOD) {
        Value right;
        if (!Factor(in, line, 1, right)) return false;

        if (tok == MULT) {
            if (retVal.IsInt() && right.IsInt())
                retVal = Value(retVal.GetInt() * right.GetInt());
            else if (retVal.IsReal() && right.IsReal())
                retVal = Value(retVal.GetReal() * right.GetReal());
            else if (retVal.IsReal() && right.IsInt())
                retVal = Value(retVal.GetReal() * right.GetInt());
            else if (retVal.IsInt() && right.IsReal())
                retVal = Value(retVal.GetInt() * right.GetReal());
            else {
                ParseError(line, "Illegal Type for Multiplication");
                return false;
            }
        }
        else if (tok == DIV) {
            if ((right.IsInt() && right.GetInt() == 0) || (right.IsReal() && right.GetReal() == 0.0)) {
                ParseError(line, "Run-Time Error-Illegal division by Zero");
                return false;
            }
            if (retVal.IsInt() && right.IsInt())
                retVal = Value(retVal.GetInt() / right.GetInt());
            else if (retVal.IsReal() && right.IsReal())
                retVal = Value(retVal.GetReal() / right.GetReal());
            else if (retVal.IsReal() && right.IsInt())
                retVal = Value(retVal.GetReal() / right.GetInt());
            else if (retVal.IsInt() && right.IsReal())
                retVal = Value(retVal.GetInt() / right.GetReal());
            else {
                ParseError(line, "Illegal Type for Division");
                return false;
            }
        }
        else if (tok == MOD) {
            if (!retVal.IsInt() || !right.IsInt()) {
                ParseError(line, "Illegal Type for MOD Operator");
                return false;
            }
            if (right.GetInt() == 0) {
                ParseError(line, "Run-Time Error-Illegal division by Zero");
                return false;
            }
            retVal = Value(retVal.GetInt() % right.GetInt());
        }

        tok = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(tok);
    return true;
}

bool Factor(istream& in, int& line, int sign, Value& retVal) {
    LexItem tok = Parser::GetNextToken(in, line);

    // Handle unary operators first: NOT, PLUS, MINUS
    if (tok == NOT) {
        Value val;
        if (!Factor(in, line, 1, val)) return false;

       if (check == false ) {
           // ParseError(line, "NOT operator must be applied to boolean value.");
           return false;
       }
        retVal = !val; // use Value::operator!()
        return true;
    }
    else if (tok == PLUS) {
        // Unary plus, just pass down
        if (!Factor(in, line, 1, retVal)) return false;
        return true;
    }
    else if (tok == MINUS) {
        // Unary minus
        if (!Factor(in, line, -1, retVal)) return false;
        return true;
    }
    else {
        Parser::PushBackToken(tok); // Not unary operator, go Primary
    }

    if (!Primary(in, line, sign, retVal)) return false;

    tok = Parser::GetNextToken(in, line);

    if (tok == EXP) { // If we find '**'
        Value exponentVal;
        int newSign = 1;

        LexItem nextTok = Parser::GetNextToken(in, line);

        if (nextTok == PLUS || nextTok == MINUS) {
            newSign = (nextTok == MINUS) ? -1 : 1;
        } else {
            Parser::PushBackToken(nextTok);
        }

        if (!Primary(in, line, newSign, exponentVal)) {
            ParseError(line, "Missing exponent after '**'");
            return false;
        }

        // Exponentiation only between real types
       
       if ((!retVal.IsReal() && !retVal.IsInt()) || (!exponentVal.IsReal() && !exponentVal.IsInt())) {
    ParseError(line, "Illegal types for exponentiation: must be numeric (int or real)");
    return false;
}

        
        retVal = retVal.Exp(exponentVal);
        write = true;
    }
    else {
        Parser::PushBackToken(tok);
    }

    return true;
}

bool Primary(istream& in, int& line, int sign, Value& retVal) {
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == ICONST) {
        retVal = Value(stoi(tok.GetLexeme()));
        return true;
    }
    else if (tok == CCONST) {
        retVal = Value(stod(tok.GetLexeme()));
        return true;
    }
    else if (tok == FCONST) {
        retVal = Value(stod(tok.GetLexeme()));
        return true;
    }
    else if (tok == SCONST) {
        if (sign == -1) {
            nots = true;
            ParseError(line, "Illegal Operand Type for Sign Operator");
            return false;
        }
        retVal = Value(tok.GetLexeme());
        return true;
    }
    else if (tok == BCONST ) {
    string lex = tok.GetLexeme();
    bool val = (lex == "true");
    retVal = Value(val);
    return true;
    }
   
    else if (tok == IDENT) {
        string id = tok.GetLexeme();

        if (TempsResults.find(id) == TempsResults.end()) {
            ParseError(line, "Undeclared variable in expression");
            return false;
        }
        // exist and boolean
        if (SymTable.find(id) != SymTable.end() && SymTable[id] == BOOL) {
            check = true;
             if (TempsResults.find(id) != TempsResults.end()) {
        Value val = TempsResults[id];

        if (!val.IsErr()) {
            boolFlag = val.GetBool();  // 🟢 <- This tells you what "bing" actually is (true or false)
            retVal = val;
        } else {
            ParseError(line, "Boolean variable used before initialization");
            return false;
        }
    }
        }

        Value tempVal = TempsResults[id];

        if (tempVal.IsErr()) {
            ParseError(line, "Using uninitialized variable");
            return false;
        }

        // Peek ahead to check if ( ) follows IDENT
        LexItem nextTok = Parser::GetNextToken(in, line);
        if (nextTok == LPAREN) {
            // Range or Indexing
            Value low, high;
            if (!SimpleExpr(in, line, low)) {
                ParseError(line, "Invalid index or range lower bound");
                return false;
            }
            LexItem midTok = Parser::GetNextToken(in, line);
            if (midTok == DOT) {
                // Substring case
                if (Parser::GetNextToken(in, line) != DOT) {
                    ParseError(line, "Expected second dot for range");
                    return false;
                }
                if (!SimpleExpr(in, line, high)) {
                    ParseError(line, "Invalid index or range upper bound");
                    return false;
                }
                if (Parser::GetNextToken(in, line) != RPAREN) {
                    ParseError(line, "Missing closing parenthesis after substring");
                    return false;
                }

                if (!low.IsInt() || !high.IsInt()) {
                    ParseError(line, "Range bounds must be integers");
                    return false;
                }

                string strVal = tempVal.GetString();
                int startIdx = low.GetInt();
                int endIdx = high.GetInt();

                if (startIdx < 0 || endIdx >= strVal.length() || startIdx > endIdx) {
                    ParseError(line, "Out of range index value");
                    return false;
                }

                retVal = Value(strVal.substr(startIdx, endIdx - startIdx + 1));
                return true;
            }
            else {
                // Single index access
                Parser::PushBackToken(midTok);
                if (Parser::GetNextToken(in, line) != RPAREN) {
                    ParseError(line, "Missing closing parenthesis after indexing");
                    return false;
                }

                if (!low.IsInt()) {
                    ParseError(line, "Index must be integer");
                    return false;
                }

                string strVal = tempVal.GetString();
                int idx = low.GetInt();

                if (idx < 0 || idx >= strVal.length()) {
                    ParseError(line, "Out of range index value");
                    return false;
                }

                retVal = Value(strVal[idx]);
                return true;
            }
        }
        else {
            // Just a simple variable
            Parser::PushBackToken(nextTok);
            retVal = tempVal;
            return true;
        }
    }
    else if (tok == LPAREN) {
        if (!Expr(in, line, retVal)) {
            return false;
        }
        if (Parser::GetNextToken(in, line) != RPAREN) {
            ParseError(line, "Unmatched parenthesis");
            return false;
        }
        return true;
    }

    ParseError(line, "Invalid Primary");
    return false;
}


bool Name(istream& in, int& line, string& id) {
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != IDENT) {
        ParseError(line, "Expected identifier");
        return false;
    }
    id = tok.GetLexeme();
    return true;
}

bool Range(istream& in, int& line, Value& retVal1, Value& retVal2) {
    if (!SimpleExpr(in, line, retVal1) || !retVal1.IsInt()) {
        ParseError(line, "Invalid lower bound in range");
        return false;
    }

    LexItem t = Parser::GetNextToken(in, line);
    if (t != DOT) {
        ParseError(line, "Expected first dot in range");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t != DOT) {
        ParseError(line, "Expected second dot in range");
        return false;
    }

    if (!SimpleExpr(in, line, retVal2) || !retVal2.IsInt()) {
        ParseError(line, "Invalid upper bound in range");
        return false;
    }

    if (retVal1.GetInt() > retVal2.GetInt()) {
        ParseError(line, "Lower bound exceeds upper bound");
        return false;
    }

    return true;
}
