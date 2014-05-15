

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"


extern int semant_debug;
extern char *curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
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

ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr) {

    /* Construct inheritance graph, a graph of <child, parent> class mappings */
    /* Add basic classes */
    install_basic_classes();

    /* Add other classes */
    for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
        add_to_class_table(classes->nth(i));
    }
}

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
   // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
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
	       filename);  

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
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
	       filename);

    add_to_class_table(Object_class);
    add_to_class_table(IO_class);
    add_to_class_table(Int_class);
    add_to_class_table(Bool_class);
    add_to_class_table(Str_class);
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 

/*
 * Other ClassTable methods
 */

/*
 * Adds the class to the class table (specifically, adds it to the class map
 * and the inheritance graph) with the following caveats:
 * - Cannot add a class that has already been defined.
 * - Classes cannot inherit from Bool, SELF_TYPE, or String.
 * - Cannot define a SELF_TYPE class.
 */
void ClassTable::add_to_class_table(Class_ c) {
    Symbol name = c->get_name();
    Symbol parent = c->get_parent();
    if (parent == Bool || parent == SELF_TYPE || parent == Str) {
        ostream &err_stream = semant_error(c);
        err_stream << "Class " << name << " cannot inherit class " << parent << ".\n"; 
    }
    else if (name == SELF_TYPE) {
        ostream &err_stream = semant_error(c);
        err_stream << "Redefinition of basic class " << name << ".\n";
    }
    else if (class_map.count(name) == 0 && inheritance_graph.count(name) == 0) {
        class_map[name] = c;
        inheritance_graph[name] = parent;
    }
    else {
        ostream &err_stream = semant_error(c);
        err_stream << "Class " << name << " has already been defined.\n";
    }
}

/*
 * Validates the inheritance graph by checking that:
 *   1. Every parent class is defined.
 *   2. There are no cycles.
 *   3. The class Main is defined.
 */
void ClassTable::validate() {
    bool is_main_defined = false;
    for (std::map<Symbol, Symbol>::iterator iter = inheritance_graph.begin();
        iter != inheritance_graph.end(); ++iter) {
        Symbol child = iter->first;
        Symbol parent = iter->second;
        if (child == Main)
            is_main_defined = true;
        while (parent != No_class) {
            if (parent == child) {
                // Error - cycle detected
                ostream& err_stream = semant_error(class_map[child]);
                err_stream << "Class " << child << " inherits from itself.\n";
                break;
            }
            else if (inheritance_graph.count(parent) == 0) {
                // Error - parent not found
                ostream& err_stream = semant_error(class_map[child]);
                err_stream << "Class " << child << " inherits from undefined class "
                           << parent << ".\n";
                break;
            }
            else {
                parent = inheritance_graph[parent];
            }
        }
    } 
    if (is_main_defined == false) {
        ostream &err_stream = semant_error();
        err_stream << "Class Main is not defined.\n";
    }
}

/*
 * Returns the least upper bound (least common ancestor)
 * of classes class1 and class2) in the inheritance graph.
 * This is a naive O(N^2) algorithm where N is the branch
 * length.
 */
Symbol ClassTable::lub(Symbol class1, Symbol class2) {
    Symbol c1 = class1;
    Symbol parent = Object;
    while (c1 != Object) {
        Symbol c2 = class2;
        while (c2 != Object) {
            if (c1 == c2) {
                parent = c1;
                goto finish;
            }
            c2 = inheritance_graph[c2];
        }
        c1 = inheritance_graph[c1];
    }
    finish:
        return parent;
}

/*
 * Returns the class pointer of the input class name.
 */
Class_ ClassTable::get_class(Symbol class_name) {
    return class_map[class_name];
}

/*
 * Traverses the inheritance chain of the input class until
 * it finds a method matching the input method name and returns
 * the corresponding formal parameters. Outputs an error and
 * returns NULL if no matching method is found.
 */
