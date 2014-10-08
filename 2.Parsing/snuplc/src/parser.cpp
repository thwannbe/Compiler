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
  //          "begin" statSequence "end" ident ".".
  //
  // FIRST(module) = { tModule }
  // FOLLOW(module) = { $ }
  //
  CToken dummy;
  CAstModule *m = new CAstModule(dummy, "placeholder");
  
  EToken tt = _scanner->Peek().GetType();
  
  if(tt != tModule) {
    SetError(_scanner->Peek(), "\"module\" expected.");
  } else {
    Consume(tModule);
  }
  
  CAstOperand *id = NULL;

  tt = _scanner->Peek().GetType();
  if(tt != tIdent) {
    SetError(_scanner->Peek(), "identifier expected.");
  } else {
    Consume(tIdent);
  }

  tt = _scanner->Peek().GetType();
  if(tt != tSemicolon) {
    SetError(_scanner->Peek(), "semicolon(;) expected.");
  } else {
    Consume(tSemicolon);
  }

  CAstExpression *varDecl = NULL;
  varDecl = varDeclaration(m);
 
  CAstExpression *srDecl = NULL;
  tt = _scanner->Peek().GetType();
  if(tt != tBegin) {
    do {
      srDecl = subroutineDecl(m);
      
      tt = _scanner->Peek().GetType();
      if (tt == tBegin) break;

    } while (!_abort);
  }

  tt = _scanner->Peek().GetType();
  if(tt != tBegin) {
    SetError(_scanner->Peek(), "\"begin\" expected.");
  } else {
    Consume(tBegin);
  }
  
  CAstStatement *statseq = NULL;
  statseq = statSequence(m);
  
  tt = _scanner->Peek().GetType();
  if(tt != tEnd) {
    SetError(_scanner->Peek(), "\"end\" expected.");
  } else {
    Consume(tEnd);
  }
  
  CAstOperand *id2 = NULL;

  tt = _scanner->Peek().GetType();
  if(tt != tIdent) {
    SetError(_scanner->Peek(), "identifier expected.");
  } else {
    Consume(tIdent);
    //id2 = ident();
  }

  Consume(tDot);

  m->SetStatementSequence(statseq);
  // other things

  return m;
}

CAstType* CParser::type(void)
{
  //
  // type = "integer" | "boolean".
  //
  // FIRST(type) = { tInteger, tBoolean }
  // FOLLOW(type) = { tSemicolon }
  //

  EToken tt = _scanner->Peek().GetType();
  if(tt == tInteger || tt == tBoolean)
    Consume(tt);
  else
    SetError(_scanner->Peek(), "type expected.");

  return NULL;
}

CAstBinaryOp* CParser::factOp(void)
{
  //
  // factOp = "*" | "/" | "&&".
  //
  // FIRST(factOp) = { tMulDiv, tAnd }
  // FOLLOW(factOp) = { tIdent, tNumber, tBoolConst, tLBrak, tNot }
  //

  EToken tt = _scanner->Peek().GetType();
  if(tt == tMulDiv || tt == tAnd)
    Consume(tt);
  else
    SetError(_scanner->Peek(), "factOp expected.");

  return NULL;
}

CAstBinaryOp* CParser::termOp(void)
{
  //
  // termOp = "+" | "-" | "||".
  //
  // FIRST(termOp) = { tPlusMinus, tOr }
  // FOLLOW(termOp) = { tIdent, tNumber, tBoolConst, tLBrak, tNot }
  //

  EToken tt = _scanner->Peek().GetType();
  if(tt == tPlusMinus || tt == tOr)
    Consume(tt);
  else
    SetError(_scanner->Peek(), "termOp expected.");

  return NULL;
}

CAstBinaryOp* CParser::relOp(void)
{
  //
  // relOp = "=" | "#" | "<" | "<=" | ">" | ">=".
  //
  // FIRST(relOp) = { tRelOp }
  // FOLLOW(relOp) = { tIdent, tNumber, tBoolConst, tLBrak, tNot, tPlusMinus }
  //

  EToken tt = _scanner->Peek().GetType();
  if(tt == tRelOp)
    Consume(tt);
  else
    SetError(_scanner->Peek(), "RelOp expected.");

  return NULL;
}

