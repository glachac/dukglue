#include "test_assert.h"
#include <dukglue.h>

#include <iostream>

class Dog {
public:
	Dog(const std::string& name) : name_(name), bark_count_(0) {}

	~Dog() {
		std::cout << "puppy goes bye-bye" << std::endl;
	}

	// test const method
	// return const value reference
	const std::string& getName() const {
		return name_;
	}

	// test non-const method with member access
	void rename(const std::string& name) {
		name_ = name;
	}

	// test non-const method
	void bark(bool loud) {
		std::cout << (loud ? "WOOF WOOF" : "woof.") << std::endl;
		bark_count_++;
	}

	int getBarkCount() {
		return bark_count_;
	}

private:
	std::string name_;
	int bark_count_;
};

// need something that is not a Dog
class Goat {
};

void pokeWithStick(Dog* dog) {
	if(dog)
		dog->bark(true);
}

Dog* visitPuppy() {
	static Dog* puppy = new Dog("Sammie");
	return puppy;
}

void test_classes() {
	duk_context* ctx = duk_create_heap_default();

	// - dukglue_register_constructor with arguments
	// - dukglue_register_method (const and non-const)
	dukglue_register_constructor<Dog, const std::string&>(ctx, "Dog");
	dukglue_register_method(ctx, &Dog::getName, "getName");  // const method
	dukglue_register_method(ctx, &Dog::rename, "rename");
	dukglue_register_method(ctx, &Dog::bark, "bark");
	dukglue_register_method(ctx, &Dog::getBarkCount, "getBarkCount");

	// - test that script can actually call constructor + methods
	test_eval(ctx, "var test = new Dog('Gus');");
	test_eval_expect(ctx, "test.getName();", "Gus");
	test_eval(ctx, "test.rename('Archie');");
	test_eval_expect(ctx, "test.getName();", "Archie");

	// - test native objects as arguments to native functions
	dukglue_register_function(ctx, pokeWithStick, "pokeWithStick");
	test_eval(ctx, "pokeWithStick(test);");
	test_eval_expect(ctx, "test.getBarkCount();", 1);

	// - test type-safety when passing native types
	dukglue_register_constructor<Goat>(ctx, "Goat");
	dukglue_register_delete<Goat>(ctx);
	test_eval(ctx, "var goat = new Goat();");
	test_eval_expect_error(ctx, "pokeWithStick(goat);");
	test_eval(ctx, "goat.delete()");

	// - test that objects can be passed from C++ to script correctly
	dukglue_register_function(ctx, visitPuppy, "visitPuppy");
	test_eval(ctx, "var puppy = visitPuppy();");
	test_eval_expect(ctx, "puppy.getName()", "Sammie");
	test_eval(ctx, "pokeWithStick(puppy);");
	test_eval_expect(ctx, "puppy.getBarkCount()", 1);
	test_assert(visitPuppy()->getBarkCount() == 1);

	// - test that dynamic fields persist when passing the same native object twice
	test_eval(ctx, "puppy.dynamicfield = 4;");
	test_eval_expect(ctx, "puppy.dynamicfield", 4);
	test_eval(ctx, "puppy = null; puppy = visitPuppy();");
	test_eval_expect(ctx, "puppy.dynamicfield", 4);

	// - test that references are properly invalidated by obj.delete()
	dukglue_register_delete<Dog>(ctx);
	test_eval(ctx, "var myPuppy = new Dog('Squirt');");
	test_eval(ctx, "pokeWithStick(myPuppy);");
	test_eval(ctx, "myPuppy.delete();");
	test_eval_expect_error(ctx, "myPuppy.bark();");
	test_eval_expect_error(ctx, "pokeWithStick(myPuppy);");
	test_eval_expect_error(ctx, "myPuppy.delete();");
	test_eval(ctx, "test.delete()");

	duk_destroy_heap(ctx);

	std::cout << "Classes tested OK" << std::endl;
}
