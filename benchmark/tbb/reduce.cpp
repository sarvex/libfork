#include <numeric>
#include <span>
#include <vector>

#include "../bench.hpp"

#include <tbb/task_arena.h>
#include <tbb/task_group.h>

auto reduce(std::span<unsigned int const> x, std::size_t grain_size) -> unsigned int {
  //
  if (x.size() <= grain_size) {
    return std::reduce(x.begin(), x.end());
  }

  auto h = x.size() / 2;
  auto t = x.size() - h;

  unsigned int a, b;

  tbb::task_group g;

  g.run([&] {
    a = reduce(x.first(h), grain_size);
  });

  b = reduce(x.last(t), grain_size);

  g.wait();

  return a + b;
}

void run(std::string name, std::span<unsigned int> data) {
  //
  auto correct = std::reduce(data.begin(), data.end());

  benchmark(name, [&](std::size_t n, auto&& bench) {
    unsigned int res = 0;

    tbb::task_arena(n).execute([&] {
      bench([&] {
        res = reduce(data, data.size() / (10 * n));
      });
    });

    if (res != correct) {
      throw std::runtime_error("Incorrect result");
    }

    return res;
  });
}

auto main() -> int {
  std::vector<unsigned int> data(100'000'000);

  std::iota(data.begin(), data.end(), 0);

  run("tbb-reduce-1_000", std::span{data}.first(1'000));
  run("tbb-reduce-10_000", std::span{data}.first(10'000));
  run("tbb-reduce-100_000", std::span{data}.first(100'000));
  run("tbb-reduce-1_000_000", std::span{data}.first(1'000'000));
  run("tbb-reduce-10_000_000", std::span{data}.first(10'000'000));
  run("tbb-reduce-100_000_000", std::span{data}.first(100'000'000));

  return 0;
}