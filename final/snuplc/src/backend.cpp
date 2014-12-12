//------------------------------------------------------------------------------
/// @brief SnuPL backend
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2012/11/28 Bernhard Egger created
/// 2013/06/09 Bernhard Egger adapted to SnuPL/0
///
/// @section license_section License
/// Copyright (c) 2012,2013 Bernhard Egger
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

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "backend.h"
using namespace std;


//------------------------------------------------------------------------------
// CBackend
//
CBackend::CBackend(ostream &out)
  : _out(out)
{
}

CBackend::~CBackend(void)
{
}

bool CBackend::Emit(CModule *m)
{
  assert(m != NULL);
  _m = m;

  if (!_out.good()) return false;

  bool res = true;

  try {

    EmitHeader();
    EmitCode();
    EmitData();
    EmitFooter();

    res = _out.good();
  } catch (...) {
    res = false;
  }

  return res;
}

void CBackend::EmitHeader(void)
{
}

void CBackend::EmitCode(void)
{
}

void CBackend::EmitData(void)
{
}

void CBackend::EmitFooter(void)
{
}


//------------------------------------------------------------------------------
// CBackendx86
//
CBackendx86::CBackendx86(ostream &out)
  : CBackend(out), _curr_scope(NULL)
{
  _ind = string(4, ' ');
}

CBackendx86::~CBackendx86(void)
{
}

void CBackendx86::EmitHeader(void)
{
  _out << "##################################################" << endl
       << "# " << _m->GetName() << endl
       << "#" << endl
       << endl;
}

void CBackendx86::EmitCode(void)
{
  _out << _ind << "#-----------------------------------------" << endl
       << _ind << "# text section" << endl
       << _ind << "#" << endl
       << _ind << ".text" << endl
       << _ind << ".align 4" << endl
       << endl
       << _ind << "# entry point and pre-defined functions" << endl
       << _ind << ".global main" << endl
       << _ind << ".extern Input" << endl
       << _ind << ".extern Output" << endl
       << endl;

  vector<CScope*>::const_iterator it = _m->GetSubscopes().begin();
  while (it != _m->GetSubscopes().end())
  {
    SetScope(*it);
    EmitScope(*it);
    it++;
  }

  SetScope(_m);
  EmitScope(_m);

	  
  _out << _ind << "# end of text section" << endl
       << _ind << "#-----------------------------------------" << endl
       << endl;
}

void CBackendx86::EmitData(void)
{
  _out << _ind << "#-----------------------------------------" << endl
       << _ind << "# global data section" << endl
       << _ind << "#" << endl
       << _ind << ".data" << endl
       << _ind << ".align 4" << endl
       << endl;
  EmitGlobalData(_m);

  _out << _ind << "# end of global data section" << endl
       << _ind << "#-----------------------------------------" << endl
       << endl;
}

void CBackendx86::EmitFooter(void)
{
  _out << _ind << ".end" << endl
       << "##################################################" << endl;
}

void CBackendx86::SetScope(CScope *scope)
{
  _curr_scope = scope;
}

CScope* CBackendx86::GetScope(void) const
{
  return _curr_scope;
}

void CBackendx86::EmitScope(CScope *scope)
{
  assert(scope != NULL);

  string label;
  size_t size = ComputeStackOffsets(scope->GetSymbolTable());

  if (scope->GetParent() == NULL) label = "main";
  else label = scope->GetName();

  // label
  _out << _ind << "# scope " << scope->GetName() << endl
       << label << ":" << endl;

  _out << _ind << "pushl   %ebp" << endl;
  _out << _ind << "movl    %esp, %ebp" << endl;
  _out << _ind << "pushl   %ebx" << endl;
  _out << _ind << "pushl   %esi" << endl;
  _out << _ind << "pushl   %edi" << endl;
  _out << _ind << "subl    " << Imm(size) << ", %esp" << endl
       << endl;

  EmitCodeBlock(scope->GetCodeBlock(), scope->GetSymbolTable());

  _out << Label("exit") << ":" << endl;
  _out << _ind << "addl    " << Imm(size) << ", %esp" << endl;
  _out << _ind << "popl    %edi" << endl;
  _out << _ind << "popl    %esi" << endl;
  _out << _ind << "popl    %ebx" << endl;
  _out << _ind << "popl    %ebp" << endl;
  _out << _ind << "ret" << endl;

  _out << endl;
}

