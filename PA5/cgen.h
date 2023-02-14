#include <assert.h>
#include <stdio.h>
#include "emit.h"
#include "cool-tree.h"
#include "symtab.h"
#include <vector>
#include <map>

enum Basicness     {Basic, NotBasic};
#define TRUE 1
#define FALSE 0

class CgenClassTable;
typedef CgenClassTable *CgenClassTableP;
typedef std::vector<std::pair<Symbol, CgenNode*> > DispTableT;

class CgenNode;
typedef CgenNode *CgenNodeP;

struct Coordinate {
    Coordinate(int o, char* r) : offset(o), reg(r) {}
    int offset;
    char* reg;
    std::string to_string();
};

typedef SymbolTable<Symbol,Coordinate> EnvironmentT;

class CgenClassTable : public SymbolTable<Symbol,CgenNode> {
private:
   List<CgenNode> *nds;
   Classes& classes;
   ostream& str;
   int stringclasstag;
   int intclasstag;
   int boolclasstag;
    std::vector<CgenNode*> global_class_tags;

// The following methods emit code for
// constants and global declarations.

   void code_global_data();
   void code_global_text();
   void code_bools(int);
   void code_select_gc();
   void code_constants();

// The following creates an inheritance graph from
// a list of classes.  The graph is implemented as
// a tree of `CgenNode', and class names are placed
// in the base class symbol table.

   void install_basic_classes();
   void install_class(CgenNodeP nd);
   void install_classes(Classes cs);
   void build_inheritance_tree();
   void set_relations(CgenNodeP nd);
   
   //Returns string const associated with certain class
   std::string get_str_const(const std::string&);

   //Returns string const associated with certain class
   std::string get_int_const(const std::string&);

   //Sets additional object properties, like its attribute and disptable
   void set_obj_properties();

   //Recursive helper function for method above
   void rec_obj_properties(CgenNode*);

   //Emit prototype for a certain class node
   void emit_prototype(CgenNode*);

   //Emit dispatch table for a certain class node
   void emit_disptable(CgenNode*);

   void emit_name_table(); //class_nameTab
    
   void emit_obj_table(); //class_objTab

   void emit_initializer(CgenNode*);

   void emit_methods();
   
public:
   CgenClassTable(Classes&, ostream& str);
   void code();
   CgenNodeP root();
};

const int metadata_size = 3;
class CgenNode : public class__class {
private: 
   CgenNodeP parentnd;                        // Parent of class
   List<CgenNode> *children;                  // Children of class
   Basicness basic_status;                    // `Basic' if class is basic
                                              // `NotBasic' otherwise
       //Attributes Written by me
   std::vector<Feature> attributes;           // will hold all attributes from entire 
                                              // class chain, following the necessary order
   std::vector<Feature> local_attributes;
   DispTableT  disptable;                     // dispatch table implementation
                                              //maps method name to its corresponding class
   std::vector<Feature> local_methods;

   SymbolTable<Symbol,Coordinate> environment;
   int classtag;
   std::string default_val;

public:
   CgenNode(Class_ c,
            Basicness bstatus,
            CgenClassTableP class_table,
            std::string default_val_ = "0");

   void add_child(CgenNodeP child);
   List<CgenNode> *get_children() { return children; }
   void set_parentnd(CgenNodeP p);
   CgenNodeP get_parentnd() { return parentnd; }
   int basic() { return (basic_status == Basic); }
   
    //Methods written by me
   void set_classtag(int classtag_) {classtag = classtag_;}
   int get_classtag() { return classtag;}
   int get_objsize() {return attributes.size() + metadata_size;}
   std::vector<Feature>& get_attributes() {return attributes;}
   std::vector<Feature>& get_local_attributes() {return local_attributes;}
   std::vector<Feature>& get_local_methods() {return local_methods;}
   std::vector<std::pair<Symbol,CgenNode*> >& get_disptable() {return disptable;}
   std::string get_default_val() {return default_val;}
   void set_default_val(const std::string& s) {default_val = s;}
   Features get_features() {return features;}
   SymbolTable<Symbol,Coordinate>& get_environment() { return environment;}
};

class BoolConst 
{
 private: 
  int val;
 public:
  BoolConst(int);
  void code_def(ostream&, int boolclasstag);
  void code_ref(ostream&) const;
};
