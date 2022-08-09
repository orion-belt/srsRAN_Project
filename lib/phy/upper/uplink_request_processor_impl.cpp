/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "uplink_request_processor_impl.h"
#include "srsgnb/phy/support/prach_buffer_pool.h"
#include "srsgnb/phy/upper/upper_phy_rx_symbol_request_notifier.h"

using namespace srsgnb;

uplink_request_processor_impl::uplink_request_processor_impl(
    upper_phy_rx_symbol_request_notifier& symbol_request_notifier,
    prach_buffer_pool&                    prach_memory_pool) :
  symbol_request_notifier(symbol_request_notifier), prach_memory_pool(prach_memory_pool)
{
}

void uplink_request_processor_impl::process_prach_request(const prach_buffer_context& context)
{
  // Grab a PRACH buffer from the pool.
  prach_buffer& buffer = prach_memory_pool.get_prach_buffer();

  // Notify the PRACH capture request event.
  symbol_request_notifier.on_prach_capture_request(context, buffer);
}