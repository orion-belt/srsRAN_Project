/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "dft_processor_test_data.h"
#include "srsgnb/phy/generic_functions/generic_functions_factories.h"
#include "srsgnb/srsvec/copy.h"
#include "srsgnb/support/test_utils.h"
#include "fmt/ostream.h"
#include <gtest/gtest.h>

using namespace srsgnb;

// Maximum allowed error.
static constexpr float ASSERT_MAX_ERROR = 4e-6;

namespace srsgnb {

std::ostream& operator<<(std::ostream& os, test_case_t test_case)
{
  fmt::print(os,
             "size={} direction={}",
             test_case.config.size,
             test_case.config.dir == dft_processor::direction::DIRECT ? "direct" : "inverse");
  return os;
}

static bool operator==(span<const cf_t> transform, span<const cf_t> dft_output)
{
  auto length = static_cast<float>(transform.size());
  return std::equal(transform.begin(),
                    transform.end(),
                    dft_output.begin(),
                    dft_output.end(),
                    [length](cf_t transform_val, cf_t dft_output_val) {
                      return (std::abs(transform_val - dft_output_val) / std::sqrt(length) <= ASSERT_MAX_ERROR);
                    });
}

} // namespace srsgnb

using DFTprocessorParams = std::tuple<std::string, test_case_t>;

class DFTprocessorFixture : public ::testing::TestWithParam<DFTprocessorParams>
{
protected:
  std::shared_ptr<dft_processor_factory> dft_factory = nullptr;

  void SetUp() override
  {
    // Call the required dft factory.
    std::string dft_factory_str = std::get<0>(GetParam());
    if (dft_factory_str == "generic") {
      dft_factory = create_dft_processor_factory_generic();
    } else if (dft_factory_str == "fftx") {
      // 'fftx' tests are silently skipped for now.
      GTEST_SKIP();
    } else if (dft_factory_str == "fftw") {
      dft_factory = create_dft_processor_factory_fftw();
      if (dft_factory == nullptr) {
        // 'fftw' tests are silently skipped if the libraries are missing.
        GTEST_SKIP();
      }
    }
    ASSERT_NE(dft_factory, nullptr);
  }
};

TEST_P(DFTprocessorFixture, DFTProcessorUnittest)
{
  const test_case_t& test_case = std::get<1>(GetParam());

  // Load the input data.
  std::vector<cf_t> input = test_case.data.read();

  // Load the golden data.
  std::vector<cf_t> transform = test_case.transform.read();

  // Create DFT processor configuration.
  dft_processor::configuration config = test_case.config;

  // Create processor.
  std::unique_ptr<dft_processor> dft = dft_factory->create(config);

  // Skip test case silently if it is not available.
  if (dft == nullptr) {
    GTEST_SKIP();
  }

  // Provide the input data to the DFT.
  srsvec::copy(dft->get_input().first(config.size), input);

  // Run DFT.
  span<const cf_t> dft_output = dft->run();

  // Assert shared channel data matches.
  ASSERT_EQ(transform, dft_output);
}

// Creates test suite that combines all possible parameters.
INSTANTIATE_TEST_SUITE_P(DFTprocessorVectorTest,
                         DFTprocessorFixture,
                         ::testing::Combine(::testing::Values("generic", "fftx", "fftw"),
                                            ::testing::ValuesIn(dft_processor_test_data)));