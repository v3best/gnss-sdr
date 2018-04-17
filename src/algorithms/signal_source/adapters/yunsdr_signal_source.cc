/*!
 * \filei yunsdr_signal_source.cc
 * \brief signal source for sdr hardware from V3 Inc based on YunSDR. 
 * \author V3
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */

#include "yunsdr_signal_source.h"
#include "configuration_interface.h"
#include "gnss_sdr_valve.h"
#include "GPS_L1_CA.h"
#include "GPS_L2C.h"
#include <glog/logging.h>
#include <iostream>


using google::LogMessage;

YunSDRSignalSource::YunSDRSignalSource(ConfigurationInterface* configuration,
    std::string role, unsigned int in_stream, unsigned int out_stream,
    boost::shared_ptr<gr::msg_queue> queue) : role_(role), in_stream_(in_stream), out_stream_(out_stream), queue_(queue)
{
    std::string default_item_type = "gr_complex";
    std::string default_dump_file = "./data/signal_source.dat";
    uri_ = configuration->property(role + ".device_address", std::string("192.168.1.10"));
    freq_ = configuration->property(role + ".freq", GPS_L1_FREQ_HZ);
    sample_rate_ = configuration->property(role + ".sampling_frequency", 2600000);
    bandwidth_ = configuration->property(role + ".bandwidth", 2000000);
    rx1_en_ = configuration->property(role + ".rx1_enable", true);
    rx2_en_ = configuration->property(role + ".rx2_enable", false);
    gain_mode_rx1_ = configuration->property(role + ".gain_mode_rx1", std::string("manual"));
    gain_mode_rx2_ = configuration->property(role + ".gain_mode_rx2", std::string("manual"));
    rf_gain_rx1_ = configuration->property(role + ".gain_rx1", 64.0);
    rf_gain_rx2_ = configuration->property(role + ".gain_rx2", 64.0);
    ref_clock_ = configuration->property(role + ".ref_clock", std::string("internal"));
    vco_ = configuration->property(role + ".vco", std::string("auxdac1"));
    buffer_size_ = configuration->property(role + ".buffer_size", 0xA0000);
    rf_port_select_ = configuration->property(role + ".rf_port_select", std::string("RX"));
    filter_file_ = configuration->property(role + ".filter_file", std::string(""));
    filter_auto_ = configuration->property(role + ".filter_auto", true);
    item_type_ = configuration->property(role + ".item_type", default_item_type);
    samples_ = configuration->property(role + ".samples", 0);
    dump_ = configuration->property(role + ".dump", false);
    dump_filename_ = configuration->property(role + ".dump_filename", default_dump_file);

    item_size_ = sizeof(gr_complex);

    std::cout << "device address: " << uri_ << std::endl;
    std::cout << "LO frequency : " << freq_ << " Hz" << std::endl;
    std::cout << "sample rate: " << sample_rate_ << " Hz" << std::endl;

    if (item_type_.compare("gr_complex") == 0)
        {
            if (rx1_en_ and rx2_en_)
                {
                    LOG(FATAL) << "Configuration error: both rx1 and rx2 are enabled!";
                }
            else
                {
                    yunsdr_source_ = gr::yunsdr::yunsdr_source::make(
                        uri_.c_str(), freq_, sample_rate_,
                        bandwidth_,
                        rx1_en_, rx2_en_,
                        gain_mode_rx1_.c_str(), rf_gain_rx1_,
                        gain_mode_rx2_.c_str(), rf_gain_rx2_,
                        ref_clock_, vco_,
                        buffer_size_,
                        rf_port_select_.c_str(), filter_file_.c_str(),
                        filter_auto_);
                }
        }
    else
        {
            LOG(FATAL) << "Configuration error: item type " << item_type_ << " not supported!";
        }

    if (samples_ != 0)
        {
            DLOG(INFO) << "Send STOP signal after " << samples_ << " samples";
            valve_ = gnss_sdr_make_valve(item_size_, samples_, queue_);
            DLOG(INFO) << "valve(" << valve_->unique_id() << ")";
        }

    if (dump_)
        {
            DLOG(INFO) << "Dumping output into file " << dump_filename_;
            file_sink_ = gr::blocks::file_sink::make(item_size_, dump_filename_.c_str());
            DLOG(INFO) << "file_sink(" << file_sink_->unique_id() << ")";
        }
}


YunSDRSignalSource::~YunSDRSignalSource()
{

}


void YunSDRSignalSource::connect(gr::top_block_sptr top_block)
{
    if (samples_ != 0)
        {
            top_block->connect(yunsdr_source_, 0, valve_, 0);
            DLOG(INFO) << "connected yunsdr source to valve";
            if (dump_)
                {
                    top_block->connect(valve_, 0, file_sink_, 0);
                    DLOG(INFO) << "connected valve to file sink";
                }
        }
    else
        {
            if (dump_)
                {
                    top_block->connect(yunsdr_source_, 0, file_sink_, 0);
                    DLOG(INFO) << "connected yunsdr source to file sink";
                }
        }
}


void YunSDRSignalSource::disconnect(gr::top_block_sptr top_block)
{
    if (samples_ != 0)
        {
            top_block->disconnect(yunsdr_source_, 0, valve_, 0);
            if (dump_)
                {
                    top_block->disconnect(valve_, 0, file_sink_, 0);
                }
        }
    else
        {
            if (dump_)
                {
                    top_block->disconnect(yunsdr_source_, 0, file_sink_, 0);
                }
        }
}


gr::basic_block_sptr YunSDRSignalSource::get_left_block()
{
    LOG(WARNING) << "Trying to get signal source left block.";
    return gr::basic_block_sptr();
}


gr::basic_block_sptr YunSDRSignalSource::get_right_block()
{
    if (samples_ != 0)
        {
            return valve_;
        }
    else
        {
            return (yunsdr_source_);
        }
}
