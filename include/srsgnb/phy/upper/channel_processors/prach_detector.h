/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsgnb/adt/static_vector.h"
#include "srsgnb/phy/support/prach_buffer.h"
#include "srsgnb/ran/phy_time_unit.h"
#include "srsgnb/ran/prach/prach_preamble_format.h"
#include "srsgnb/ran/prach/restricted_set_config.h"
#include "srsgnb/ran/subcarrier_spacing.h"

namespace srsgnb {

/// \brief Describes a PRACH detector interface.
///
/// The PRACH sequence generation to detect is described in TS38.211 Section 6.3.3.
class prach_detector
{
public:
  /// Maximum number of preambles that can be detected in a slot.
  static constexpr unsigned MAX_NOF_PREAMBLES_PER_SLOT = 16;

  /// Describes the PRACH detector slot configuration for detecting PRACH.
  struct slot_configuration {
    /// Root sequence index. Possibles values are {0, ..., 837} for long preambles and {0, ..., 137} for short
    /// preambles.
    unsigned root_sequence_index;
    /// Preamble format.
    preamble_format format;
    /// Restricted set configuration.
    restricted_set_config restricted_set;
    /// Zero-correlation zone configuration index to calculate \f$N_{CS}\f$ as per TS38.211 section 6.3.3.1.
    unsigned zero_correlation_zone;
    /// Start preamble index to monitor. Possible values are {0, ..., 63}.
    unsigned start_preamble_index;
    /// Number of preamble indices to monitor. Possible values are {1, ..., 64}.
    unsigned nof_preamble_indices;
  };

  /// Describes the detection of a single preamble.
  struct preamble_indication {
    /// Indicates the detected preamble index. Possible values are {0, ..., 63}.
    unsigned preamble_index;
    /// Timing advance between the observed arrival time (for the considered UE) and the reference uplink time.
    phy_time_unit time_advance;
    /// Average RSRP value in dB, relative to 1.
    float power_dB;
    /// Average SNR value in dB.
    float snr_dB;
  };

  /// Describes a detection result.
  struct detection_result {
    /// Average RSSI value in dB, relative to 1.
    float rssi_dB;
    /// Detector time resolution. This is given by the DFT size of the detector and the PRACH subcarrier spacing.
    phy_time_unit time_resolution;
    /// List of detected preambles.
    static_vector<preamble_indication, MAX_NOF_PREAMBLES_PER_SLOT> preambles;
  };

  /// Default destructor.
  virtual ~prach_detector() = default;

  /// \brief Detects PRACH transmissions described by \c config.
  ///
  /// The PRACH detector assumes that \c signal is captured at the start of the PRACH reception window and it is at
  /// least as long as the PRACH preamble sequence.
  ///
  /// \param[in] signal Provides the baseband signal buffer.
  /// \param[in] config Provides the PRACH configuration for the slot.
  /// \return The detection result.
  virtual detection_result detect(const prach_buffer& input, const slot_configuration& config) = 0;
};

} // namespace srsgnb