CAstExpression* CParser::expression(CAstScope *s) {
  //
  // expression = simpleexpr [ relOp simpleexpr ].
  //
  // FIRST(expression) = { tIdent, tNumber, tBoolConst, tLBrak, tNot, tPlusMinus }
  // FOLLOW(expression) = { tEnd, tElse, tSemicolon, tComma, tRBrak }
  //
  CAstExpression *head = NULL;

  EToken tt = _scanner->Peek().GetType();
  switch(tt) {
    // simpleexpr
    case tIdent:
    case tNumber:
    case tBoolConst:
    case tLBrak:
    case tNot:
    case tPlusMinus:
      head = simpleexpr(s);
      break;

    default:
      SetError(_scanner->Peek(), "simpleexpr expected.");
      break;
  }
  assert(head != NULL);
  
  tt = _scanner->Peek().GetType();
  
  CAstBinaryOp *relop = NULL;
  CAstExpression *tail = NULL;
  if(tt == tRelOp) {
    relop = relOp();

    tt = _scanner->Peek().GetType();
    switch(tt) {
      // simpleexpr
      case tIdent:
      case tNumber:
      case tBoolConst:
      case tLBrak:
      case tNot:
      case tPlusMinus:
        tail = simpleexpr(s);
        break;

      default:
        SetError(_scanner->Peek(), "simpleexpr expected.");
        break;
    }
    assert(tail != NULL);
  }

  return head;
}

CAstExpression* CParser::simpleexpr(CAstScope *s)
{
  //
  // simpleexpr = ["+"|"-"] term { termOp term }.
  //
  // FIRST(simpleexpr) = { tIdent, tNumber, tBoolConst, tLBrak, tNot, tPlusMinus }
  // FOLLOW(simpleexpr) = { tRelOp, tComma, tRBrak, tSemicolon, tEnd, tElse }
  //
  CAstExpression *head = NULL;
  CAstExpression *tail = NULL;

  EToken tt = _scanner->Peek().GetType();
  if(tt == tPlusMinus)
    Consume(tPlusMinus);

  do {
    tt = _scanner->Peek().GetType();
    CAstExpression *t = NULL;

    switch(tt) {
      // term
      case tIdent:
      case tNumber:
      case tBoolConst:
      case tLBrak:
      case tNot:
        t = term(s);
        break;

      default:
        SetError(_scanner->Peek(), "term expected.");
        break;
    }

    assert(t != NULL);
    if (head == NULL) head = t;
    tail = t;
    
    tt = _scanner->Peek().GetType();
    if (tt == tPlusMinus || tt == tOr) {
      Consume(tt);
    } else {
      break;
    }
  } while (!_abort);
  
  return head;
}

CAstExpression* CParser::term(CAstScope *s)
{
  //
  // term = factor { factOp factor }. 
  //
  // FIRST(term) = { tIdent, tNumber, tBoolConst, tLBrak, tNot }
  // FOLLOW(term) = { tPlusMinus, tOr, tRelOp, tComma, tRBrak, tSemicolon, tEnd, tElse }
  //
  CAstExpression *head = NULL;
  CAstExpression *tail = NULL;

  EToken tt = _scanner->Peek().GetType();
  
  do {
    tt = _scanner->Peek().GetType();
    CAstExpression *fact = NULL;

    switch(tt) {
      // factor
      case tIdent:
      case tNumber:
      case tBoolConst:
      case tLBrak:
      case tNot:
        fact = factor(s);
        break;

      default:
        SetError(_scanner->Peek(), "factor expected.");
        break;
    }

    assert(fact != NULL);
    if (head == NULL) head = fact;
    tail = fact;

    tt = _scanner->Peek().GetType();
    if (tt == tMulDiv || tt == tAnd) {
      Consume(tt);
    } else {
      break;
    }
  } while (!_abort);

  return head;
}

CAstExpression* CParser::factor(CAstScope *s)
{
  //
  // factor = ident | number | boolean | "(" expression ")" | subroutineCall | "!" factor.
  //
  // FIRST(factor) = { tIdent, tNumber, tBoolConst, tLBrak, tNot }
  // FOLLOW(factor) = { tMulDiv, tAnd, tPlusMinus, tOr, tRelOp, tComma, tRBrak, tSemicolon, tEnd, tElse }
  //
  CAstConstant *n = NULL;
  CAstOperand *id = NULL;
  CAstConstant *b = NULL;
  CAstExpression *expr = NULL;
  CAstStatCall *srcall = NULL;
  CAstExpression *fact = NULL;
  EToken tt = _scanner->Peek().GetType();
  
  switch(tt) {
    case tIdent:
      id = ident();
      
      tt = _scanner->Peek().GetType();
      if(tt == tLBrak) {
        srcall = subroutineCall(s);
        assert(srcall != NULL);
        return srcall;
      } else {
        return id;
      }

    case tNumber:
      n = number();
      return n;
    
    case tBoolConst:
      b = boolean();
      return b;

    case tLBrak:
      Consume(tLBrak);

      expr = expression(s);
      assert(expr != NULL);
      
      Consume(tRBrak);
      return expr;

    case tNot:
      Consume(tNot);

      fact = factor(s);
      assert(fact != NULL);
      return fact;

    default:
      SetError(_scanner->Peek(), "identifier, number, boolean, (expression), subroutineCall, or !factor expected.");
      assert(true);
      break;
  }

  return NULL;
}


