
//**************************************************************
//
// Code generator SKELETON
//
// Read the comments carefully. Make sure to
//    initialize the base class tags in
//       `CgenClassTable::CgenClassTable'
//
//    Add the label for the dispatch tables to
//       `IntEntry::code_def'
//       `StringEntry::code_def'
//       `BoolConst::code_def'
//
//    Add code to emit everyting else that is needed
//       in `CgenClassTable::code'
//
//
// The files as provided will produce code to begin the code
// segments, declare globals, and emit constants.  You must
// fill in the rest.
//
//**************************************************************

#include "cgen.h"
#include "cgen_gc.h"

#include <functional>
#include <sstream>
#include <algorithm>

extern void emit_string_constant(ostream& str, char *s);
extern int cgen_debug;

//
// Three symbols from the semantic analyzer (semant.cc) are used.
// If e : No_type, then no code is generated for e.
// Special code is generated for new SELF_TYPE.
// The name "self" also generates code different from other references.
//
//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
Symbol 
       arg,
       arg2,
       Bool,
       concat,
       cool_abort,
       copy,
       Int,
       in_int,
       in_string,
       IO,
       length,
       Main,
       main_meth,
       No_class,
       No_type,
       Object,
       out_int,
       out_string,
       prim_slot,
       self,
       SELF_TYPE,
       Str,
       str_field,
       substr,
       type_name,
       val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
  arg         = idtable.add_string("arg");
  arg2        = idtable.add_string("arg2");
  Bool        = idtable.add_string("Bool");
  concat      = idtable.add_string("concat");
  cool_abort  = idtable.add_string("abort");
  copy        = idtable.add_string("copy");
  Int         = idtable.add_string("Int");
  in_int      = idtable.add_string("in_int");
  in_string   = idtable.add_string("in_string");
  IO          = idtable.add_string("IO");
  length      = idtable.add_string("length");
  Main        = idtable.add_string("Main");
  main_meth   = idtable.add_string("main");
//   _no_class is a symbol that can't be the name of any 
//   user-defined class.
  No_class    = idtable.add_string("_no_class");
  No_type     = idtable.add_string("_no_type");
  Object      = idtable.add_string("Object");
  out_int     = idtable.add_string("out_int");
  out_string  = idtable.add_string("out_string");
  prim_slot   = idtable.add_string("_prim_slot");
  self        = idtable.add_string("self");
  SELF_TYPE   = idtable.add_string("SELF_TYPE");
  Str         = idtable.add_string("String");
  str_field   = idtable.add_string("_str_field");
  substr      = idtable.add_string("substr");
  type_name   = idtable.add_string("type_name");
  val         = idtable.add_string("_val");
}

static char *gc_init_names[] =
  { "_NoGC_Init", "_GenGC_Init", "_ScnGC_Init" };
static char *gc_collect_names[] =
  { "_NoGC_Collect", "_GenGC_Collect", "_ScnGC_Collect" };

static int labelNum = 0;
static int localOffset = -1;
static CgenNode* cur_node = NULL;
  
//  BoolConst is a class that implements code generation for operations
//  on the two booleans, which are given global names here.
BoolConst falsebool(FALSE);
BoolConst truebool(TRUE);

//*********************************************************
//
// Define method for code generation
//
// This is the method called by the compiler driver
// `cgtest.cc'. cgen takes an `ostream' to which the assembly will be
// emmitted, and it passes this and the class list of the
// code generator tree to the constructor for `CgenClassTable'.
// That constructor performs all of the work of the code
// generator.
//
//*********************************************************

void program_class::cgen(ostream &os) 
{
  // spim wants comments to start with '#'
  os << "# start of generated code\n";

  initialize_constants();
  CgenClassTable *codegen_classtable = new CgenClassTable(classes,os);
  codegen_classtable->code();
  codegen_classtable->exitscope();

  os << "\n# end of generated code\n";
}


//////////////////////////////////////////////////////////////////////////////
//
//  emit_* procedures
//
//  emit_X  writes code for operation "X" to the output stream.
//  There is an emit_X for each opcode X, as well as emit_ functions
//  for generating names according to the naming conventions (see emit.h)
//  and calls to support functions defined in the trap handler.
//
//  Register names and addresses are passed as strings.  See `emit.h'
//  for symbolic names you can use to refer to the strings.
//
//////////////////////////////////////////////////////////////////////////////

static void emit_load(char *dest_reg, int offset, char *source_reg, ostream& s)
{
  s << LW << dest_reg << " " << offset * WORD_SIZE << "(" << source_reg << ")" 
    << endl;
}

static void emit_store(char *source_reg, int offset, char *dest_reg, ostream& s)
{
  s << SW << source_reg << " " << offset * WORD_SIZE << "(" << dest_reg << ")"
      << endl;
}

static void emit_load_imm(char *dest_reg, int val, ostream& s)
{ s << LI << dest_reg << " " << val << endl; }

static void emit_load_address(char *dest_reg, char *address, ostream& s)
{ s << LA << dest_reg << " " << address << endl; }

static void emit_partial_load_address(char *dest_reg, ostream& s)
{ s << LA << dest_reg << " "; }

static void emit_load_bool(char *dest, const BoolConst& b, ostream& s)
{
  emit_partial_load_address(dest,s);
  b.code_ref(s);
  s << endl;
}

static void emit_load_string(char *dest, StringEntry *str, ostream& s)
{
  emit_partial_load_address(dest,s);
  str->code_ref(s);
  s << endl;
}

