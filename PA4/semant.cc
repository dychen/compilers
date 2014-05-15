

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
        ostream& err_stream = semant_error(c);
        err_stream << "Class " << name << " cannot inherit class " << parent << ".\n"; 
    }
    else if (name == SELF_TYPE) {
        ostream& err_stream = semant_error(c);
        err_stream << "Redefinition of basic class " << name << ".\n";
    }
    else if (class_map.count(name) == 0 && inheritance_graph.count(name) == 0) {
        class_map[name] = c;
        inheritance_graph[name] = parent;
    }
    else {
        ostream& err_stream = semant_error(c);
        err_stream << "Class " << name << " has already been defined.\n";
    }
}

/*
 * Validates the inheritance graph by checking that:
 *   1. Every parent class is defined.
 *   2. There are no cycles.
 *   3. The class Main is defined.
 */
bool ClassTable::is_valid() {
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
                return false;
            }
            else if (inheritance_graph.count(parent) == 0) {
                // Error - parent not found
                ostream& err_stream = semant_error(class_map[child]);
                err_stream << "Class " << child << " inherits from undefined class "
                           << parent << ".\n";
                return false;
            }
            else
                parent = inheritance_graph[parent];
        }
    } 
    if (is_main_defined == false) {
        ostream& err_stream = semant_error();
        err_stream << "Class Main is not defined.\n";
        return false;
    }
    return true;
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

bool ClassTable::class_exists(Symbol c) {
    return (inheritance_graph.count(c) > 0);
}

Class_ ClassTable::get_class(Symbol class_name) {
    return class_map[class_name];
}

/*
 * Traverses the inheritance chain of the input class until
 * it finds a method matching the input method name and returns
 * the corresponding formal parameters. Returns NULL if no matching
 * method is found.
 */
Formals ClassTable::get_formals(Symbol class_name, Symbol method_name) {
    Symbol cname = class_name;
    while (cname != No_class) {
        Class_ c = class_map[cname];
        Formals f = c->get_formals(method_name);
        if (f != NULL)
            return f;
        cname = inheritance_graph[cname]; 
    }
    return NULL;
}

/*
 * Traverses the inheritance chain of the input class until
 * it finds a method matching the input method name and returns
 * the corresponding formal parameters. Returns NULL if no matching
 * method is found.
 */
Symbol ClassTable::get_return_type(Symbol class_name, Symbol method_name) {
    Symbol cname = class_name;
    while (cname != No_class) {
        Class_ c = class_map[cname];
        Symbol r = c->get_return_type(method_name);
        if (r != NULL)
            return r;
        cname = inheritance_graph[cname];
    }
    return NULL;
}

/*
 * Traverses the inheritance chain of the input class, starting with
 * the parent, until it finds a method matching the input method name
 * and returns the corresponding class name. Returns NULL if no matching
 * method is found. This is a kind of hacky solution that uses get_return_type,
 * which works because get_return_type <=> method exists in that class.
 */
Symbol ClassTable::get_ancestor_method_class(Symbol class_name, Symbol method_name) {
    Symbol cname = inheritance_graph[class_name];
    while (cname != No_class) {
        Class_ c = class_map[cname];
        if (c->get_return_type(method_name) != NULL)
            return c->get_name();
        cname = inheritance_graph[cname];
    }
    return NULL;
}

/*
 * Checks the method signature of the input method for the two classes.
 * Returns true if the signatures are the same; specifically, if the
 * following hold:
 * - Formal parameters have the same types in the same order
 * - Number of formal parameters are the same
 * - Return types match
 * Does NOT ensure that the formal parameters have the same identifiers.
 */
