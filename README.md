# Running Unit Tests

## Prerequisites

First, ensure the project is built with test support:

### Release mode + no logs
```bash
cmake -B build-release-none -S . -DLOG_LEVEL=NONE -DCMAKE_BUILD_TYPE=Release
cmake --build build-release-none
```

Note: to run the tests with other configurations, it is recommended to use the build folder naming format as follows: build-<mode:release|debug>-<log_level:none|info|debug>

## Running Tests

### Option 1: Run Tests Directly

Execute the test binary directly:

```bash
./build-release-none/order_book_tests
```

This will run all tests and display results in the console.

### Option 2: Use CTest (Recommended)

CTest provides more control and integration with build systems:

```bash
# Run all tests
ctest --test-dir build

# Run with verbose output
ctest --test-dir build --verbose

# Run only specific test patterns
ctest --test-dir build -R "BookTest"

# Show output only on test failures
ctest --test-dir build --output-on-failure

# Run tests in parallel (if you have multiple test executables later)
ctest --test-dir build -j4
```

### Expected Output

When tests pass, you'll see output similar to:
```
Running main() from /path/to/googletest/src/gtest_main.cc
[==========] Running 5 tests from 2 test cases.
[----------] Global test environment set-up.
[----------] 3 tests from OrderBookTest
[ RUN      ] OrderBookTest.AddOrder
[       OK ] OrderBookTest.AddOrder (0 ms)
[ RUN      ] OrderBookTest.CancelOrder
[       OK ] OrderBookTest.CancelOrder (0 ms)
[ RUN      ] OrderBookTest.ModifyOrder
[       OK ] OrderBookTest.ModifyOrder (0 ms)
[----------] 3 tests from OrderBookTest (0 ms total)

[----------] 2 tests from MatchingEngineTest
[ RUN      ] MatchingEngineTest.BasicMatch
[       OK ] MatchingEngineTest.BasicMatch (0 ms)
[ RUN      ] MatchingEngineTest.PartialFill
[       OK ] MatchingEngineTest.PartialFill (0 ms)
[----------] 2 tests from MatchingEngineTest (0 ms total)

[==========] 5 tests from 2 test cases ran. (1 ms total)
[  PASSED  ] 5 tests.
```

If tests fail, the output will show `[  FAILED  ]` with details about what went wrong.

### Test Organization

- **Unit Tests**: Located in `tests/core/`, `tests/engine/`, `tests/utils/`
- **Integration Tests**: Located in `tests/integration/` (planned for future phases)
- **Test Discovery**: CTest automatically discovers all tests registered with `add_test()` in CMakeLists.txt

### Troubleshooting

- **Tests not found**: Ensure you've built the project with `cmake --build build`
- **Permission denied**: Make sure the test binary is executable: `chmod +x build/order_book_tests`
- **Missing dependencies**: If gtest fails to download, check your internet connection and CMake version (3.16+ required)