static void emit_load_int(char *dest, IntEntry *i, ostream& s)
{
  emit_partial_load_address(dest,s);
  i->code_ref(s);
  s << endl;
}

static void emit_move(char *dest_reg, char *source_reg, ostream& s)
{ s << MOVE << dest_reg << " " << source_reg << endl; }

static void emit_neg(char *dest, char *src1, ostream& s)
{ s << NEG << dest << " " << src1 << endl; }

static void emit_add(char *dest, char *src1, char *src2, ostream& s)
{ s << ADD << dest << " " << src1 << " " << src2 << endl; }

static void emit_addu(char *dest, char *src1, char *src2, ostream& s)
{ s << ADDU << dest << " " << src1 << " " << src2 << endl; }

static void emit_addiu(char *dest, char *src1, int imm, ostream& s)
{ s << ADDIU << dest << " " << src1 << " " << imm << endl; }

static void emit_div(char *dest, char *src1, char *src2, ostream& s)
{ s << DIV << dest << " " << src1 << " " << src2 << endl; }

static void emit_mul(char *dest, char *src1, char *src2, ostream& s)
{ s << MUL << dest << " " << src1 << " " << src2 << endl; }

static void emit_sub(char *dest, char *src1, char *src2, ostream& s)
{ s << SUB << dest << " " << src1 << " " << src2 << endl; }

static void emit_sll(char *dest, char *src1, int num, ostream& s)
{ s << SLL << dest << " " << src1 << " " << num << endl; }

static void emit_jalr(char *dest, ostream& s)
{ s << JALR << "\t" << dest << endl; }

static void emit_jal(char *address,ostream &s)
{ s << JAL << address << endl; }

static void emit_return(ostream& s)
{ s << RET << endl; }

static void emit_gc_assign(ostream& s)
{ s << JAL << "_GenGC_Assign" << endl; }

static void emit_disptable_ref(Symbol sym, ostream& s)
{  s << sym << DISPTAB_SUFFIX; }

static void emit_init_ref(Symbol sym, ostream& s)
{ s << sym << CLASSINIT_SUFFIX; }

static void emit_label_ref(int l, ostream &s)
{ s << "label" << l; }

static void emit_protobj_ref(Symbol sym, ostream& s)
{ s << sym << PROTOBJ_SUFFIX; }

static void emit_method_ref(Symbol classname, Symbol methodname, ostream& s)
{ s << classname << METHOD_SEP << methodname; }

static void emit_label_def(int l, ostream &s)
{
  emit_label_ref(l,s);
  s << ":" << endl;
}

