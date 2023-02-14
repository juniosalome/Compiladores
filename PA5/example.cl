
(*  Example cool program testing as many aspects of the code generator
    as possible.
 *)


class C {
    b : Int <- 666;
    f() : Int {b};
};

class A {
    a : Int;
    d: C <- new C;
    c: Bool <- true;
    g() : Bool {false};
    f(c: Int, e: Int) : Int {d.f()};
};

class B inherits A {
    hh : String <- "AAA";
    b : Int <- 2;
    h() : String {"BBB"};
    f(b: Int, c: Int) : Int {b <- 3};
    y(a: Int, b: Int) : Int {a+b};
};

class Main inherits IO {
  a: String;
  b: Int;
  c: B <- new B;
  plus(): Int {1 + 2};
  sub() : Int {3 - 1};
  mul() : Int {3 * 1};
  div() : Int {3 / 1};
  lt() : Bool {1 < 2};
  comp() : Bool {NOT true};
  caller() : Int {parameters(1,"2")};
  neg() : Int {~2};
  parameters(x: Int, y: String) : Int {x};
  cond() : Int {
    if (true) then 1 else 2 fi
  };
  oi() : String {"oi\n"};
  loopF() : Object { while false loop 1 pool };
  isVoidF() : Bool {isvoid 1};
  assign() : Int {b <- 3};
  newF() : Int {new Int};
  static_disp() : Int {c@A.f(2,3)};
  disp() : Int {c.f(2,3)};
  lets() : Bool { let a : Bool in let b: Bool in a};
  caseF() : Object { 
    case c of 
    e : C => e;
    f: B => 2;
    esac
  };

  main(): Object { { 

(* Just calling a lot of stuff - make sure spim wont crash *)
  plus();
  sub();
  mul();
  div();
  lt();
  comp();
  caller();
  neg();
  cond();
  loopF();
  isVoidF();
  newF();
  static_disp();
  disp();
  lets();
  caseF();

(* Printing stuff to the screen *)
    out_string("Hello World\n"); 
    out_int(3-1); 
    out_string("\n"); 
    if (1 < 2) then out_string("Deu bom\n") else out_string("Deu ruim\n") fi;
    if (3*1 = c.y(2,1)) then out_string("Deu bom\n") else out_string("Deu ruim\n") fi;
    if (3*1 <= c.y(2,1)) then out_string("Deu bom\n") else out_string("Deu ruim\n") fi;
    if (3*1 < c.y(2,1)) then out_string("Deu bom\n") else out_string("Deu ruim\n") fi;
    out_int(~666);
    out_string("\n"); 
    b <- 0;
    while b < 3 loop {out_string(oi()); b <- b + 1;} pool;
    let b: Int <- 668 in out_int(b);
    out_string("\n"); 
} };
};
