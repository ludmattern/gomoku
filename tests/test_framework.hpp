#pragma once
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <stdexcept>

namespace tfx
{

	struct TestDesc
	{
		const char *name;
		void (*fn)();
	};

	inline std::vector<TestDesc> &registry()
	{
		static std::vector<TestDesc> r;
		return r;
	}

	struct Registrar
	{
		Registrar(const char *name, void (*fn)())
		{
			registry().push_back({name, fn});
		}
	};

	struct Context
	{
		int failures = 0;
		bool verbose = false;
		bool printBoards = false;
	};

	inline Context *&currentCtxSlot()
	{
		static Context *ptr = nullptr;
		return ptr;
	}

	inline Context &ctx()
	{
		auto *p = currentCtxSlot();
		if (!p)
			throw std::runtime_error("No test context");
		return *p;
	}

} // namespace tfx

#define TEST(Name)                                              \
	static void test_fn_##Name();                               \
	static ::tfx::Registrar reg_##Name(#Name, &test_fn_##Name); \
	static void test_fn_##Name()

#define CHECK(Expr)                                                                          \
	do                                                                                       \
	{                                                                                        \
		if (!(Expr))                                                                         \
		{                                                                                    \
			::tfx::ctx().failures++;                                                         \
			std::cerr << "[CHECK FAIL] " << __FILE__ << ":" << __LINE__ << " : " #Expr "\n"; \
		}                                                                                    \
	} while (0)

#define REQUIRE(Expr)                                                                          \
	do                                                                                         \
	{                                                                                          \
		if (!(Expr))                                                                           \
		{                                                                                      \
			::tfx::ctx().failures++;                                                           \
			std::cerr << "[REQUIRE FAIL] " << __FILE__ << ":" << __LINE__ << " : " #Expr "\n"; \
			throw std::runtime_error("REQUIRE failed");                                        \
		}                                                                                      \
	} while (0)