bool ClassTable::check_method_signature(Symbol c1, Symbol c2, Symbol method_name) {
    Class_ class1 = class_map[c1];
    Class_ class2 = class_map[c2];
    Formals f1 = class1->get_formals(method_name);
    Formals f2 = class2->get_formals(method_name);
    Symbol ret1 = class1->get_return_type(method_name);
    Symbol ret2 = class2->get_return_type(method_name);
    int i = f1->first();
    int j = f2->first();
    // Check formals
    while (f1->more(i) && f2->more(j)) {
        if (f1->nth(i)->get_type() != f2->nth(j)->get_type())
            return false;
        i = f1->next(i);
        j = f2->next(j);
    }
    if (f1->more(i) || f2->more(j))
        return false;
    // Check return type
    if (ret1 != ret2)
        return false;
    return true;
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
    if (!classtable->errors() && classtable->is_valid()) {
        type_env_t env;
        env.om = new SymbolTable<Symbol, Symbol>();
        env.curr = NULL;
        env.ct = classtable;

        /* Recurisvely type check each class. */
        for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
            env.om->enterscope();
            env.curr = classes->nth(i);
            classes->nth(i)->init_class(env); // So the attributes are global
                                              // in the class environment/scope
            classes->nth(i)->type_check(env);
            env.om->exitscope();
        }
    }

    if (classtable->errors()) {
	cerr << "Compilation halted due to static semantic errors." << endl;
	exit(1);
    }
}

/*
 * Other semantic analysis helper methods defined in cool-tree.h.
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

/*
 * Simple accessors for classes defined in cool-tree.h.
 */
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

Symbol formal_class::get_type() { return type_decl; }
Symbol branch_class::get_type() { return type_decl; }

/*
 * Top-most step in recursive type checking. Recursively checks each of the
 * features (methods and attributes). Does not impose any type restrictions.
 */
Class_ class__class::type_check(type_env_t env) {
    for (int i = features->first(); features->more(i); i = features->next(i)) {
        features->nth(i)->type_check(env);
    }
    return this;
}

/*
 * Recursively checks formal parameters for the method as follows:
 * 1. In a new environment scope, bind the keyword self.
 * 2. Recursively check the formal parameters in this environment.
 * 3. For the resulting parameter types:
 *    - If the method is inherited, make sure the ancestor method is properly overwritten.
 *    - Declared return value of SELF_TYPE should return SELF_TYPE.
 *    - Make sure the return type is a defined class (exists in the class table).
 *    - Make sure the return type is a subtype of the declared return type.
 */
Feature method_class::type_check(type_env_t env) {
    env.om->enterscope();
    Symbol curr_class = env.curr->get_name();
    env.om->addid(self, &curr_class);
    for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
        formals->nth(i)->type_check(env);
    }
    Symbol tret = expr->type_check(env)->type;

    Symbol ancestor = NULL;
    if ((ancestor = env.ct->get_ancestor_method_class(curr_class, name)) != NULL) {
        if (!env.ct->check_method_signature(ancestor, curr_class, name)) {
            ostream &err_stream = env.ct->semant_error(env.curr->get_filename(), this);
            err_stream << "Overriding method signature of " << name << " for class "
                       << curr_class << " doesn't match method signature for ancestor "
                       << ancestor << ".\n";
        }
    }

    if (return_type == SELF_TYPE) {
        if (tret != SELF_TYPE) {
            ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
            err_stream << "Inferred return type " << tret << " of method " << name
                       << " does not conform to declared return type " << return_type << ".\n";
        }
    }
    else if (!env.ct->class_exists(return_type)) {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "Undefined return type " << return_type << " in method " << name << ".\n";
    }
    else {
        if (tret == SELF_TYPE)
            tret = env.curr->get_name();
        if (!env.ct->is_child(tret, return_type)) {
            ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
            err_stream << "Method initialization " << tret
                       << " is not a subclass of " << return_type << ".\n";
        }
    }
    env.om->exitscope();
    return this;
}

/*
 * Type checks a class attribute as follows:
 * 1. In a new environment scope, bind the keyword self.
 * 2. Evaluate the initialization of the attribute.
 * 3. Make sure the initialized type is a subclass of the declared type.
 */
Feature attr_class::type_check(type_env_t env) {
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
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "'self' cannot be the name of an attribute.\n";
    }
    return this;
}

/*
 * Type checks a formal parameter by making sure it hasn't already been defined in the
 * current scope.
 */
Formal formal_class::type_check(type_env_t env) {
    if (env.om->probe(name) != NULL) {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "Duplicate formal " << name << ".\n";
    }
    else if (type_decl == SELF_TYPE) {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "Formal parameter " << name << " cannot have type SELF_TYPE\n";
    }
    else
        env.om->addid(name, &type_decl);
    return this;
}

