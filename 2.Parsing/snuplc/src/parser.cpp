//------------------------------------------------------------------------------
/// @brief SnuPL/0 parser
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2012/09/14 Bernhard Egger created
/// 2013/03/07 Bernhard Egger adapted to SnuPL/0
/// 2014/09/28 Bernhard Egger assignment 2: parser for SnuPL/-1
///
/// @section license_section License
/// Copyright (c) 2012-2014, Bernhard Egger
/// All rights reserved.
///
/// Redistribution and use in source and binary forms,  with or without modifi-
/// cation, are permitted provided that the following conditions are met:
///
/// - Redistributions of source code must retain the above copyright notice,
///   this list of conditions and the following disclaimer.
/// - Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY  AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER  OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT,  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSE-
/// QUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF  SUBSTITUTE
/// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
/// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT
/// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY
/// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
/// DAMAGE.
//------------------------------------------------------------------------------

#include <limits.h>
#include <cassert>
#include <vector>
#include <iostream>
#include <exception>

#include "parser.h"
using namespace std;


//------------------------------------------------------------------------------
// CParser
//
CParser::CParser(CScanner *scanner)
{
  _scanner = scanner;
  _module = NULL;
}

CAstNode* CParser::Parse(void)
{
  _abort = false;

  if (_module != NULL) { delete _module; _module = NULL; }

  try {
    if (_scanner != NULL) _module = module();

    if (_module != NULL) {
      CToken t;
      string msg;
      if (!_module->TypeCheck(&t, &msg)) SetError(t, msg);
    }
  } catch (...) {
    _module = NULL;
  }

  return _module;
}

const CToken* CParser::GetErrorToken(void) const
{
  if (_abort) return &_error_token;
  else return NULL;
}

string CParser::GetErrorMessage(void) const
{
  if (_abort) return _message;
  else return "";
}

void CParser::SetError(CToken t, const string message)
{
  _error_token = t;
  _message = message;
  _abort = true;
  throw message;
}

bool CParser::Consume(EToken type, CToken *token)
{
  if (_abort) return false;

  CToken t = _scanner->Get();

  if (t.GetType() != type) {
    SetError(t, "expected '" + CToken::Name(type) + "', got '" +
             t.GetName() + "'");
  }

  if (token != NULL) *token = t;

  return t.GetType() == type;
}

void CParser::InitSymbolTable(CSymtab *s)
{
  CTypeManager *tm = CTypeManager::Get();

  CSymProc *input = new CSymProc("Input", tm->GetInt());

  CSymProc *output = new CSymProc("Output", tm->GetNull());
  output->AddParam(new CSymParam(0, "x", tm->GetInt()));

  s->AddSymbol(input);
  s->AddSymbol(output);
}

CAstModule* CParser::module(void)
{
  //
  // module = "module" ident ";" varDeclaration { subroutineDecl }
  // "begin" statSequence "end" ident ".".
  //
  // FIRST(module) = { tModule }
  // FOLLOW(module) = { $ }
  //
  CToken mod;
  Consume(tModule, &mod);

  CToken modName;
  Consume(tIdent, &modName);
  CAstModule *m = new CAstModule(mod, modName.GetName());
  InitSymbolTable(m->GetSymbolTable());
  
  // As module class is constructed, it initialize symbol table too.
  Consume(tSemicolon);

  // global variable is added at symbol table
  varDeclaration(m);
 
  EToken tt = _scanner->Peek().GetType();
  if(tt != tBegin) {  // means subroutineDecl exists
    do {
      switch (tt) {
        case tProcedure:
        case tFunction:
          subroutineDecl(m);
          break;
        
        default:
          SetError(_scanner->Peek(), "subroutineDecl expected.");
          break;
      }
      
      tt = _scanner->Peek().GetType();
      if(tt == tBegin) break;
      
    } while (!_abort);
  }  
  Consume(tBegin);

  CAstStatement *statseq = NULL;
  statseq = statSequence(m);
  
  Consume(tEnd);
  Consume(tIdent);
  Consume(tDot);

  m->SetStatementSequence(statseq);
  
  return m;
}

