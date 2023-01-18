/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "lib/cu_cp/ue_manager.h"
#include "lib/ngap/ngap_asn1_helpers.h"
#include "lib/ngap/ngc_asn1_packer.h"
#include "unittests/ngap/test_helpers.h"
#include "srsgnb/gateways/sctp_network_gateway_factory.h"
#include "srsgnb/ngap/ngc_configuration_helpers.h"
#include "srsgnb/ngap/ngc_factory.h"
#include "srsgnb/support/async/async_test_utils.h"
#include "srsgnb/support/io_broker/io_broker_factory.h"
#include "srsgnb/support/test_utils.h"
#include "srsgnb/support/timers.h"
#include <gtest/gtest.h>

using namespace srsgnb;
using namespace srs_cu_cp;

/// This test is an integration test between:
/// * NGAP (including ASN1 packer and NG setup procedure)
/// * SCTP network gateway
/// * IO broker
class ngap_network_adapter : public ngc_message_notifier,
                             public ngc_message_handler,
                             public network_gateway_control_notifier,
                             public network_gateway_data_notifier
{
public:
  ngap_network_adapter(const sctp_network_gateway_config& nw_config_) :
    nw_config(nw_config_),
    epoll_broker(create_io_broker(io_broker_type::epoll)),
    gw(create_sctp_network_gateway({nw_config, *this, *this})),
    packer(*gw, *this)
  {
    gw->create_and_connect();
    epoll_broker->register_fd(gw->get_socket_fd(), [this](int fd) { gw->receive(); });
  }

  ~ngap_network_adapter() {}

  void connect_ngc(ngc_interface* ngc_) { ngc = ngc_; }

private:
  // NGAP calls interface to send (unpacked) NGAP PDUs
  void on_new_message(const ngc_message& msg) override { packer.handle_message(msg); }

  // SCTP network gateway calls interface to inject received PDUs (ASN1 packed)
  void on_new_pdu(byte_buffer pdu) override { packer.handle_packed_pdu(pdu); }

  // The packer calls this interface to inject unpacked NGAP PDUs
  void handle_message(const ngc_message& msg) override { ngc->handle_message(msg); }

  /// \brief Simply log those events for now
  void on_connection_loss() override { test_logger.info("on_connection_loss"); }
  void on_connection_established() override { test_logger.info("on_connection_established"); }

  /// We require a network gateway and a packer
  const sctp_network_gateway_config&    nw_config;
  std::unique_ptr<io_broker>            epoll_broker;
  std::unique_ptr<sctp_network_gateway> gw;
  ngc_asn1_packer                       packer;
  ngc_interface*                        ngc = nullptr;

  srslog::basic_logger& test_logger = srslog::fetch_basic_logger("TEST");
};

class ngap_integration_test : public ::testing::Test
{
protected:
  void SetUp() override
  {
    srslog::fetch_basic_logger("TEST").set_level(srslog::basic_levels::debug);
    srslog::init();

    cfg.gnb_id        = 411;
    cfg.ran_node_name = "srsgnb01";
    cfg.plmn          = "00101";
    cfg.tac           = 7;

    sctp_network_gateway_config nw_config;
    nw_config.connect_address   = "10.12.1.105";
    nw_config.connect_port      = 38412;
    nw_config.bind_address      = "10.8.1.10";
    nw_config.bind_port         = 0;
    nw_config.non_blocking_mode = true;
    adapter                     = std::make_unique<ngap_network_adapter>(nw_config);

    ngc_ue_task_scheduler = std::make_unique<dummy_ngc_ue_task_scheduler>(timers);

    ngc = create_ngc(cfg, *ngc_ue_task_scheduler, ue_mng, *adapter);
    adapter->connect_ngc(ngc.get());
  }

  ngc_configuration                            cfg;
  timer_manager                                timers;
  ue_manager                                   ue_mng;
  std::unique_ptr<dummy_ngc_ue_task_scheduler> ngc_ue_task_scheduler;
  std::unique_ptr<ngap_network_adapter>        adapter;
  std::unique_ptr<ngc_interface>               ngc;

  srslog::basic_logger& test_logger = srslog::fetch_basic_logger("TEST");
};

ng_setup_request generate_ng_setup_request(ngc_configuration ngc_cfg)
{
  ng_setup_request request_msg = {};
  fill_asn1_ng_setup_request(request_msg.msg, ngc_cfg.gnb_id, ngc_cfg.ran_node_name, ngc_cfg.plmn, ngc_cfg.tac);
  return request_msg;
}

/// Test successful ng setup procedure
TEST_F(ngap_integration_test, when_ng_setup_response_received_then_amf_connected)
{
  // Action 1: Launch NG setup procedure
  ngc_configuration ngc_cfg = srsgnb::config_helpers::make_default_ngc_config();

  ng_setup_request request_msg = generate_ng_setup_request(ngc_cfg);
  test_logger.info("Launching NG setup procedure...");
  async_task<ng_setup_response>         t = ngc->handle_ng_setup_request(request_msg);
  lazy_task_launcher<ng_setup_response> t_launcher(t);

  // Status: Procedure not yet ready.
  ASSERT_FALSE(t.ready());

  std::this_thread::sleep_for(std::chrono::seconds(3));
}
