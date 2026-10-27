/**
 * golden.hpp
 * Author: Miguel Mochizuki Silva
 * Description: Header-only helper comparing text against golden files in tests/golden
 */
#ifndef SNAKE_TESTS_GOLDEN_HPP
#define SNAKE_TESTS_GOLDEN_HPP

#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

#ifndef SNAKE_GOLDEN_DIR
	#error "SNAKE_GOLDEN_DIR must be defined by the build"
#endif

/**
 * Compares actual against the golden file, or rewrites it when SNAKE_UPDATE_GOLDEN=1
 *
 * Parameters:
 * const std::string& name: Golden file name inside SNAKE_GOLDEN_DIR
 * const std::string& actual: Text produced by the code under test
 *
 * Returns void:
 */
inline void expectGolden(const std::string& name, const std::string& actual) {
	const std::string path = std::string(SNAKE_GOLDEN_DIR) + "/" + name;
	const char* update = std::getenv("SNAKE_UPDATE_GOLDEN");
	if (update != nullptr && std::string(update) == "1") {
		std::ofstream out(path, std::ios::binary | std::ios::trunc);
		ASSERT_TRUE(out.is_open()) << "cannot write golden file " << path;
		out << actual;
		return;
	}
	std::ifstream in(path, std::ios::binary);
	ASSERT_TRUE(in.is_open()) << "missing golden file " << path
		<< "; generate it with SNAKE_UPDATE_GOLDEN=1 ctest --test-dir build -R <test>";
	const std::string expected((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	EXPECT_EQ(expected, actual) << "golden mismatch: " << path;
}

#endif