void CParser::varDeclaration(CAstScope *s)
{
  // 
  // varDeclaration = [ "var" { ident { "," ident } ":" type ";" } ].
  //
  // FIRST(varDeclaration) = { tVarDecl, e }
  // FOLLOW(varDeclaration) = { tProcedure, tFunction, tBegin }
  //
  // dummy test varDeclaration ; only consume
  EToken tt = _scanner->Peek().GetType();
  if(tt == tVarDecl) {
    Consume(tVarDecl);
    do {
      tt = _scanner->Peek().GetType();
      if(tt == tProcedure || tt == tFunction || tt == tBegin) break;
      
      Consume(tIdent);
      do {
        tt = _scanner->Peek().GetType();
        if (tt != tComma) break;

        Consume(tComma);
        Consume(tIdent);
      } while (!_abort);

      Consume(tColon);

      tt = _scanner->Peek().GetType();
      if(tt == tInteger || tt == tBoolean)
        Consume(tt);
      else
        SetError(_scanner->Peek(), "type expected.");

      Consume(tSemicolon);
      
    } while (!_abort);
  }
}

void CParser::subroutineDecl(CAstScope *s)
{
  // 
  // subroutineDecl = (procedureDecl | functionDecl) subroutineBody ident ";".
  //
  // FIRST(subroutineDecl) = { tProcedure, tFunction }
  // FOLLOW(subroutineDecl) = { tBegin }
  //
  // dummy test subroutineDecl ; only consume
  EToken tt = _scanner->Peek().GetType();
  switch(tt) {
    case tProcedure:
      procedureDecl(s);
      break;

    case tFunction:
      functionDecl(s);
      break;

    default:
      SetError(_scanner->Peek(), "procedureDecl or functionDecl expected.");
      break;
  }

  subroutineBody(s);
  
  Consume(tIdent);
  Consume(tSemicolon);
  
}

void CParser::subroutineBody(CAstScope *s)
{
  //
  // subroutineBody = varDeclaration "begin" statSequence "end".
  //
  // FIRST(subroutineBody) = { tVarDecl, tBegin }
  // FOLLOW(subroutineBody) = { tIdent }
  //
  // dummy test subroutineBody ; only consume
  varDeclaration(s);

  Consume(tBegin);
  CAstStatement *statseq = NULL;
  statseq = statSequence(s);
  
  Consume(tEnd);

  //SetStatementSequence(statseq);
}

void CParser::procedureDecl(CAstScope *s)
{
  //
  // procedureDecl = "procedure" ident [ formalParam ] ";".
  //
  // FIRST(procedureDecl) = { tProcedure }
  // FOLLOW(procedureDecl) = { tVarDecl, tBegin }
  //
  // dummy test procedureDecl ; only consume
  Consume(tProcedure);
  Consume(tIdent);
  
  EToken tt = _scanner->Peek().GetType();
  if(tt != tSemicolon)
    formalParam(s);
  
  Consume(tSemicolon);
}

void CParser::functionDecl(CAstScope *s)
{
  //
  // functionDecl = "function" ident [formalParam ] ":" type ";".
  //
  // FIRST(functionDecl) = { tFunction }
  // FOLLOW(functionDecl) = { tVarDecl, tBegin }
  //
  // dummy test functionDecl ; only consume
  Consume(tFunction);
  Consume(tIdent);

  EToken tt = _scanner->Peek().GetType();
  if(tt != tColon)
    formalParam(s);

  Consume(tColon);
  tt = _scanner->Peek().GetType();
  if(tt == tInteger || tt == tBoolean)
    Consume(tt);
  else
    SetError(_scanner->Peek(), "type expected.");

  Consume(tSemicolon);
}

