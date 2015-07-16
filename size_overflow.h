#ifndef SIZE_OVERFLOW_H
#define SIZE_OVERFLOW_H

#define CREATE_NEW_VAR NULL_TREE
#define CANNOT_FIND_ARG 32
#define MAX_PARAM 31
#define BEFORE_STMT true
#define AFTER_STMT false

#define TURN_OFF_ASM_STR "# size_overflow MARK_TURN_OFF "
#define YES_ASM_STR "# size_overflow MARK_YES "
#define OK_ASM_STR "# size_overflow "

enum mark {
	MARK_NO, MARK_YES, MARK_NOT_INTENTIONAL, MARK_TURN_OFF
};

enum intentional_overflow_type {
	NO_INTENTIONAL_OVERFLOW, RHS1_INTENTIONAL_OVERFLOW, RHS2_INTENTIONAL_OVERFLOW
};


#if BUILDING_GCC_VERSION >= 5000
typedef struct hash_set<const_gimple> gimple_set;

static inline bool pointer_set_insert(gimple_set *visited, const_gimple stmt)
{
	return visited->add(stmt);
}

static inline bool pointer_set_contains(gimple_set *visited, const_gimple stmt)
{
	return visited->contains(stmt);
}

static inline gimple_set* pointer_set_create(void)
{
	return new hash_set<const_gimple>;
}

static inline void pointer_set_destroy(gimple_set *visited)
{
	delete visited;
}

typedef struct hash_set<tree> tree_set;

static inline bool pointer_set_insert(tree_set *visited, tree node)
{
	return visited->add(node);
}

static inline bool pointer_set_contains(tree_set *visited, tree node)
{
	return visited->contains(node);
}

static inline tree_set *tree_pointer_set_create(void)
{
	return new hash_set<tree>;
}

static inline void pointer_set_destroy(tree_set *visited)
{
	delete visited;
}
#else
typedef struct pointer_set_t gimple_set;
typedef struct pointer_set_t tree_set;

static inline tree_set *tree_pointer_set_create(void)
{
	return pointer_set_create();
}
#endif

struct visited {
	gimple_set *stmts;
	gimple_set *my_stmts;
	gimple_set *skip_expr_casts;
	gimple_set *no_cast_check;
};

// size_overflow_plugin.c
extern tree report_size_overflow_decl;
extern tree size_overflow_type_HI;
extern tree size_overflow_type_SI;
extern tree size_overflow_type_DI;
extern tree size_overflow_type_TI;


// size_overflow_plugin_hash.c
struct size_overflow_hash {
	const struct size_overflow_hash * const next;
	const char * const name;
	const unsigned int param;
};

struct interesting_node {
	struct interesting_node *next;
	gimple first_stmt;
	const_tree fndecl;
	tree node;
#if BUILDING_GCC_VERSION <= 4007
	VEC(tree, gc) *last_nodes;
#else
	vec<tree, va_gc> *last_nodes;
#endif
	unsigned int num;
	enum mark intentional_attr_decl;
	enum mark intentional_attr_cur_fndecl;
	gasm *intentional_mark_from_gimple;
};

extern bool is_size_overflow_asm(const gasm *stmt);
extern unsigned int get_function_num(const_tree node, const_tree orig_fndecl);
extern unsigned int get_correct_arg_count(unsigned int argnum, const_tree fndecl);
extern bool is_missing_function(const_tree orig_fndecl, unsigned int num);
extern bool is_a_return_check(const_tree node);
extern const struct size_overflow_hash *get_function_hash(const_tree fndecl);
extern unsigned int find_arg_number_tree(const_tree arg, const_tree func);


// size_overflow_debug.c
extern struct opt_pass *make_dump_pass(void);


// intentional_overflow.c
extern enum mark get_intentional_attr_type(const_tree node);
extern bool is_size_overflow_intentional_asm_yes(const gasm *stmt);
extern bool is_size_overflow_intentional_asm_turn_off(const gasm *stmt);
extern bool is_end_intentional_intentional_attr(const_tree decl, unsigned int argnum);
extern bool is_yes_intentional_attr(const_tree decl, unsigned int argnum);
extern bool is_turn_off_intentional_attr(const_tree decl);
extern void print_missing_intentional(enum mark callee_attr, enum mark caller_attr, const_tree decl, unsigned int argnum);
extern void check_intentional_attribute_ipa(struct interesting_node *cur_node);
extern bool is_a_cast_and_const_overflow(const_tree no_const_rhs);
extern bool is_const_plus_unsigned_signed_truncation(const_tree lhs);
extern bool is_a_constant_overflow(const gassign *stmt, const_tree rhs);
extern tree handle_intentional_overflow(struct visited *visited, struct cgraph_node *caller_node, bool check_overflow, gassign *stmt, tree change_rhs, tree new_rhs2);
extern tree handle_integer_truncation(struct visited *visited, struct cgraph_node *caller_node, const_tree lhs);
extern bool is_a_neg_overflow(const_gimple stmt, const_tree rhs);
extern enum intentional_overflow_type add_mul_intentional_overflow(const gassign *def_stmt);
extern void unsigned_signed_cast_intentional_overflow(struct visited *visited, gassign *stmt);


// insert_size_overflow_check_ipa.c
extern unsigned int search_function(void);
extern unsigned int call_count;
extern struct opt_pass *make_insert_size_overflow_check(void);
extern const_tree get_interesting_orig_fndecl(const_gimple stmt, unsigned int argnum);


// insert_size_overflow_asm.c
extern struct opt_pass *make_insert_size_overflow_asm_pass(void);


// misc.c
extern void set_current_function_decl(tree fndecl);
extern void unset_current_function_decl(void);
extern tree get_lhs(const_gimple stmt);
extern gimple get_def_stmt(const_tree node);
extern tree create_new_var(tree type);
extern gimple build_cast_stmt(struct visited *visited, tree dst_type, tree rhs, tree lhs, gimple_stmt_iterator *gsi, bool before, bool force);
extern bool skip_types(const_tree var);
extern tree cast_a_tree(tree type, tree var);
extern bool is_size_overflow_type(const_tree var);


// insert_size_overflow_check_core.c
extern tree expand(struct visited *visited, struct cgraph_node *caller_node, tree lhs);
extern void check_size_overflow(struct cgraph_node *caller_node, gimple stmt, tree size_overflow_type, tree cast_rhs, tree rhs, bool before);
extern tree dup_assign(struct visited *visited, gassign *oldstmt, const_tree node, tree rhs1, tree rhs2, tree __unused rhs3);
extern tree create_assign(struct visited *visited, gimple oldstmt, tree rhs1, bool before);


// remove_unnecessary_dup.c
extern struct opt_pass *make_remove_unnecessary_dup_pass(void);
extern void insert_cast_expr(struct visited *visited, gassign *stmt, enum intentional_overflow_type type);
extern bool skip_expr_on_double_type(const gassign *stmt);
extern void create_up_and_down_cast(struct visited *visited, gassign *use_stmt, tree orig_type, tree rhs);

#endif