Formals ClassTable::get_formals(Symbol class_name, Symbol method_name) {
    Symbol cname = class_name;
    while (class_name != No_class) {
        Class_ c = class_map[cname];
        Formals f = c->get_formals(method_name);
        if (f != NULL)
            return f;
        cname = inheritance_graph[cname]; 
    }
    cerr << "No method " << method_name << " found for class " << class_name << "\n.";
    return NULL;
}

/*
 * Traverses the inheritance chain of the input class until
 * it finds a method matching the input method name and returns
 * the corresponding formal parameters. Outputs an error and
 * returns NULL if no matching method is found.
 */
Symbol ClassTable::get_return_type(Symbol class_name, Symbol method_name) {
    Symbol cname = class_name;
    while (class_name != No_class) {
        Class_ c = class_map[cname];
        Symbol r = c->get_return_type(method_name);
        if (r != NULL)
            return r;
        cname = inheritance_graph[cname];
    }
    cerr << "No method " << method_name << "found for class " << class_name << "\n.";
    return NULL;
}

/*
 * Returns true if child is a subclass of parent, false otherwise.
 * This is an O(N) algorithm where N is the branch length.
 */
bool ClassTable::is_child(Symbol child, Symbol parent) {
    Symbol c = child;
    if (parent == Object)
        return true;
    while (c != Object) {
        if (c == parent)
            return true;
        c = inheritance_graph[c];
    }
    return false;
}

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant()
{
    initialize_constants();

    /* Initialize a new ClassTable inheritance graph and make sure it
       is well-formed. */
    ClassTable *classtable = new ClassTable(classes);
    classtable->validate();

    type_env_t env;
    env.om = new SymbolTable<Symbol, Symbol>();
    env.curr = NULL;
    env.ct = classtable;

    /* Perform a top-down traversal to fill out the symbol table and then
       a bottom-up traversal to type-check. */
    for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
        env.om->enterscope();
        env.curr = classes->nth(i);
        classes->nth(i)->init_class(env);
        classes->nth(i)->type_check(env);
        env.om->exitscope();
    }

    if (classtable->errors()) {
	cerr << "Compilation halted due to static semantic errors." << endl;
	exit(1);
    }
}

/*
 * Other semantic analysis helper methods defined in cool-tree.h
 */

Symbol class__class::get_name() {
    return name;
}

Symbol class__class::get_parent() {
    return parent;
}

/*
 * Initializes environment tables with class attributes.
 * Adds class attributes along the inheritance change,
 * adding parent class attributes first.
 * TODO: Fix attribute overriding (child attribute should
 *       override parent attribute, but attributes shouldn't
 *       be multiply defined for a single class.
 */
void class__class::init_class(type_env_t env) {
    if (name != Object) {
        env.ct->get_class(parent)->init_class(env);
    }
    for (int i = features->first(); features->more(i); i = features->next(i)) {
        features->nth(i)->add_to_environment(env);
    }
}

/*
 * Gets the list of formals for a particular method of the class.
 * Returns NULL if the method isn't found.
 */
Formals class__class::get_formals(Symbol method) {
    for (int i = features->first(); features->more(i); i = features->next(i)) {
        Feature feature = features->nth(i);
        if (feature->is_method() && feature->get_name() == method)
            return feature->get_formals();
    }
    return NULL;
}

/*
 * Gets the return type of a particular method of the class.
 * Returns NULL if the method isn't found.
 */
Symbol class__class::get_return_type(Symbol method) {
    for (int i = features->first(); features->more(i); i = features->next(i)) {
        Feature feature = features->nth(i);
        if (feature->is_method() && feature->get_name() == method)
            return feature->get_return_type();
    }
    return NULL;
}

bool method_class::is_method() { return true; }
bool attr_class::is_method() { return false; }

Formals method_class::get_formals() { return formals; }
Symbol method_class::get_return_type() { return return_type; }

// These two methods should never be called.
Formals attr_class::get_formals() { return NULL; }
Symbol attr_class::get_return_type() { return NULL; }

