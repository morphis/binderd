/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gtest/gtest.h>

#include "binderd/client.h"
#include "binderd/server.h"
#include "binderd/logger.h"

#include "binder/Parcel.h"
#include "binder/Status.h"
#include "binder/IServiceManager.h"

#include "tests/common/benchmark.h"
#include "tests/common/standalone_server.h"

#include <thread>

#include <boost/filesystem.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/density.hpp>
#include <boost/accumulators/statistics/kurtosis.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/skewness.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>

/*
namespace fs = boost::filesystem;
namespace ba = boost::accumulators;
namespace bt= binderd::testing;

namespace {
typedef std::chrono::high_resolution_clock Clock;
typedef bt::Benchmark::Result::Timing::Seconds Resolution;

typedef ba::accumulator_set<Resolution::rep,
  ba::stats<ba::tag::count, ba::tag::min, ba::tag::max, ba::tag::mean, ba::tag::variance>> Statistics;

void fill_results_from_statistics(bt::Benchmark::Result& result, const Statistics& stats) {
    result.sample_size = ba::count(stats);
    result.timing.min = Resolution{static_cast<Resolution::rep>(ba::min(stats))};
    result.timing.max = Resolution{static_cast<Resolution::rep>(ba::max(stats))};
    result.timing.mean = Resolution{static_cast<Resolution::rep>(ba::mean(stats))};
    result.timing.std_dev = Resolution{static_cast<Resolution::rep>(std::sqrt(ba::variance(stats)))};
}

uint64_t now_ns() {
   struct timespec ts;
   memset(&ts, 0, sizeof(ts));
   clock_gettime(CLOCK_MONOTONIC, &ts);
   return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

uint64_t now_us() {
    return now_ns() / 1000;
}

class EndToEndBenchmark : public bt::Benchmark {
 public:
  struct MessageSendingConfiguration {
    std::uint32_t iterations{1000};
    std::size_t buffer_size{2048};
    StatisticsConfiguration statistics_configuration{};
  };

  bt::Benchmark::Result ForMessageSending(const MessageSendingConfiguration &config) {
    Statistics stats;
    bt::Benchmark::Result benchmark_result;

    server.async_run();

    for (auto n = 0; n < config.iterations; n++) {
      binderd::Client c1(server.socket_path());

      std::thread t1([this, config, n]() {
        binderd::Client c2(server.socket_path());

        auto msg = binderd::Message::create();
        msg->set_cookie(1234);

        // This is a bit lazy as we guess the handle of the other connection here
        const auto other_handle = 1 + n * 2;
        msg->set_destination(other_handle);

        auto writer = msg->add_item(binderd::Message::ItemType::Data);
        auto buffer = binderd::create_buffer_with_size(config.buffer_size);
        writer.write_sized_data(buffer->data(), buffer->size());

        const auto now = now_us();
        writer.write_uint64(now);

        c2.queue_message(msg);

        // FIXME Need a better way to ensure a message is sent
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
      });

      auto msg = c1.dequeue_message();

      const auto now = now_us();

      auto reader = msg->next_item();
      reader.read_sized_data();
      const auto t = reader.read_uint64();
      const auto diff = now - t;

      // Converting from microseconds to seconds as that is what the
      // sample stores internally.
      double seconds = static_cast<double>(diff);

      stats(seconds);

      t1.join();
    }

    fill_results_from_statistics(benchmark_result, stats);
    return benchmark_result;
  }

 private:
  bt::StandaloneServer server;
};
}

TEST(Performance, EndToEndIsAcceptable) {
  binderd::Log().Init(binderd::Logger::Severity::kDebug);

  bt::Benchmark::Result reference_result;

  EndToEndBenchmark benchmark;
  const EndToEndBenchmark::MessageSendingConfiguration config;
  auto result = benchmark.ForMessageSending(config);

  DEBUG("%s", result);
}
*/
