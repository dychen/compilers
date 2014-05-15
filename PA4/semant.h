#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;

/*
 * This clas is used to check class and method inheritance and to print errors
 * encountered in semantic analysis. In type checking, it functions as the method
 * mapping M.
 * It contains a class map <class name, Class_ pointer> and an inheritance graph
 * <class name, parent class name>. The inheritance graph is constructed and
 * verified to ensure that the class declarations are valid (e.g., no cycles
 * in the inheritance tree). During type checking, helper methods use these maps
 * to get method definitions and check method inheritance.
 * See semant.cc for more details.
 */
class ClassTable {
private:
  int semant_errors;
  void install_basic_classes();
  ostream& error_stream;
  std::map<Symbol, Class_> class_map;         // Maps class names to the class pointers
  std::map<Symbol, Symbol> inheritance_graph; // Maps child classes to parents

public:
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
  void add_to_class_table(Class_ c);
  bool is_valid();
  Symbol lub(Symbol c1, Symbol c2);
  bool is_child(Symbol child, Symbol parent);
  bool class_exists(Symbol c);
  Class_ get_class(Symbol class_name);
  Formals get_formals(Symbol class_name, Symbol method_name);
  Symbol get_return_type(Symbol class_name, Symbol method_name);
  Symbol get_ancestor_method_class(Symbol class_name, Symbol method_name);
  bool check_method_signature(Symbol c1, Symbol c2, Symbol method_name);
};


#endif