Symbol method_class::get_name() { return name; };
Symbol attr_class::get_name() { return name; };

void method_class::add_to_environment(type_env_t env) { /* Nothing to do */ }

void attr_class::add_to_environment(type_env_t env) {
    if (env.om->probe(name) == NULL)
        env.om->addid(name, &type_decl);
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "Unable to add attribute " << name
                   << " to object map (already defined).\n";
    }
}

Class_ class__class::type_check(type_env_t env) {
    //cout << "Starting type check for class " << name << ".\n";
    for (int i = features->first(); features->more(i); i = features->next(i)) {
        features->nth(i)->type_check(env);
    }
    return this;
}

Feature method_class::type_check(type_env_t env) {
    //cout << "Type checking method " << name << ".\n";
    // This assumes the method is already in the method map.
    // Doesn't check for inheritance.
    env.om->enterscope();
    Symbol curr_class = env.curr->get_name();
    env.om->addid(self, &curr_class);
    for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
        formals->nth(i)->type_check(env);
    }
    Symbol tret = expr->type_check(env)->type;
    if (tret == SELF_TYPE)
        tret = env.curr->get_name();
    if (return_type == SELF_TYPE) {
        if (env.ct->is_child(tret, curr_class)) {}
        else {
            ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
            err_stream << "Method initialization " << tret
                       << " is not a subclass of " << curr_class << ".\n";
        }
    }
    else {
        if (env.ct->is_child(tret, return_type)) {}
        else {
            ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
            err_stream << "Method initialization " << tret
                       << " is not a subclass of " << return_type << ".\n";
        }
    }
    env.om->exitscope();
    return this;
}

Feature attr_class::type_check(type_env_t env) {
    //cout << "Type checking attribute " << name << ".\n";
//    env.om->addid(name, &type_decl);
    env.om->enterscope();
    Symbol curr_class = env.curr->get_name();
    env.om->addid(self, &curr_class);
    Symbol t1 = init->type_check(env)->type;
    env.om->exitscope();
    if (t1 == SELF_TYPE)
        t1 = env.curr->get_name();
    if (t1 != No_type) {
        if (!(env.ct->is_child(t1, type_decl))) {
            ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
            err_stream << "Attribute initialization " << t1
                       << " is not a subclass of " << type_decl << ".\n";
        }
    }
    if (name == self) {
        ostream &err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "'self' cannot be the name of an attribute.\n";
    }
    return this;
}

Formal formal_class::type_check(type_env_t env) {
    if (env.om->probe(name) == NULL)
        env.om->addid(name, &type_decl);
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "Duplicate formal " << name << ".\n";
    }
    return this;
}

Symbol branch_class::type_check(type_env_t env) {
    env.om->addid(name, &type_decl);
    return expr->type_check(env)->type;
}

Expression assign_class::type_check(type_env_t env) {
    //cout << "In assign_class\n";
    Symbol t1 = *env.om->lookup(name);
    Symbol t2 = expr->type_check(env)->type;
    if (t2 == SELF_TYPE) {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "Cannot assign to 'self'.\n";
        type = Object;
    }
    else if (env.ct->is_child(t2, t1))
        type = t2;
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << t2 << " is not a subclass of " << t1 << ".\n";
        type = Object;
    }
    return this;
}

Expression static_dispatch_class::type_check(type_env_t env) {
    //cout << "In static_dispatch_class\n";
    std::vector<Symbol> param_types;
    Symbol t0 = expr->type_check(env)->type;
    if (t0 == SELF_TYPE)
        t0 = env.curr->get_name();
    if (env.ct->is_child(t0, type_name)) {
        for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
            param_types.push_back(actual->nth(i)->type_check(env)->type);
        }
        Formals formals = env.ct->get_formals(t0, name);
        Symbol ret_type = env.ct->get_return_type(t0, name);
        for (std::vector<Symbol>::iterator iter = param_types.begin(); iter != param_types.end(); ++iter) {
            Symbol param_type = *iter;
            // TODO: Check formals
        }
        if (ret_type == SELF_TYPE)
            type = t0;
        else
            type = ret_type;
    }
    else {
        ostream &err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "Evaluated class " << t0 << " must be a child of declared class "
                   << type_name << " in static dispatch.\n";
        type = Object;
    }
    return this;
}

