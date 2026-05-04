#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>
#include <iostream>
#include <chrono>
#include <random>
#include <format>
#include <algorithm>

int GetRandomInt() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    std::uniform_int_distribution<int> dist(0, 1000000);
    return dist(gen);
}
int main() {
  std::vector<int> nums;
  nums.resize(100);
  std::generate(nums.begin(), nums.end(), GetRandomInt);

  auto to_sub = GetRandomInt();

  auto start = std::chrono::steady_clock::now();
  for (auto& x : nums) {
    x -= to_sub;
  }
  auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();

  std::cout << std::format("Elapsed time: {} ns. \n", elapsed);
}