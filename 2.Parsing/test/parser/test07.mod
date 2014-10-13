//
// test07
//
// parser test:
// - integer & boolean, binary and unary expressions
// - my own test
//
module test07;

var a,b,c: integer;
    b0,b1: boolean;

procedure foo(a);
var b: integer;
begin
  b := c;
  c := (a+b)
end foo;

procedure unary();
begin
  a := -b;
  foo();
  b0 := !b1
end unary;

function binop(p0, p1): boolean;
begin
  return !((-p0+p1 > 0) || (p0 > 0) && (p1 < 0))
end binop;

function intop(p0, p1): integer;
begin
  p0 := binop(3, 4);
  return -p0 + (+p1)
end intop;

function compare(a, b): boolean;
begin
  if (binop(a, b) && (intop(b, a) < a) && (a # b)) then
    return true
  else
    return false
  end
end compare;

begin
end test07.