Expression dispatch_class::type_check(type_env_t env) {
    //cout << "In dispatch_class\n";
    std::vector<Symbol> param_types;
    Symbol t0 = expr->type_check(env)->type;
    Symbol curr = t0;
    if (t0 == SELF_TYPE)
        curr = env.curr->get_name();
    for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
        param_types.push_back(actual->nth(i)->type_check(env)->type);
    }
    Formals formals = env.ct->get_formals(curr, name);
    Symbol ret_type = env.ct->get_return_type(curr, name);
    for (std::vector<Symbol>::iterator iter = param_types.begin(); iter != param_types.end(); ++iter) {
        Symbol param_type = *iter;
        // TODO: Check formals
    }
    if (ret_type == SELF_TYPE)
        type = t0;    
    else
        type = ret_type;
    return this;
}

Expression cond_class::type_check(type_env_t env) {
    //cout << "In cond_class\n";
    Symbol t1 = pred->type_check(env)->type;
    Symbol t2 = then_exp->type_check(env)->type;
    if (t2 == SELF_TYPE)
        t2 = env.curr->get_name();
    Symbol t3 = else_exp->type_check(env)->type;
    if (t3 == SELF_TYPE)
        t3 = env.curr->get_name();
    if (t1 == Bool)
        type = env.ct->lub(t2, t3);
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "If condition did not evaluate to a boolean.\n";
        type = Object;
    }
    return this;
}

Expression loop_class::type_check(type_env_t env) {
    //cout << "In loop_class\n";
    Symbol t1 = pred->type_check(env)->type;
    Symbol t2 = body->type_check(env)->type;
    if (t1 == Bool)
        type = Object;
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "While condition did not evaluate to a boolean.\n";
        type = Object;
    }
    return this;
}

Expression typcase_class::type_check(type_env_t env) {
    //cout << "In typecase_class\n";
    Symbol t0 = expr->type_check(env)->type;
    // Warning: The following typechecks the first case twice.
    //          Might need to change this.
    Symbol tn = cases->nth(cases->first())->type_check(env);
    for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
        env.om->enterscope();
        tn = env.ct->lub(tn, cases->nth(i)->type_check(env));
        env.om->exitscope();
    }
    type = tn;
    return this;
}

Expression block_class::type_check(type_env_t env) {
    //cout << "In block_class\n";
    Symbol t1;
    for (int i = body->first(); body->more(i); i = body->next(i)) {
        t1 = body->nth(i)->type_check(env)->type;
    }
    type = t1;
    return this;
}

Expression let_class::type_check(type_env_t env) {
    //cout << "In let_class\n";
    Symbol t0 = type_decl;
    Symbol t1 = init->type_check(env)->type;
    // No init
    if (t1 == No_type) {
        env.om->enterscope();
        env.om->addid(identifier, &t0);
        Symbol t2 = body->type_check(env)->type;
        env.om->exitscope();
        type = t2;
    }
    // With init
    else {
        if (env.ct->is_child(t1, t0)) {
            env.om->enterscope();
            env.om->addid(identifier, &t0);
            Symbol t2 = body->type_check(env)->type;
            env.om->exitscope();
            type = t2;
        }
        else {
            ostream &err_stream = env.ct->semant_error(env.curr->get_filename(), this);
            err_stream << "Expression must evaluate to a child of " << t0 << ".\n";
            type = Object;
        }
    }
    return this;
}

Expression plus_class::type_check(type_env_t env) {
    Symbol t1 = e1->type_check(env)->type;
    Symbol t2 = e2->type_check(env)->type;
    if (t1 == Int && t2 == Int)
        type = Int;
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "non-Int arguments " << t1 << " + " << t2 << ".\n";
        type = Object;
    }
    return this;
}