void CBackendx86::EmitGlobalData(CScope *scope)
{
  assert(scope != NULL);

  // emit the globals for the current scope
  
  _out << _ind << "# scope: " << scope->GetName() << endl
       << endl;

  string str;
  vector<CSymbol*> vect = scope->GetSymbolTable()->GetSymbols();

  for (int i = 0; i < vect.size() ; i++)
  {	  

    if (vect.at(i)->GetSymbolType() == stGlobal)
    {
      str = vect.at(i)->GetName();
      _out << str << ":" << '\t'
           << ".skip" << _ind << "4" << '\t' << '\t'
           << _ind;
      if(vect.at(i)->GetDataType()->IsBoolean())
        _out << "# <bool>" << endl;
      else
        _out << "# <int>" << endl;
    }
  }


  // emit globals in subscopes (necessary if we support static local variables)
  vector<CScope*>::const_iterator sit = scope->GetSubscopes().begin();
  while (sit != scope->GetSubscopes().end()) EmitGlobalData(*sit++);
}

void CBackendx86::EmitCodeBlock(CCodeBlock *cb, CSymtab *symtab)
{
  assert(cb != NULL);
  assert(symtab != NULL);

  const list<CTacInstr*> &instr = cb->GetInstr();
  list<CTacInstr*>::const_iterator it = instr.begin();

  while (it != instr.end()) EmitInstruction(*it++, symtab);
}

void CBackendx86::EmitInstruction(CTacInstr *i, CSymtab *symtab)
{
  assert(i != NULL);
  assert(symtab != NULL);
  
  ostringstream cmt;
  cmt << i;
  string str;

  EOperation op = i->GetOperation();
  switch (op) {
    // binary operators
    // dst = src1 op src2
    case opAdd:
    case opSub:
    case opAnd:
    case opOr:
      str = _ind + "movl    " + Operand(i->GetSrc(1)) + ", %eax";
      _out << left << setw(40) << str << '#';
      i->print(_out, 0);
      _out << endl;	   
      _out << _ind << Condition(op) << "    " << Operand(i->GetSrc(2)) << ", %eax" << endl;
      _out << _ind << "movl    %eax, " << Operand(i->GetDest()) << endl;  
      break;

    case opMul:
    case opDiv:
      str = _ind + "movl    " + Operand(i->GetSrc(1)) + ", %eax";
      _out << left << setw(40) << str << '#';
      i->print(_out, 0);
      _out << endl;
      if (op == opDiv)
        _out << _ind << "cdq" << endl;
      _out << _ind << "movl    " << Operand(i->GetSrc(2)) << ", %ebx" << endl;
      _out << _ind << Condition(op) << "   %ebx" << endl;
      _out << _ind << "movl    %eax, " << Operand(i->GetDest()) << endl;  
      break;

    // unary operators
    // dst = op src1
    case opNeg:
    case opNot:
    case opPos:
      str = _ind + "movl    " + Operand(i->GetSrc(1)) + ", %eax";
      _out << left << setw(40) << str <<'#';
      i->print(_out, 0);
      _out << endl;
      if(op != opPos)
        _out << _ind << Condition(op) << "l     %eax" << endl;
      _out << _ind << "movl    %eax, " << Operand(i->GetDest()) << endl;
      break;
    
    // memory operations
    // dst = src1
    case opAssign:
      str = _ind + "movl    " + Operand(i->GetSrc(1)) + ", %eax";
      _out << left << setw(40) << str <<'#';
      i->print(_out, 0);
      _out << endl;
      _out << _ind << "movl    %eax, " << Operand(i->GetDest()) << endl;
      break;

    // unconditional branching
    // goto dst
    case opGoto:
      str = _ind + "jmp     " + Operand(i->GetDest());
      _out << left << setw(40) << str << '#';
      i->print(_out, 0);
      _out << endl;
      break;   

    // conditional branching
    // if src1 relOp src2 then goto dst
    case opEqual:
    case opNotEqual:
    case opLessThan:
    case opLessEqual:
    case opBiggerThan:
    case opBiggerEqual:
      str = _ind + "movl    " + Operand(i->GetSrc(1)) + ", %eax";
      _out << left << setw(40) << str << '#';
      i->print(_out, 0);
      _out << endl;
      _out << _ind << "cmpl    " << Operand(i->GetSrc(2)) << ", %eax" << endl;
      _out << _ind << "j" << Condition(op) << "     " << Operand(i->GetDest()) << endl;
      break;

    // function call-related operations
    case opCall:
      str = _ind + "call    " + Operand(i->GetSrc(1));
      _out << left << setw(40) << str << '#';
      i->print(_out, 0);
      _out << endl;
      _out << _ind << "addl    " << Imm(4*((dynamic_cast<const CSymProc*>(symtab->FindSymbol(Operand(i->GetSrc(1)))))->GetNParams())) << ", %esp" << endl;
      if (i->GetDest() != NULL)
        _out << _ind << "movl    %eax, " << Operand(i->GetDest()) << endl;
      break;

    case opReturn:
      if (i->GetSrc(1) != NULL)
	_out << _ind << "movl    " << Operand(i->GetSrc(1)) << ", %eax" << endl;
      str = _ind + "jmp     " + Label("exit");
      _out << left << setw(40) << str << '#';
      i->print(_out, 0);
      _out << endl;
      break;

    case opParam:
      str = _ind + "pushl   " + Operand(i->GetSrc(1));
      _out << left << setw(40) << str << '#';
      i->print(_out, 0);
      _out<< endl;
      break;


    // special
    case opLabel:
      _out << Label(dynamic_cast<CTacLabel*>(i)) << ":" << endl;
      break;

    case opNop:
      EmitInstruction("nop", "", cmt.str());
      break;


    default:
      EmitInstruction("# ???", "not implemented", cmt.str());
  }
}