void CParser::formalParam(CAstScope *s)
{
  //
  // formalParam = "(" [ ident { "," ident } ] ")"
  // 
  // FIRST(formalParam) = { tLBrak }
  // FOLLOW(formalParam) = { tSemicolon, tColon }
  //
  // dummy test formalParam ; only consume
  Consume(tLBrak);

  do {
    EToken tt = _scanner->Peek().GetType();
    if (tt == tRBrak) break;

    Consume(tIdent);
    tt = _scanner->Peek().GetType();
    if (tt != tComma) break;

    Consume(tComma);
  } while (!_abort);

  Consume(tRBrak);
}

CAstStatement* CParser::statSequence(CAstScope *s)
{
  //
  // statSequence = [ statement { ";" statement } ].
  // statement    = assignment | subroutineCall | ifStatement |
  //                whileStatement | returnStatement.
  //
  // FIRST(statSequence) = { tIdent, tIf, tWhile, tReturn, e }
  // FOLLOW(statSequence) = { tEnd, tElse }
  //
  //dummy test statSequence ; only consume
  CAstStatement *head = NULL;


  EToken tt = _scanner->Peek().GetType();
  if (tt != tEnd && tt != tElse) {
    CAstStatement *tail = NULL;

    do {
      CToken t;
      EToken tt = _scanner->Peek().GetType();
      CAstStatement *st = NULL;

      switch (tt) {
        case tIdent:  // assignment or subroutineCall
          Consume(tIdent);
          tt = _scanner->Peek().GetType();
          if(tt == tAssign)
            st = assignment(s);
          else
            st = subroutineCall(s);
          break;

        case tIf:
          st = ifStatement(s);
          break;

        case tWhile:
          st = whileStatement(s);
          break;

        case tReturn:
          st = returnStatement(s);
          break;

        default:
          SetError(_scanner->Peek(), "statement expected.");
          break;
      }
      assert(st != NULL);
      if (head == NULL) head = st;
      else tail->SetNext(st);
      tail = st;

      tt = _scanner->Peek().GetType();
      if (tt == tEnd || tt == tElse) break;

      Consume(tSemicolon);
    } while (!_abort);
  }

  return head;
}

CAstStatAssign* CParser::assignment(CAstScope *s)
{
  //
  // assignment = ident ":=" expression.
  //
  // FIRST(assignment) = { tIdent }
  // FOLLOW(assignment) = { tEnd, tElse, tSemicolon }
  //
  //dummy test assignment ; only consume
  CToken t;

  //CAstConstant *lhs = ident();

  Consume(tAssign, &t);

  CAstExpression *rhs = expression(s);

  //return new CAstStatAssign(t, lhs, rhs);
  return NULL;
}

CAstStatCall* CParser::subroutineCall(CAstScope *s)
{
  //
  // subroutineCall = ident "(" [ expression { "," expression } ] ")".
  //
  // FIRST(subroutineCall) = { tIdent }
  // FOLLOW(subroutineCall) = { tMulDiv, tAnd, tPlusMinus, tOr, tRelOp, tComma,
  //                            tRBrak, tSemicolon, tEnd, tElse }
  //dummy test subroutineCall ; only consume
  CToken t;

  //Consume(tIdent, &t);
  Consume(tLBrak);

  do {
    EToken tt = _scanner->Peek().GetType();
    if(tt == tRBrak) break;

    expression(s);

    tt = _scanner->Peek().GetType();
    if(tt == tComma)
      Consume(tComma);
      
  } while(!_abort);

  Consume(tRBrak);

  return NULL;

  // should be take out proc symbol from symtab
  // return new CAstStatCall(t, &call); 
}