static void emit_beqz(char *source, int label, ostream &s)
{
  s << BEQZ << source << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_beq(char *src1, char *src2, int label, ostream &s)
{
  s << BEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bne(char *src1, char *src2, int label, ostream &s)
{
  s << BNE << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bleq(char *src1, char *src2, int label, ostream &s)
{
  s << BLEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blt(char *src1, char *src2, int label, ostream &s)
{
  s << BLT << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blti(char *src1, int imm, int label, ostream &s)
{
  s << BLT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bgti(char *src1, int imm, int label, ostream &s)
{
  s << BGT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_branch(int l, ostream& s)
{
  s << BRANCH;
  emit_label_ref(l,s);
  s << endl;
}

//
// Push a register on the stack. The stack grows towards smaller addresses.
//
static void emit_push(char *reg, ostream& str)
{
  emit_store(reg,0,SP,str);
  emit_addiu(SP,SP,-4,str);
}

static void emit_pop(char *reg, ostream& str) {
  emit_load(reg,1,SP,str);    
  emit_addiu(SP,SP,4,str);
}

//
// Fetch the integer value in an Int object.
// Emits code to fetch the integer value of the Integer object pointed
// to by register source into the register dest
//
static void emit_fetch_int(char *dest, char *source, ostream& s)
{ emit_load(dest, DEFAULT_OBJFIELDS, source, s); }

//
// Emits code to store the integer value contained in register source
// into the Integer object pointed to by dest.
//
static void emit_store_int(char *source, char *dest, ostream& s)
{ emit_store(source, DEFAULT_OBJFIELDS, dest, s); }


static void emit_test_collector(ostream &s)
{
  emit_push(ACC, s);
  emit_move(ACC, SP, s); // stack end
  emit_move(A1, ZERO, s); // allocate nothing
  s << JAL << gc_collect_names[cgen_Memmgr] << endl;
  emit_addiu(SP,SP,4,s);
  emit_load(ACC,0,SP,s);
}

static void emit_gc_check(char *source, ostream &s)
{
  if (source != A1) emit_move(A1, source, s);
  s << JAL << "_gc_check" << endl;
}

static void emit_before_function(ostream& s, Feature method = NULL) {

    //Populate environment
    if (method!= NULL) {
        //I'm assuming the arguments will be pushed to stack
        //in their natural order
        for (int ii = method->get_formals()->first(); method->get_formals()->more(ii); 
                ii = method->get_formals()->next(ii)) {
            int offset = DEFAULT_OBJFIELDS +  method->get_formals()->len() - ii -1;
            cur_node->get_environment().addid(method->get_formals()->nth(ii)->get_name(),
                    new Coordinate(offset, FP));
        }
    }


    //Pushes fp, s0 and ra to the stack, respectively
    emit_addiu(SP, SP, -12, s );
    emit_store(FP, 3, SP,s);
    emit_store(SELF, 2, SP,s);
    emit_store(RA, 1, SP,s);
    emit_addiu(FP,SP, 4, s);

    return;

}

//Common code to be emmitted after a function's body
static void emit_after_function(ostream& s, Feature method = NULL) {
    int numAttrs = DEFAULT_OBJFIELDS;
    if (method != NULL) {
        numAttrs += method->get_formals()->len();
    }

    //Pop fp, s0 and ra from stack
    emit_load(FP, 3, SP,s);
    emit_load(SELF, 2, SP,s);
    emit_load(RA, 1, SP,s);
    emit_addiu(SP, SP, numAttrs*WORD_SIZE, s );
    emit_return(s);
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
//
// coding strings, ints, and booleans
//
// Cool has three kinds of constants: strings, ints, and booleans.
// This section defines code generation for each type.
//
// All string constants are listed in the global "stringtable" and have
// type StringEntry.  StringEntry methods are defined both for String
// constant definitions and references.
//
// All integer constants are listed in the global "inttable" and have
// type IntEntry.  IntEntry methods are defined for Int
// constant definitions and references.
//
// Since there are only two Bool values, there is no need for a table.
// The two booleans are represented by instances of the class BoolConst,
// which defines the definition and reference methods for Bools.
//
///////////////////////////////////////////////////////////////////////////////

//
// Strings
//
void StringEntry::code_ref(ostream& s)
{
  s << STRCONST_PREFIX << index;
}

//
// Emit code for a constant String.
// You should fill in the code naming the dispatch table.
//

void StringEntry::code_def(ostream& s, int stringclasstag)
{
  IntEntryP lensym = inttable.add_int(len);

  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s  << LABEL                                             // label
      << WORD << stringclasstag << endl                                 // tag
      << WORD << (DEFAULT_OBJFIELDS + STRING_SLOTS + (len+4)/4) << endl // size
      << WORD;


 /***** Add dispatch information for class String ******/
      s << "String_dispTab";
      s << endl;                                              // dispatch table
      s << WORD;  lensym->code_ref(s);  s << endl;            // string length
  emit_string_constant(s,str);                                // ascii string
  s << ALIGN;                                                 // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the 
// stringtable.
//
void StrTable::code_string_table(ostream& s, int stringclasstag)
{  
  for (List<StringEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,stringclasstag);
}

//
// Ints
//
void IntEntry::code_ref(ostream &s)
{
  s << INTCONST_PREFIX << index;
}

//
// Emit code for a constant Integer.
// You should fill in the code naming the dispatch table.
//

void IntEntry::code_def(ostream &s, int intclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                // label
      << WORD << intclasstag << endl                      // class tag
      << WORD << (DEFAULT_OBJFIELDS + INT_SLOTS) << endl  // object size
      << WORD; 

 /***** Add dispatch information for class Int ******/
      s << "Int_dispTab";
      s << endl;                                          // dispatch table
      s << WORD << str << endl;                           // integer value
}


//
// IntTable::code_string_table
// Generate an Int object definition for every Int constant in the
// inttable.
//
void IntTable::code_string_table(ostream &s, int intclasstag)
{
  for (List<IntEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,intclasstag);
}


//
// Bools
//
BoolConst::BoolConst(int i) : val(i) { assert(i == 0 || i == 1); }

void BoolConst::code_ref(ostream& s) const
{
  s << BOOLCONST_PREFIX << val;
}
  
//
// Emit code for a constant Bool.
// You should fill in the code naming the dispatch table.
//

void BoolConst::code_def(ostream& s, int boolclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                  // label
      << WORD << boolclasstag << endl                       // class tag
      << WORD << (DEFAULT_OBJFIELDS + BOOL_SLOTS) << endl   // object size
      << WORD;

 /***** Add dispatch information for class Bool ******/

      s << "Bool_dispTab";
 
      s << endl;                                            // dispatch table
      s << WORD << val << endl;                             // value (0 or 1)
}

//////////////////////////////////////////////////////////////////////////////
//
//  CgenClassTable methods
//
//////////////////////////////////////////////////////////////////////////////

//***************************************************
//
//  Emit code to start the .data segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_data()
{
  Symbol main    = idtable.lookup_string(MAINNAME);
  Symbol string  = idtable.lookup_string(STRINGNAME);
  Symbol integer = idtable.lookup_string(INTNAME);
  Symbol boolc   = idtable.lookup_string(BOOLNAME);

  str << "\t.data\n" << ALIGN;
  //
  // The following global names must be defined first.
  //
  str << GLOBAL << CLASSNAMETAB << endl;
  str << GLOBAL; emit_protobj_ref(main,str);    str << endl;
  str << GLOBAL; emit_protobj_ref(integer,str); str << endl;
  str << GLOBAL; emit_protobj_ref(string,str);  str << endl;
  str << GLOBAL; falsebool.code_ref(str);  str << endl;
  str << GLOBAL; truebool.code_ref(str);   str << endl;
  str << GLOBAL << INTTAG << endl;
  str << GLOBAL << BOOLTAG << endl;
  str << GLOBAL << STRINGTAG << endl;

  //
  // We also need to know the tag of the Int, String, and Bool classes
  // during code generation.
  //
  str << INTTAG << LABEL
      << WORD << intclasstag << endl;
  str << BOOLTAG << LABEL 
      << WORD << boolclasstag << endl;
  str << STRINGTAG << LABEL 
      << WORD << stringclasstag << endl;    
}


//***************************************************
//
//  Emit code to start the .text segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_text()
{
  str << GLOBAL << HEAP_START << endl
      << HEAP_START << LABEL 
      << WORD << 0 << endl
      << "\t.text" << endl
      << GLOBAL;
  emit_init_ref(idtable.add_string("Main"), str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Int"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("String"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Bool"),str);
  str << endl << GLOBAL;
  emit_method_ref(idtable.add_string("Main"), idtable.add_string("main"), str);
  str << endl;
}

void CgenClassTable::code_bools(int boolclasstag)
{
  falsebool.code_def(str,boolclasstag);
  truebool.code_def(str,boolclasstag);
}

void CgenClassTable::code_select_gc()
{
  //
  // Generate GC choice constants (pointers to GC functions)
  //
  str << GLOBAL << "_MemMgr_INITIALIZER" << endl;
  str << "_MemMgr_INITIALIZER:" << endl;
  str << WORD << gc_init_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_COLLECTOR" << endl;
  str << "_MemMgr_COLLECTOR:" << endl;
  str << WORD << gc_collect_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_TEST" << endl;
  str << "_MemMgr_TEST:" << endl;
  str << WORD << (cgen_Memmgr_Test == GC_TEST) << endl;
}


//********************************************************
//
// Emit code to reserve space for and initialize all of
// the constants.  Class names should have been added to
// the string table (in the supplied code, is is done
// during the construction of the inheritance graph), and
// code for emitting string constants as a side effect adds
// the string's length to the integer table.  The constants
// are emmitted by running through the stringtable and inttable
// and producing code for each entry.
//
//********************************************************

void CgenClassTable::code_constants()
{
  //
  // Add constants that are required by the code generator.
  //
  stringtable.add_string("");
  inttable.add_string("0");

  stringtable.code_string_table(str,stringclasstag);
  inttable.code_string_table(str,intclasstag);
  code_bools(boolclasstag);
}


CgenClassTable::CgenClassTable(Classes classes, ostream& s) : nds(NULL) , str(s)
{
   stringclasstag = 4 /* Change to your String class tag here */;
   intclasstag =    2 /* Change to your Int class tag here */;
   boolclasstag =   3 /* Change to your Bool class tag here */;

   enterscope();
   if (cgen_debug) cout << "Building CgenClassTable" << endl;
   install_basic_classes();
   install_classes(classes);
   build_inheritance_tree();

   if (cgen_debug) cout << "coding global data" << endl;
   code_global_data();

   if (cgen_debug) cout << "choosing gc" << endl;
   code_select_gc();

   if (cgen_debug) cout << "coding constants" << endl;
   code_constants();

   set_obj_properties();
}

void CgenClassTable::install_basic_classes()
{

// The tree package uses these globals to annotate the classes built below.
  curr_lineno  = 0;
  Symbol filename = stringtable.add_string("<basic class>");

//
// A few special class names are installed in the lookup table but not
// the class list.  Thus, these classes exist, but are not part of the
// inheritance hierarchy.
// No_class serves as the parent of Object and the other special classes.
// SELF_TYPE is the self class; it cannot be redefined or inherited.
// prim_slot is a class known to the code generator.
//
  addid(No_class,
        new CgenNode(class_(No_class,No_class,nil_Features(),filename),
                            Basic,this));
  addid(SELF_TYPE,
        new CgenNode(class_(SELF_TYPE,No_class,nil_Features(),filename),
                            Basic,this));
  addid(prim_slot,
        new CgenNode(class_(prim_slot,No_class,nil_Features(),filename),
                            Basic,this));

// 
// The Object class has no parent class. Its methods are
//        cool_abort() : Object    aborts the program
//        type_name() : Str        returns a string representation of class name
//        copy() : SELF_TYPE       returns a copy of the object
//
// There is no need for method bodies in the basic classes---these
// are already built in to the runtime system.
//
  install_class(
   new CgenNode(
    class_(Object, 
           No_class,
           append_Features(
           append_Features(
           single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
           single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
           single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
           filename),
    Basic,this));

// 
// The IO class inherits from Object. Its methods are
//        out_string(Str) : SELF_TYPE          writes a string to the output
//        out_int(Int) : SELF_TYPE               "    an int    "  "     "
//        in_string() : Str                    reads a string from the input
//        in_int() : Int                         "   an int     "  "     "
//
   install_class(
    new CgenNode(
     class_(IO, 
            Object,
            append_Features(
            append_Features(
            append_Features(
            single_Features(method(out_string, single_Formals(formal(arg, Str)),
                        SELF_TYPE, no_expr())),
            single_Features(method(out_int, single_Formals(formal(arg, Int)),
                        SELF_TYPE, no_expr()))),
            single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
            single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
           filename),       
    Basic,this));

//
// The Int class has no methods and only a single attribute, the
// "val" for the integer. 
//
   install_class(
    new CgenNode(
     class_(Int, 
            Object,
            single_Features(attr(val, prim_slot, no_expr())),
            filename),
     Basic,this));

//
// Bool also has only the "val" slot.
//
    install_class(
     new CgenNode(
      class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename),
      Basic,this, "bool_const0"));

//
// The class Str has a number of slots and operations:
//       val                                  ???
//       str_field                            the string itself
//       length() : Int                       length of the string
//       concat(arg: Str) : Str               string concatenation
//       substr(arg: Int, arg2: Int): Str     substring
//       
   install_class(
    new CgenNode(
      class_(Str, 
             Object,
             append_Features(
             append_Features(
             append_Features(
             append_Features(
             single_Features(attr(val, Int, no_expr())),
            single_Features(attr(str_field, prim_slot, no_expr()))),
            single_Features(method(length, nil_Formals(), Int, no_expr()))),
            single_Features(method(concat, 
                                   single_Formals(formal(arg, Str)),
                                   Str, 
                                   no_expr()))),
            single_Features(method(substr, 
                                   append_Formals(single_Formals(formal(arg, Int)), 
                                                  single_Formals(formal(arg2, Int))),
                                   Str, 
                                   no_expr()))),
             filename),
        Basic,this));

}

// CgenClassTable::install_class
// CgenClassTable::install_classes
//
// install_classes enters a list of classes in the symbol table.
//
void CgenClassTable::install_class(CgenNodeP nd)
{
  Symbol name = nd->get_name();

  if (probe(name))
    {
      return;
    }

  nd->set_classtag(list_length(nds));
  global_class_tags.push_back(nd);
    
  // The class name is legal, so add it to the list of classes
  // and the symbol table.
  nds = new List<CgenNode>(nd,nds);
  addid(name,nd);
}

void CgenClassTable::install_classes(Classes cs)
{
  for(int i = cs->first(); cs->more(i); i = cs->next(i))
    install_class(new CgenNode(cs->nth(i),NotBasic,this));
}

//
// CgenClassTable::build_inheritance_tree
//
void CgenClassTable::build_inheritance_tree()
{
  for(List<CgenNode> *l = nds; l; l = l->tl())
      set_relations(l->hd());
}

//
// CgenClassTable::set_relations
//
// Takes a CgenNode and locates its, and its parent's, inheritance nodes
// via the class table.  Parent and child pointers are added as appropriate.
//
void CgenClassTable::set_relations(CgenNodeP nd)
{
  CgenNode *parent_node = probe(nd->get_parent());
  nd->set_parentnd(parent_node);
  parent_node->add_child(nd);
}

void CgenNode::add_child(CgenNodeP n)
{
  children = new List<CgenNode>(n,children);
}

void CgenNode::set_parentnd(CgenNodeP p)
{
  assert(parentnd == NULL);
  assert(p != NULL);
  parentnd = p;
}

std::string CgenClassTable::get_str_const(const std::string& s) {
    std::stringstream ss;
    StringEntry* entry = stringtable.lookup_string(const_cast<char*>(s.c_str()));
    
    entry->code_ref(ss);

    return ss.str();

}

std::string CgenClassTable::get_int_const(const std::string& s) {
    std::stringstream ss;
    IntEntry* entry = inttable.lookup_string(const_cast<char*>(s.c_str()));
    
    entry->code_ref(ss);

    return ss.str();
}

void CgenClassTable::code()
{

//                 Add your code to emit
//                   - prototype objects
//                   - class_nameTab
//                   - dispatch tables
//

    emit_name_table();

    emit_obj_table();

    for(List<CgenNode> *l = nds; l; l = l->tl()) {
        cur_node = l->hd();
        emit_disptable(l->hd());
    }

    for(List<CgenNode> *l = nds; l; l = l->tl()) {
        cur_node = l->hd();
        emit_prototype(l->hd());
    }

  if (cgen_debug) cout << "coding global text" << endl;
  code_global_text();

//                 Add your code to emit
//                   - object initializer
//                   - the class methods
//                   - etc...

    for(List<CgenNode> *l = nds; l; l = l->tl()) {
        emit_initializer(l->hd());
    }

    emit_methods();


}

void CgenClassTable::emit_name_table() {

    str << "class_nameTab:" << endl;

    for (size_t ii = 0 ; ii < global_class_tags.size(); ++ii) {
        CgenNode* node = global_class_tags[ii];

        str << WORD << get_str_const(node->get_name()->get_string()) << endl;
    }

}

void CgenClassTable::emit_obj_table() {

    str << "class_objTab:" << endl;
    for (size_t ii = 0 ; ii < global_class_tags.size(); ++ii) {
        CgenNode* node = global_class_tags[ii];

        str << WORD << node->get_name() << "_protObj" << endl;
        str << WORD << node->get_name() << "_init" << endl;
    }

}

void CgenClassTable::emit_initializer(CgenNode* node) {

    emit_init_ref(node->get_name(),str); str << LABEL;
    
    emit_before_function(str);
    emit_move(SELF, ACC,str);

    //Call parent's init (JAL)
    CgenNode* parent = node->get_parentnd();
    if (parent->get_name() != No_class) {
        std::string init(parent->get_name()->get_string());
        init += CLASSINIT_SUFFIX;
        emit_jal((char*)init.c_str(),str);
    }

    //Calls code to init attributes (if necessary)

    for (size_t ii = 0; ii < node->get_local_attributes().size(); ++ii) {
        node->get_local_attributes()[ii]->code(node,str);
    }


    emit_move(ACC, SELF,str);
    emit_after_function(str);

}

void CgenClassTable::emit_methods() {
    //Write emit_methods() method (which will recursively write everything else)

    //Iterate through classes
    for(List<CgenNode> *l = nds; l; l = l->tl()) {
        CgenNode* node = l->hd();
        cur_node = node;
       
        //Iterate through methods
        for (size_t ii = 0; ii < node->get_local_methods().size(); ++ii) {
            node->get_local_methods()[ii]->code(node, str);
        }
    }
}

void CgenClassTable::emit_disptable(CgenNode* node) {
    str << node->get_name() << "_dispTab:" << endl;
    for (DispTableT::iterator it = node->get_disptable().begin();
            it != node->get_disptable().end();
            ++it) {
        str << WORD << it->second->get_name() << "." << it->first << endl;
    }
}

void CgenClassTable::emit_prototype(CgenNode* node) {
    //This -1 seems to be mandatory
   str << WORD << "-1" << endl; 
   str << node->get_name() << "_protObj:" << endl;



   //Class Tag
   str << WORD << node->get_classtag() << endl;

   //Object size
   str << WORD << node->get_objsize() << endl;

   //Dispatch pointer
   str << WORD << node->get_name() << "_dispTab" << endl;

   //Attributes...

   for (std::vector<Feature>::iterator it = node->get_attributes().begin();
           it != node->get_attributes().end(); 
           ++it) {
       str << WORD << probe((*it)->get_type())->get_default_val() << endl; 
   }

}


void attr_class::set_node_info(CgenNode* node) {
    node->get_attributes().push_back(this);
    node->get_local_attributes().push_back(this);

    //Populate environment accordingly
    node->get_environment().addid(this->name, new Coordinate(DEFAULT_OBJFIELDS + 
                node->get_attributes().size() - 1, SELF));
}

struct PairComparator {
    PairComparator(Symbol s) : sym(s) {}
    inline bool operator()(const std::pair<Symbol,CgenNode*>& pair) {
        return pair.first == sym;
    }

    Symbol sym;
};

void method_class::set_node_info(CgenNode* node) {
    DispTableT& disptable = node->get_disptable();
    std::pair<Symbol,CgenNode*> pair = std::pair<Symbol,CgenNode*>(this->name,node);
    DispTableT::iterator it = std::find_if(disptable.begin(),disptable.end(), 
            PairComparator(this->name));

    //Updates reference of a certain method on the table if that method is already
    //there
    if (it != disptable.end()) {
        it->second = node;
    } else {
        disptable.push_back(pair);
    }

    node->get_local_methods().push_back(this);
}

//You can also see this as a "first pass"
void CgenClassTable::set_obj_properties() {
    rec_obj_properties(root());
}

void CgenClassTable::rec_obj_properties(CgenNode* node) {
    EnvironmentT& env = node->get_environment();

    env.enterscope();
    
    //Adds features from class right above
    node->get_attributes() = node->get_parentnd()->get_attributes();

    //Repopulate environment accordingly
    for (size_t ii = 0; ii < node->get_attributes().size(); ++ii) {
        Feature attr = node->get_attributes()[ii];
        env.addid(attr->get_name(), new Coordinate(DEFAULT_OBJFIELDS + ii, SELF));
    }

    node->get_disptable() = node->get_parentnd()->get_disptable();

    if (node->get_name() == Str) {
        node->set_default_val(get_str_const(""));
    } else if (node->get_name() == Int) {
        node->set_default_val(get_int_const("0"));
    }

    Features features = node->get_features();
    for (int ii = features->first(); features->more(ii); ii = features->next(ii)) {
        features->nth(ii)->set_node_info(node);
    }

    for (List<CgenNode>* l = node->get_children(); l; l = l->tl()) {
        rec_obj_properties(l->hd());
    }

    //If control reaches here, it has reached base case for recursion
}

//Saves a certain value from $a0 to a certain class attribute 
//(which is located at a certain offset from SELF, or $s0)
static void emit_save_attr(CgenNode* node, Feature attr, std::ostream& s) {

    int diff = DEFAULT_OBJFIELDS;

    for (size_t ii=0; ii<node->get_attributes().size(); ++ii) {
        if (node->get_attributes()[ii] == attr) {
           emit_store(ACC, diff + ii, SELF,s ); 
           return;
        }
    }

    return;
    
}

void attr_class::code(CgenNode* node, std::ostream& s) {
    init->code(s);
    if (init->get_type()) {
        emit_save_attr(node, this, s);        
    }
    return;
}

std::string Coordinate::to_string() {
    std::stringstream ss;
    ss << offset*WORD_SIZE << "(" << reg << ")";
    return ss.str();
}

void method_class::code(CgenNode* node, std::ostream& s) {
    if (!node->basic()) {
        emit_method_ref(node->get_name(), this->name, s ); 
        s << LABEL;

        //Now the magic happens...

        //(probably in the emit_X_function methods...)
        node->get_environment().enterscope();
        emit_before_function(s, this);
        emit_move(SELF, ACC,s);
        expr->code(s);
        emit_after_function(s, this);
        node->get_environment().exitscope();
    }
    return;

}

CgenNodeP CgenClassTable::root()
{
   return probe(Object);
}


///////////////////////////////////////////////////////////////////////
//
// CgenNode methods
//
///////////////////////////////////////////////////////////////////////

CgenNode::CgenNode(Class_ nd, Basicness bstatus, CgenClassTableP ct, std::string default_val_) :
   class__class((const class__class &) *nd),
   parentnd(NULL),
   children(NULL),
   basic_status(bstatus),
   default_val(default_val_)
{ 
   stringtable.add_string(name->get_string());          // Add class name to string table
}


//******************************************************************
//
//   Fill in the following methods to produce code for the
//   appropriate expression.  You may add or remove parameters
//   as you wish, but if you do, remember to change the parameters
//   of the declarations in `cool-tree.h'  Sample code for
//   constant integers, strings, and booleans are provided.
//
//*****************************************************************

void assign_class::code(ostream &s) {
    EnvironmentT& env = cur_node->get_environment();
    expr->code(s);
    
    emit_store(ACC,env.lookup(name)->offset ,env.lookup(name)->reg,s);
}

void static_dispatch_class::code(ostream &s) {
    for (int ii = actual->first(); actual->more(ii); ii = actual->next(ii) ) {
        actual->nth(ii)->code(s);    
        emit_push(ACC,s);
    }
    expr->code(s);
    emit_bne(ACC,ZERO, labelNum,s);

    //Dispatching on void
    StringEntry* entry = stringtable.lookup_string(cur_node->get_filename()->get_string());
    emit_load_string(ACC, entry,s);
    emit_load_imm(T1, line_number,s);
    emit_jal("_dispatch_abort", s);

    //Dispatch table stuff
    emit_label_def(labelNum,s);
    emit_partial_load_address(T1,s); emit_disptable_ref(type_name,s); s << endl;

    //Gets offset
    CgenNode* type;
    if (type_name == SELF_TYPE) {
        type = cur_node;
    } else {
        type = codegen_classtable->lookup(type_name);
    }
    DispTableT& disp = type->get_disptable();
    size_t offset = 0;
    for (offset = 0; offset < disp.size(); ++offset) {
        if (disp[offset].first == name) break;
    }

    emit_load(T1, offset,T1,s);
    emit_jalr(T1,s);


    ++labelNum;
}

void dispatch_class::code(ostream &s) {
    for (int ii = actual->first(); actual->more(ii); ii = actual->next(ii) ) {
        actual->nth(ii)->code(s);    
        emit_push(ACC,s);
    }
    expr->code(s);
    emit_bne(ACC,ZERO, labelNum,s);

    //Dispatching on void
    StringEntry* entry = stringtable.lookup_string(cur_node->get_filename()->get_string());
    emit_load_string(ACC, entry,s);
    emit_load_imm(T1, line_number,s);
    emit_jal("_dispatch_abort", s);

    //Dispatch table stuff
    emit_label_def(labelNum,s);
    emit_load(T1, DISPTABLE_OFFSET, ACC,s);

    //Gets offset
    CgenNode* type;
    if (expr->get_type() == SELF_TYPE) {
        type = cur_node;
    } else {
       type =  codegen_classtable->lookup(expr->get_type());
    }
    DispTableT& disp = type->get_disptable();
    size_t offset = 0;
    for (offset = 0; offset < disp.size(); ++offset) {
        if (disp[offset].first == name) break;
    }

    emit_load(T1, offset,T1,s);
    emit_jalr(T1,s);


    ++labelNum;
}

void cond_class::code(ostream &s) {
    pred->code(s);

    emit_load(ACC,3,ACC,s);

    int falseLabel = labelNum++;
    int endLabel = labelNum++;

    emit_beqz(ACC,falseLabel,s); //branch to false if ACC = 0

    //True branch
    then_exp->code(s);
    emit_branch(endLabel,s); //goes to the end branch

    //False branch
    emit_label_def(falseLabel,s);
    else_exp->code(s);

    emit_label_def(endLabel,s);
}

void loop_class::code(ostream &s) {
    int loopBegin = labelNum++;
    int loopEnd = labelNum++;

    emit_label_def(loopBegin,s);
    pred->code(s);
    emit_load(T1,3,ACC,s);
    emit_beq(T1,ZERO,loopEnd,s);
    body->code(s);
    emit_branch(loopBegin,s);
    emit_label_def(loopEnd,s);
    emit_move(ACC,ZERO,s);
}

void typcase_class::code(ostream &s) {
    int firstLabel = labelNum; ++labelNum;
    expr->code(s);
    emit_bne(ACC,ZERO,labelNum,s);
    StringEntry* entry = stringtable.lookup_string(cur_node->get_filename()->get_string());
    emit_load_string(ACC, entry,s);
    emit_load_imm(T1, line_number,s);
    emit_jal("_case_abort2", s);


    for (int ii = cases->first(); cases->more(ii); ii = cases->next(ii)) {
        emit_label_def(labelNum,s);
        ++labelNum;
        Symbol type = cases->nth(ii)->get_type();
        int tag = codegen_classtable->lookup(type)->get_classtag();
        EnvironmentT& env = cur_node->get_environment();

        emit_load(T2,0,ACC,s);
        emit_blti(T2,tag,labelNum,s);
        emit_bgti(T2,tag,labelNum,s);
        env.enterscope();
        env.addid(cases->nth(ii)->get_name(), new Coordinate(
                    localOffset, FP));
        emit_push(ACC,s);
        --localOffset;
        cases->nth(ii)->code(s);
        ++localOffset;
        env.exitscope();
        emit_addiu(SP,SP,4,s);
        emit_branch(firstLabel,s);
    }

    emit_label_def(labelNum,s);
    emit_jal("_case_abort",s);

    emit_label_def(firstLabel,s);


    ++labelNum;
}

void block_class::code(ostream &s) {
    for (int ii = body->first(); body->more(ii); ii = body->next(ii)) {
        body->nth(ii)->code(s);
    }
}

void let_class::code(ostream &s) {
    EnvironmentT& env = cur_node->get_environment();

    init->code(s);
    env.enterscope();

    if (init->get_type() == NULL) {
        emit_load_address(T1,(char*)
                codegen_classtable->lookup(type_decl)->get_default_val().c_str(),s);
    } else {
        emit_move(T1, ACC,s);
    }

    emit_push(T1,s); //make room for local variable
    env.addid(identifier, new Coordinate(localOffset,FP));
    --localOffset;
    body->code(s);
    ++localOffset;

    emit_addiu(SP,SP,4,s); //clear the local variable

    env.exitscope();
}

void plus_class::code(ostream &s) {
    e1->code(s);
    emit_jal("Object.copy",s);
    emit_push(ACC,s);
    e2->code(s);
    emit_pop(T1,s);
    emit_load(ACC,3,ACC,s); //get the actual value from e2 into acc
    emit_load(T2,3,T1,s); //get the actual value from e2 into T2
    emit_add(T2,T2,ACC,s);
    emit_store(T2,3,T1,s); //store new value in new object
    emit_move(ACC,T1,s);
}

void sub_class::code(ostream &s) {
    e1->code(s);
    emit_jal("Object.copy",s);
    emit_push(ACC,s);
    e2->code(s);
    emit_pop(T1,s);
    emit_load(ACC,3,ACC,s); //get the actual value from e2 into acc
    emit_load(T2,3,T1,s); //get the actual value from e2 into T2
    emit_sub(T2,T2,ACC,s);
    emit_store(T2,3,T1,s); //store new value in new object
    emit_move(ACC,T1,s);
}

void mul_class::code(ostream &s) {
    e1->code(s);
    emit_jal("Object.copy",s);
    emit_push(ACC,s);
    e2->code(s);
    emit_pop(T1,s);
    emit_load(ACC,3,ACC,s); //get the actual value from e2 into acc
    emit_load(T2,3,T1,s); //get the actual value from e2 into T2
    emit_mul(T2,T2,ACC,s);
    emit_store(T2,3,T1,s); //store new value in new object
    emit_move(ACC,T1,s);
}

void divide_class::code(ostream &s) {
    e1->code(s);
    emit_jal("Object.copy",s);
    emit_push(ACC,s);
    e2->code(s);
    emit_pop(T1,s);
    emit_load(ACC,3,ACC,s); //get the actual value from e2 into acc
    emit_load(T2,3,T1,s); //get the actual value from e2 into T2
    emit_div(T2,T2,ACC,s);
    emit_store(T2,3,T1,s); //store new value in new object
    emit_move(ACC,T1,s);
}

void neg_class::code(ostream &s) {
    e1->code(s);
    emit_jal("Object.copy",s);
    emit_load(T1,3,ACC,s);
    emit_neg(T1, T1,s );
    emit_store(T1,3,ACC,s);
}

void lt_class::code(ostream &s) {
    e1->code(s);
    emit_load(ACC,3,ACC,s);
    emit_push(ACC,s);
    e2->code(s);
    emit_load(ACC,3,ACC,s);
  
    emit_pop(T1,s);

    emit_load_bool(T2, BoolConst(1), s);
    emit_blt(T1, ACC,labelNum,s );
    emit_load_bool(T2, BoolConst(0), s);

    emit_label_def(labelNum,s);
    emit_move(ACC,T2,s);
    
    ++labelNum;
}

void eq_class::code(ostream &s) {
    e1->code(s);
    emit_load(ACC,3,ACC,s);
    emit_push(ACC,s);
    e2->code(s);
    emit_load(ACC,3,ACC,s);
    emit_pop(T1,s);

    emit_load_bool(T2, BoolConst(1), s);
    emit_beq(T1, ACC,labelNum,s );
    emit_load_bool(T2, BoolConst(0), s);

    emit_label_def(labelNum,s);
    emit_move(ACC,T2,s);
    
    ++labelNum;
}

void leq_class::code(ostream &s) {
    e1->code(s);
    emit_load(ACC,3,ACC,s);
    emit_push(ACC,s);
    e2->code(s);
    emit_load(ACC,3,ACC,s);
    emit_pop(T1,s);

    emit_load_bool(T2, BoolConst(1), s);
    emit_bleq(T1, ACC,labelNum,s );
    emit_load_bool(T2, BoolConst(0), s);

    emit_label_def(labelNum,s);
    emit_move(ACC,T2,s);
    
    ++labelNum;
}

void comp_class::code(ostream &s) {
    e1->code(s);
    emit_load(ACC,3,ACC,s);

    emit_load_bool(T1, BoolConst(1), s);
    emit_beqz(ACC,labelNum,s );
    emit_load_bool(T1, BoolConst(0), s);

    emit_label_def(labelNum,s);
    emit_move(ACC,T1, s);
    
    ++labelNum;
}

void int_const_class::code(ostream& s)  
{
  //
  // Need to be sure we have an IntEntry *, not an arbitrary Symbol
  //
  emit_load_int(ACC,inttable.lookup_string(token->get_string()),s);
}

void string_const_class::code(ostream& s)
{
  emit_load_string(ACC,stringtable.lookup_string(token->get_string()),s);
}

void bool_const_class::code(ostream& s)
{
  emit_load_bool(ACC, BoolConst(val), s);
}

void new__class::code(ostream &s) {
    emit_partial_load_address(ACC, s);
    emit_protobj_ref(type_name,s); s << endl;
    emit_jal("Object.copy",s);
    s << JAL << " "; emit_init_ref(type_name,s); s << endl;
}

void isvoid_class::code(ostream &s) {
    e1->code(s);
    emit_move(T1,ACC,s);
    emit_load_bool(ACC,BoolConst(1),s);
    emit_beqz(T1,labelNum,s);
    emit_load_bool(ACC,BoolConst(0),s);

    emit_label_def(labelNum,s);
    labelNum++;

}

void no_expr_class::code(ostream &s) {
}

void object_class::code(ostream &s) {
    EnvironmentT& env = cur_node->get_environment();

    if (name != self) {
        emit_load(ACC, env.lookup(name)->offset, env.lookup(name)->reg,s);
    } else {
        emit_move(ACC,SELF,s);
    }
}