/* Adds the branch to the environment. See typcase_class for more details. */
Symbol branch_class::type_check(type_env_t env) {
    // The following condition should never be true because a branch identifier
    // is the first thing initialized in its own scope. This is just a sanity check.
    if (env.om->probe(name) != NULL) {
        ostream &err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "Identifier " << name << " already defined in current scope.\n";
        return Object;
    }
    env.om->addid(name, &type_decl);
    return expr->type_check(env)->type;
}

/*
 * Adds the identifier to the environment after checking its evaulated type
 * and making sure that's a subclass of the declared type.
 */
Expression assign_class::type_check(type_env_t env) {
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

/*
 * Same as normal dispatch (see dispatch_class::type_check(type_env_t env) below),
 * with the additional condition that the evaluated type of the calling expression e
 * must be a subtype of the static class T in e@T.f(...).
 */
Expression static_dispatch_class::type_check(type_env_t env) {
    std::vector<Symbol> eval_types; // Vector of parameter types after evaluation
    Symbol t0 = expr->type_check(env)->type;
    if (t0 == SELF_TYPE)
        t0 = env.curr->get_name();
    if (env.ct->is_child(t0, type_name)) {
        for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
            Symbol tn = actual->nth(i)->type_check(env)->type;
            if (tn == SELF_TYPE)
                tn = env.curr->get_name();
            eval_types.push_back(tn);
        }
        Formals formals = env.ct->get_formals(t0, name);     // Declared formal types
        Symbol ret_type = env.ct->get_return_type(t0, name); // Declared return type
        if (formals == NULL || ret_type == NULL) {
            ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
            err_stream << "Dispatch to undefined method " << name << ".\n";
            type = Object;
            return this;
        }
        // Type check formal parameters
        std::vector<Symbol>::iterator iter = eval_types.begin();
        int fi = formals->first();
        while (iter != eval_types.end() && formals->more(fi)) {
            Symbol eval_type = *iter;
            Symbol declared_type = formals->nth(fi)->get_type();
            if (declared_type == SELF_TYPE) {
                ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
                err_stream << "Formal parameter cannot have type SELF_TYPE.\n";
            }
            else if (!env.ct->is_child(eval_type, declared_type)) {
                ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
                err_stream << "Formal parameter declared type " << declared_type
                           << " is not a subclass of " << eval_type << ".\n";
            }
            ++iter;
            fi = formals->next(fi);
        }
        if (iter != eval_types.end() || formals->more(fi)) {
            // If we're here, means the number of parameters didn't match
            // the expected number from the function definition. This should
            // not be possible as long as lexing and parsing is correct.
            ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
            err_stream << "Number of declared formals doesn't match number checked.\n";
        }

        if (ret_type == SELF_TYPE)
            type = t0;
        else
            type = ret_type;
    }
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "Evaluated class " << t0 << " must be a child of declared class "
                   << type_name << " in static dispatch.\n";
        type = Object;
    }
    return this;
}

/*
 * Given the following expression: e.f(e1, ..., en), performs the following check:
 * 1. Recursively checks e.
 * 2. Recursively checks e1, ..., en.
 * 3. Makes sure that the evaluated types of e1, ..., en are subtypes of the types
 *    declared in the method signature.
 * 4. Returns:
 *    - The evaluated type if the declared type is SELF_TYPE.
 *    - The declared type otherwise.
 */
Expression dispatch_class::type_check(type_env_t env) {
    std::vector<Symbol> eval_types; // Vector of parameter types after evaluation
    Symbol t0 = expr->type_check(env)->type;
    Symbol curr = t0;
    if (t0 == SELF_TYPE)
        curr = env.curr->get_name();
    for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
        Symbol tn = actual->nth(i)->type_check(env)->type;
        if (tn == SELF_TYPE)
            tn = env.curr->get_name();
        eval_types.push_back(tn);
    }
    Formals formals = env.ct->get_formals(curr, name);     // Declared formal types
    Symbol ret_type = env.ct->get_return_type(curr, name); // Declared return type
    if (formals == NULL || ret_type == NULL) {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "Dispatch to undefined method " << name << ".\n";
        type = Object;
        return this;
    }
 
    // Type check formal parameters
    std::vector<Symbol>::iterator iter = eval_types.begin();
    int fi = formals->first();
    while (iter != eval_types.end() && formals->more(fi)) {
        Symbol eval_type = *iter;
        Symbol declared_type = formals->nth(fi)->get_type();
        if (declared_type == SELF_TYPE) {
            ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
            err_stream << "Formal parameter cannot have type SELF_TYPE.\n";
        }
        else if (!env.ct->is_child(eval_type, declared_type)) {
            ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
            err_stream << "Formal parameter declared type " << declared_type
                       << " is not a subclass of " << eval_type << ".\n";
        }
        ++iter;
        fi = formals->next(fi);
    }
    if (iter != eval_types.end() || formals->more(fi)) {
        // If we're here, means the number of parameters didn't match
        // the expected number from the function definition. This should
        // not be possible as long as lexing and parsing is correct.
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "Number of declared formals doesn't match number checked.\n";
    }

    if (ret_type == SELF_TYPE)
        type = t0;    
    else
        type = ret_type;
    return this;
}