CAstStatIf* CParser::ifStatement(CAstScope *s)
{
  // 
  // ifStatement = "if" "(" expression ")" "then" statSequence
  //               [ "else" statSequence ] "end".
  // 
  // FIRST(ifStatement) = { tIf }
  // FOLLOW(ifStatement) = { tEnd, tElse, tSemicolon }
  //
  // dummy test ifStatement ; only consume
  CToken t;

  Consume(tIf, &t);
  Consume(tLBrak);

  CAstExpression *cond = expression(s);
  
  Consume(tRBrak);
  Consume(tThen);

  CAstStatement *ifBody = statSequence(s);
  
  CAstStatement *elseBody = NULL;
  EToken tt = _scanner->Peek().GetType();
  if (tt == tElse) {
    Consume(tElse);
    elseBody = statSequence(s);
  }
  
  Consume(tEnd);

  return new CAstStatIf(t, cond, ifBody, elseBody);  
}

CAstStatWhile* CParser::whileStatement(CAstScope *s)
{
  //
  // whileStatement = "while" "(" expression ")" "do" statSequence "end".
  //
  // FIRST(whileStatement) = { tWhile }
  // FOLLOW(whileStatement) = { tEnd, tElse, tSemicolon }
  //
  // dummy test whileStatement ; only consume
  CToken t;

  Consume(tWhile, &t);
  Consume(tLBrak);

  CAstExpression *cond = expression(s);

  Consume(tRBrak);
  Consume(tDo);

  CAstStatement *body = statSequence(s);
  
  Consume(tEnd);

  return new CAstStatWhile(t, cond, body);
}

CAstStatReturn* CParser::returnStatement(CAstScope *s)
{
  //
  // returnStatement = "return" [ expression ].
  //
  // FIRST(returnStatement) = { tReturn }
  // FOLLOW(returnStatement) = { tEnd, tElse, tSemicolon }
  //
  // dummy test returnStatement ; only consume
  CToken t;

  Consume(tReturn, &t);
  
  EToken tt = _scanner->Peek().GetType();

  CAstExpression *expr = NULL;
  if(tt != tEnd && tt != tElse && tt != tSemicolon) {
    expr = expression(s);
  }
  
  return NULL;
  // return new CAstStatReturn(t, s, expr);
}

CAstExpression* CParser::expression(CAstScope* s)
{
  //
  // expression = simpleexpr [ relOp simpleexpr ].
  //
  // FIRST(expression) = { tIdent, tNumber, tBoolConst, tLBrak, tNot, tPlusMinus }
  // FOLLOW(expression) = { tEnd, tElse, tSemicolon, tComma, tRBrak }
  //
  //dummy test expression ; only consume
  CToken t;
  EOperation relop;
  CAstExpression *left = NULL, *right = NULL;

  left = simpleexpr(s);

  if (_scanner->Peek().GetType() == tRelOp) {
    Consume(tRelOp, &t);
    right = simpleexpr(s);
   
    if (t.GetValue() == "=") relop = opEqual;
    else if (t.GetValue() == "#") relop = opNotEqual;
    else if (t.GetValue() == "<") relop = opLessThan;
    else if (t.GetValue() == "<=") relop = opLessEqual;
    else if (t.GetValue() == ">") relop = opBiggerThan;
    else if (t.GetValue() == ">=") relop = opBiggerEqual;
    else SetError(t, "invalid relation.");
    
    return new CAstBinaryOp(t, relop, left, right);
  } else {
    return left;
  }
}

CAstExpression* CParser::simpleexpr(CAstScope *s)
{
  //
  // simpleexpr ::= ["+"|"-"] term { termOp term }.
  //
  // FIRST(simpleexpr) = { tIdent, tNumber, tBoolConst, tLBrak, tNot, tPlusMinus }
  // FOLLOW(simpleexpr) = { tRelOp, tComma, tRBrak, tSemicolon, tEnd, tElse }
  //
  // dummy test simpleexpr ; only consume
  CAstExpression *n = NULL;
  
  CToken t;
  EToken tt = _scanner->Peek().GetType();
  if(tt == tPlusMinus)
    Consume(tPlusMinus, &t);

  n = term(s);

  if (t.GetValue() == "-") {
    CAstExpression *e = n;
    n = new CAstUnaryOp(t, opNeg, e);
  }

  tt = _scanner->Peek().GetType();
  while (tt == tPlusMinus || tt == tOr) {
    CAstExpression *l = n, *r;
    EOperation oper;

    Consume(tt, &t);
    if(t.GetValue() == "+") oper = opAdd;
    else if(t.GetValue() == "-") oper = opSub;
    else if(t.GetValue() == "||") oper = opOr;
    else SetError(t, "invalid termOp.");

    r = term(s);

    n = new CAstBinaryOp(t, oper, l, r);
  }

  return n;
}