void CBackendx86::EmitInstruction(string mnemonic, string args, string comment)
{
  _out << left
       << _ind
       << setw(7) << mnemonic << " "
       << setw(23) << args;
  if (comment != "") _out << " # " << comment;
  _out << endl;
}

string CBackendx86::Operand(CTac *op) const
{
  string operand;
  CTacLabel *label = dynamic_cast<CTacLabel*>(op);
  CTacConst *c = dynamic_cast<CTacConst*>(op);
  CTacName  *v = dynamic_cast<CTacName*>(op);

  if (label != NULL)
    operand = Label(label->GetLabel());

  else if (c != NULL)
    operand = Imm(c->GetValue());

  else if (v != NULL)
  {
    switch (v->GetSymbol()->GetSymbolType())
    {
      case stGlobal:
      case stProcedure:
	operand = v->GetSymbol()->GetName();
	break;

      case stLocal:
      case stParam:
	operand = to_string((long long)(v->GetSymbol()->GetOffset())) + "(" + v->GetSymbol()->GetBaseRegister() + ")";
	break;
    }
  }

  return operand;
}

string CBackendx86::Imm(int value) const
{
  ostringstream o;
  o << "$" << dec << value;
  return o.str();
}

string CBackendx86::Label(CTacLabel* label) const
{
  CScope *cs = GetScope();
  assert(cs != NULL);

  ostringstream o;
  o << "l_" << cs->GetName() << "_" << label->GetLabel();
  return o.str();
  return "l_" + cs->GetName() + "_" + label->GetLabel();
}

string CBackendx86::Label(string label) const
{
  CScope *cs = GetScope();
  assert(cs != NULL);

  return "l_" + cs->GetName() + "_" + label;
}

string CBackendx86::Condition(EOperation cond) const
{
  switch (cond) {
    case opEqual:       return "e ";
    case opNotEqual:    return "ne";
    case opLessThan:    return "l ";
    case opLessEqual:   return "le";
    case opBiggerThan:  return "g ";
    case opBiggerEqual: return "ge";
    case opAdd:		return "addl";
    case opSub:		return "subl";
    case opMul:		return "imull";
    case opDiv:		return "idivl";
    case opAnd:		return "andl";
    case opOr:		return "orl ";
    case opNeg:		return "neg";
    case opNot:		return "not";
    default:            assert(false); break;
  }
}

size_t CBackendx86::ComputeStackOffsets(CSymtab *symtab)
{
  assert(symtab != NULL);
  vector<CSymbol*> slist = symtab->GetSymbols();
  vector<CSymbol*>::iterator it = slist.begin();
  int i = -4;

  while(it != slist.end())
  {
    if ((*it)->GetSymbolType() == stLocal)
    {
      (*it)->SetBaseRegister("%ebp");
      (*it)->SetOffset(i);
      i -= 4;
    }

    else if ((*it)->GetSymbolType() == stParam)
    {
      int offset = (dynamic_cast<CSymParam*>(*it))->GetIndex();
      (*it)->SetBaseRegister("%ebp");
      (*it)->SetOffset(8+offset*4);
    }

    it++;
  }
  size_t size = (size_t)(-i-4);
  return size;
}