/*
 * Recursively type checks the predicate, then the then expression, then the
 * else expression. Makes sure the predicate evaluates to Bool and returns
 * the least upper bound of the types of the other two expressions.
 */
Expression cond_class::type_check(type_env_t env) {
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

/* Recurisvely type checks the predicate and body, makes sure predicate is a Bool. */
Expression loop_class::type_check(type_env_t env) {
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

/*
 * Type checks a linked list of branch/case expressions in the followin format:
 * case e0 of x1:T1=>e1, ..., xn:Tn=>en esac
 * For each branch i, the current identifier xi is saved in a new scope and the
 * expression ei is evaluated. The typcase class evaluates the the least upper
 * bound of the evaluated types.
 */
Expression typcase_class::type_check(type_env_t env) {
    Symbol t0 = expr->type_check(env)->type;

    // O(N^2) check to make sure there are no duplicate types.
    for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
        for (int j = cases->first(); cases->more(j); j = cases->next(j)) {
            if (i != j && cases->nth(i)->get_type() == cases->nth(j)->get_type()) {
                ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
                err_stream << "Duplicate branch " << cases->nth(i)->get_type()
                           << " in case statement.\n";
                type = Object;
                return this;
            }
        }
    }

    Symbol tn = cases->nth(cases->first())->type_check(env);
    for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
        if (i != cases->first()) {
            env.om->enterscope();
            tn = env.ct->lub(tn, cases->nth(i)->type_check(env));
            env.om->exitscope();
        }
    }
    type = tn;
    return this;
}

/* Type checks each enclosing expression. Imposes no type conditions. */
Expression block_class::type_check(type_env_t env) {
    Symbol t1;
    for (int i = body->first(); body->more(i); i = body->next(i)) {
        t1 = body->nth(i)->type_check(env)->type;
    }
    type = t1;
    return this;
}

/*
 * Given the expression: let x:T0[<-e1] in e2:T2,
 * 1. Evaluates the initializing expression e0 if provided.
 * 2. If e0 is provided, makes sure the evaluated type T1 is a child of the
 *    declared type T0.
 * 3. Enters a new environment scope with the identifier x bound to the declared
 *    type T0.
 * 4. Typechecks the body e2, and the type evaluates to the evaluated type T2.
 */
Expression let_class::type_check(type_env_t env) {
    if (identifier == self) {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "'self' cannot be bound in a 'let' expression.\n";
        type = Object;
    }
    else {
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
                ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
                err_stream << "Expression must evaluate to a child of " << t0 << ".\n";
                type = Object;
            }
        }
    }
    return this;
}

/*
 * The following arithmetic classes are self-explanatory:
 * +, -, *, /, ~, <, =, !
 */
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
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
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

Expression eq_class::type_check(type_env_t env) {
    Symbol t1 = e1->type_check(env)->type;
    Symbol t2 = e2->type_check(env)->type;
    // Any comparison is legal, except: if one argument is in {Int, Str, Bool},
    // the other must match.
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

/* Primitive constants */
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
    if (env.ct->class_exists(type_name) || type_name == SELF_TYPE)
        type = type_name;
    else {
        ostream& err_stream = env.ct->semant_error(env.curr->get_filename(), this);
        err_stream << "'new' used with undefined class " << type_name << ".\n";
        type = Object;
    }
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