CAstStatement* CParser::statSequence(CAstScope *s)
{
  //
  // statSequence = [ statement { ";" statement } ].
  // statement    = assignment.
  //
  // FIRST(statSequence) = { tNumber }
  // FOLLOW(statSequence) = { tDot }
  //
  CAstStatement *head = NULL;


  EToken tt = _scanner->Peek().GetType();
  if (!(tt == tDot)) {
    CAstStatement *tail = NULL;

    do {
      CToken t;
      EToken tt = _scanner->Peek().GetType();
      CAstStatement *st = NULL;

      switch (tt) {
        // statement ::= assignment
        case tNumber:
          st = assignment(s);
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
      if (tt == tDot) break;

      Consume(tSemicolon);
    } while (!_abort);
  }

  return head;
}

CAstStatAssign* CParser::assignment(CAstScope *s)
{
  //
  // assignment = number ":=" expression.
  //
  CToken t;

  CAstConstant *lhs = number();

  Consume(tAssign, &t);

  CAstExpression *rhs = expression(s);

  return new CAstStatAssign(t, lhs, rhs);
}

CAstExpression* CParser::expression(CAstScope* s)
{
  //
  // expression = simpleexpr [ relOp simpleexpression ].
  //
  CToken t;
  EOperation relop;
  CAstExpression *left = NULL, *right = NULL;

  left = simpleexpr(s);

  if (_scanner->Peek().GetType() == tRelOp) {
    Consume(tRelOp, &t);
    right = simpleexpr(s);

    if (t.GetValue() == "=")       relop = opEqual;
    else if (t.GetValue() == "#")  relop = opNotEqual;
    else SetError(t, "invalid relation.");

    return new CAstBinaryOp(t, relop, left, right);
  } else {
    return left;
  }
}

CAstExpression* CParser::simpleexpr(CAstScope *s)
{
  //
  // simpleexpr ::= term { termOp term }.
  //
  CAstExpression *n = NULL;

  n = term(s);

  while (_scanner->Peek().GetType() == tPlusMinus) {
    CToken t;
    CAstExpression *l = n, *r;

    Consume(tPlusMinus, &t);

    r = term(s);

    n = new CAstBinaryOp(t, t.GetValue() == "+" ? opAdd : opSub, l, r);
  }


  return n;
}

CAstExpression* CParser::term(CAstScope *s)
{
  //
  // term = factor { ("*"|"/") factor }.
  //
  CAstExpression *n = NULL;

  n = factor(s);

  while (_scanner->Peek().GetType() == tMulDiv) {
    CToken t;
    CAstExpression *l = n, *r;

    Consume(tMulDiv, &t);

    r = factor(s);

    n = new CAstBinaryOp(t, t.GetValue() == "*" ? opMul : opDiv, l, r);
  }

  return n;
}

CAstExpression* CParser::factor(CAstScope *s)
{
  //
  // factor = number | "(" expression ")".
  //
  // FIRST(factor) = { tNumber, tLBrak }
  //
  CAstExpression *n = NULL;

  switch (_scanner->Peek().GetType()) {
    // factor ::= number
    case tNumber:
      n = number();
      break;

    // factor ::= "(" expression ")"
    case tLBrak:
      Consume(tLBrak);
      n = expression(s);
      Consume(tRBrak);
      break;

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

/*
CAstOperand* CParser::ident(void)
{
  //
  // ident = letter { letter | digit }.
  //
  // " letter { letter | digit } " is scanned as one token (tIdent)
  //

  CToken t;

  Consume(tIdent, &t);

  return new CAstOperand(t);
}*/

CAstConstant* CParser::boolean(void)
{
  //
  // boolean = "true" | "false".
  //

  CToken t;

  Consume(tBoolConst, &t);

  bool v;
  if(t.GetValue() == "true") {
    v = true;
  } else if (t.GetValue() == "false") {
    v = false;
  } else {
    SetError(t, "invalid boolean const.");
  }

  return new CAstConstant(t, CTypeManager::Get()->GetBool(), v);
}

