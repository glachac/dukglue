#include "test_assert.h"
#include <dukglue.h>

#include <iostream>

// no return, no arguments (empty argument tuple, needs no stack access)
void test_no_args() {

}

// read a single value type
void test_one_arg(int a) {
	std::cout << "test(int a);" << std::endl;
}

// read multiple value types
void test_two_args(int a, const char* b) {
	std::cout << "test(int a, const char* b);" << std::endl;
}

// value-type return value
int square(int a) {
	return a * a;
}

// C-style string tests
const char* get_const_c_string() {
	return "butts_const";
}

// C++-style string tests
std::string get_cpp_string() {
	return std::string("potato");
}

const std::string& get_const_ref_cpp_string() {
	static const std::string str("potato_const_ref");
	return str;
}

// should NOT work
std::string& get_ref_cpp_string() {
	static std::string str("potato_ref");
	return str;
}

// should NOT work
std::string* get_ptr_cpp_string() {
	static std::string str("potato_ptr");
	return &str;
}

void test_primitives() {
	duk_context* ctx = duk_create_heap_default();

	dukglue_register_function(ctx, test_no_args, "test_no_args");
	test_eval(ctx, "test_no_args();");
	//test_eval_expect_error(ctx, "test_no_args(42);");

	dukglue_register_function(ctx, test_one_arg, "test_one_arg");
	test_eval(ctx, "test_one_arg(7);");
	test_eval_expect_error(ctx, "test_one_arg('butts');");

	dukglue_register_function(ctx, test_two_args, "test_two_args");
	test_eval(ctx, "test_two_args(2, 'butts');");
	//test_eval_expect_error(ctx, "test_two_args(2, 'butts', 4);");
	test_eval_expect_error(ctx, "test_two_args('butts', 2);");
	test_eval_expect_error(ctx, "test_two_args('butts');");
	test_eval_expect_error(ctx, "test_two_args(2);");
	test_eval_expect_error(ctx, "test_two_args();");

	dukglue_register_function(ctx, square, "square");
	test_eval_expect(ctx, "square(4)", 16);
	test_eval_expect_error(ctx, "square('potato')");

	dukglue_register_function(ctx, get_const_c_string, "get_const_c_string");
	test_eval_expect(ctx, "get_const_c_string()", "butts_const");

	dukglue_register_function(ctx, get_cpp_string, "get_cpp_string");
	test_eval_expect(ctx, "get_cpp_string()", "potato");

	dukglue_register_function(ctx, get_const_ref_cpp_string, "get_const_ref_cpp_string");
	test_eval_expect(ctx, "get_const_ref_cpp_string()", "potato_const_ref");

	// this shouldn't compile and give a sane error message ("Value types can only be returned as const references.")
	//dukglue_register_function(ctx, get_ref_cpp_string, "get_ref_cpp_string");

	// this shouldn't compile and give a sane error message ("Cannot return pointer to value type.")
	//dukglue_register_function(ctx, get_ptr_cpp_string, "get_ptr_cpp_string");

	duk_destroy_heap(ctx);

	std::cout << "Primitives tested OK" << std::endl;
}