Expression sub_class::type_check(type_env_t env) {
    Symbol t1 = e1->type_check(env)->type;
    Symbol t2 = e2->type_check(env)->type;
    if (t1 == Int && t2 == Int)
        type = Int;
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "non-Int arguments " << t1 << " - " << t2 << ".\n";
        type = Object;
    }
    return this;
}

Expression mul_class::type_check(type_env_t env) {
    Symbol t1 = e1->type_check(env)->type;
    Symbol t2 = e2->type_check(env)->type;
    if (t1 == Int && t2 == Int)
        type = Int;
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "non-Int arguments " << t1 << " * " << t2 << ".\n";
        type = Object;
    }
    return this;
}

Expression divide_class::type_check(type_env_t env) {
    Symbol t1 = e1->type_check(env)->type;
    Symbol t2 = e2->type_check(env)->type;
    if (t1 == Int && t2 == Int)
        type = Int;
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "non-Int arguments " << t1 << " / " << t2 << ".\n";
        type = Object;
    }
    return this;
}

Expression neg_class::type_check(type_env_t env) {
    Symbol t1 = e1->type_check(env)->type;
    if (t1 == Int)
        type = Int;
    else {
        ostream &err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "non-Int argument ~" << t1 << ".\n";
        type = Object;
    }
    return this;
}

Expression lt_class::type_check(type_env_t env) {
    Symbol t1 = e1->type_check(env)->type;
    Symbol t2 = e2->type_check(env)->type;
    if (t1 == Int && t2 == Int)
        type = Bool;
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "non-Int arguments " << t1 << " < " << t2 << ".\n";
        type = Object;
    }
    return this;
}

/*
 * Type check for e1 = e2
 * Any comparison is legal, except: if one argument is (Int || Str || Bool),
 * the other must match.
 */
Expression eq_class::type_check(type_env_t env) {
    Symbol t1 = e1->type_check(env)->type;
    Symbol t2 = e2->type_check(env)->type;
    if ((t1 == Int && t2 != Int) || (t1 != Int && t2 == Int) ||
        (t1 == Str && t2 != Str) || (t1 != Str && t2 == Str) ||
        (t1 == Bool && t2 != Bool) || (t1 != Bool && t2 == Bool)) {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "Cannot compare arguments " << t1 << " = " << t2 << ".\n";
        type = Object;
    }
    else {
        type = Bool;
    }
    return this;
}

Expression leq_class::type_check(type_env_t env) {
    Symbol t1 = e1->type_check(env)->type;
    Symbol t2 = e2->type_check(env)->type;
    if (t1 == Int && t2 == Int)
        type = Bool;
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "non-Int arguments " << t1 << " <= " << t2 << ".\n";
        type = Object;
    }
    return this;
}

Expression comp_class::type_check(type_env_t env) {
    Symbol t1 = e1->type_check(env)->type;
    if (t1 == Bool)
        type = Bool;
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "non-Bool argument !" << t1 << ".\n";
        type = Object;
    }
    return this;
}

Expression int_const_class::type_check(type_env_t env) {
    type = Int;
    return this;
}

Expression bool_const_class::type_check(type_env_t env) {
    type = Bool;
    return this;
}

Expression string_const_class::type_check(type_env_t env) {
    type = Str;
    return this;
}

Expression new__class::type_check(type_env_t env) {
    type = type_name;
    return this;
}

Expression isvoid_class::type_check(type_env_t env) {
    Symbol t1 = e1->type_check(env)->type;
    type = Bool;
    return this;
}

Expression no_expr_class::type_check(type_env_t env) {
    type = No_type;
    return this;
}

Expression object_class::type_check(type_env_t env) {
    if (name == self)
        type = SELF_TYPE;
    else if (env.om->lookup(name) != NULL)
        type = *(env.om->lookup(name));
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "Could not find identifier " << name << " in current scope.\n";
        type = Object;
    }
    return this;
}