CAstExpression* CParser::term(CAstScope *s)
{
  //
  // term = factor { factOp factor }.
  //
  // FIRST(term) = { tIdent, tNumber, tBoolConst, tLBrak, tNot }
  // FOLLOW(term) = { tPlusMinus, tOr, tRelOp, tComma, tRBrak, tSemicolon,
  //                  tEnd, tElse }
  //
  // dummy test term ; only consume
  CAstExpression *n = NULL;

  n = factor(s);

  EToken tt = _scanner->Peek().GetType();

  while (tt == tMulDiv || tt == tAnd) {
    CToken t;
    CAstExpression *l = n, *r;
    EOperation oper;

    Consume(tt, &t);
    if(t.GetValue() == "*") oper = opMul;
    else if(t.GetValue() == "/") oper = opDiv;
    else if(t.GetValue() == "&&") oper = opAnd;
    else SetError(t, "invalid factOp.");

    r = factor(s);

    n = new CAstBinaryOp(t, oper, l, r);
  }

  return n;
}

CAstExpression* CParser::factor(CAstScope *s)
{
  //
  // factor = ident | number | boolean | "(" expression ")" |
  //          subroutineCall | "!" factor.
  //
  // FIRST(factor) = { tIdent, tNumber, tBoolConst, tLBrak, tNot }
  // FOLLOW(factor) = { tMulDiv, tAnd, tPlusMinus, tOr, tRelOp, tComma, tRBrak,
  //                    tSemicolon, tEnd, tElse }
  //
  //dummy test factor ; only consume
  CToken t;
  CAstExpression *n = NULL;
  CAstExpression *e = NULL; // for factor ::= "!" factor
  EToken tt = _scanner->Peek().GetType();

  switch (tt) {
    // factor ::= ident | subroutineCall
    case tIdent:
      Consume(tIdent);
      tt = _scanner->Peek().GetType();
      if(tt == tLBrak) subroutineCall(s);
      break;
      
    // factor ::= number
    case tNumber:
      n = number();
      break;
    
    // factor ::= boolean
    case tBoolConst:
      Consume(tBoolConst);
      break;

    // factor ::= "(" expression ")"
    case tLBrak:
      Consume(tLBrak);
      n = expression(s);
      Consume(tRBrak);
      break;
    
    // factor ::= "!" factor
    case tNot:
      Consume(tNot, &t);
      e = factor(s);
      n = new CAstUnaryOp(t, opNot, e);

    default:
      SetError(_scanner->Peek(), "factor expected.");
      break;
  }

  return n;
}

CAstConstant* CParser::number(void)
{
  //
  // number = digit { digit }.
  //
  // "digit { digit }" is scanned as one token (tNumber)
  //

  CToken t;

  Consume(tNumber, &t);

  errno = 0;
  long long v = strtoll(t.GetValue().c_str(), NULL, 10);
  if (errno != 0) SetError(t, "invalid number.");

  return new CAstConstant(t, CTypeManager::Get()->GetInt(), v);
}

CAstConstant* CParser::ident(void)
{
  //
  // ident = letter { letter | digit }.
  //
  // "letter { letter | digit }" is scanned as one token (tIdent)
  //
  //dummy test ident ; only consume

  Consume(tIdent);

  return NULL;
}
