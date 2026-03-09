#include <gtest/gtest.h>

#include <apply_function.hpp>
#include <stdexcept>
#include <vector>

TEST(ApplyFunctionTest, AppliesTransformSingleThread) {
  std::vector<int> data{1, 2, 3, 4};

  ApplyFunction<int>(data, [](int& value) { value *= 2; }, 1);

  EXPECT_EQ(data, (std::vector<int>{2, 4, 6, 8}));
}

TEST(ApplyFunctionTest, AppliesTransformMultiThread) {
  std::vector<int> data{1, 2, 3, 4, 5, 6, 7, 8};

  ApplyFunction<int>(data, [](int& value) { value += 10; }, 3);

  EXPECT_EQ(data, (std::vector<int>{11, 12, 13, 14, 15, 16, 17, 18}));
}

TEST(ApplyFunctionTest, LimitsThreadCountByDataSize) {
  std::vector<int> data{5, 6};

  ApplyFunction<int>(data, [](int& value) { value *= value; }, 10);

  EXPECT_EQ(data, (std::vector<int>{25, 36}));
}

TEST(ApplyFunctionTest, HandlesZeroAndNegativeThreadCountAsSingleThread) {
  std::vector<int> data{3, 4, 5};

  ApplyFunction<int>(data, [](int& value) { value -= 1; }, 0);
  ApplyFunction<int>(data, [](int& value) { value -= 1; }, -7);

  EXPECT_EQ(data, (std::vector<int>{1, 2, 3}));
}

TEST(ApplyFunctionTest, HandlesEmptyVector) {
  std::vector<int> data;

  EXPECT_NO_THROW(ApplyFunction<int>(data, [](int& value) { value += 1; }, 4));
  EXPECT_TRUE(data.empty());
}

TEST(ApplyFunctionTest, RethrowsTransformException) {
  std::vector<int> data{1, 2, 3, 4};

  EXPECT_THROW(ApplyFunction<int>(
                   data,
                   [](int& value) {
                     if (value == 3) {
                       throw std::runtime_error("boom");
                     }
                     value += 1;
                   },
                   4),
               std::runtime_error);
}
