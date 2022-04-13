#include "IAKWrapper.h"

void IAKWrapper::memory_access(uint64_t addr) {
  requests.push_back({addr, requests.size() + living.size() + 1});

  if (requests.size() >= get_u()) {
    // std::cout << "requests chunk array:" << std::endl;
    // for (auto req : requests) {
    //   std::cout << req.first << "," << req.second << std::endl;
    // }

    process_requests();
  }
}

void print_result(MinInPlace::IAKOutput result) {
  std::cout << "living requests" << std::endl;
  for (auto living: result.living_requests) {
    std::cout << living.first << "," << living.second << " ";
  }
  std::cout << std::endl;

  std::cout << "depth vector: " << result.depth_vector.size() << std::endl;
  for (auto depth : result.depth_vector) {
    std::cout << depth << " ";
  }
  std::cout << std::endl;
}

void IAKWrapper::process_requests() {
  std::cout << std::endl;
  std::cout << "Processing chunk" << std::endl;
  std::cout << "Living requests " << living.size() << std::endl;
  std::cout << "New requests " << requests.size() << std::endl;

  std::cout << "CHUNK:" << std::endl;
  for (auto item : requests)
    std::cout << item.first << "," << item.second << std::endl;

  MinInPlace::IAKOutput result = iak_alg.get_depth_vector(living, requests);
  
  print_result(result);

  distance_histogram.resize(result.living_requests.size() + 1);

  // update depth_vector based upon living requests input to current chunk
	for (size_t i = 0; i < living.size(); i++) {
		result.depth_vector[i+1] += living.size()-i -1;
	}
  
  // Do not add living requests to distance histogram
  // 2-pointer walk through the depth vector and living requests
  // add to distance_histogram if access number not in living requests
  size_t living_idx = 0;
  for (size_t i = 0; i < result.depth_vector.size(); i++) {
		size_t depth = result.depth_vector[i];
    if (depth >= distance_histogram.size())
      std::cout << "depth: " << depth << "/" << distance_histogram.size() - 1 << std::endl;
    // assert(depth < distance_histogram.size());

    // If this index is killed than update distance histogram
    if (living_idx < result.living_requests.size() && i + 1 != result.living_requests[living_idx].second)
      distance_histogram[depth]++;
    else if (living_idx < result.living_requests.size())
      ++living_idx;
  }

  // Fix the index of the living requests so they count up from 1
  size_t num_living = result.living_requests.size();
  for (auto &living_req : result.living_requests)
    living_req.second = --num_living;

  living = result.living_requests;
  requests.clear();
  std::cout << "Size of depth vector = " << result.depth_vector.size() << std::endl;
  std::cout << "Number of surviving requests = " << living.size() << std::endl;
  std::cout << "First index of distance histogram = " << distance_histogram[1] << std::endl;
}

std::vector<size_t> IAKWrapper::get_success_function() {
  // Ensure all requests processed
  if (requests.size() > 0) {
    process_requests();
  }

  // TODO: parallel prefix sum for integrating
  uint64_t running_count = 0;
  std::vector<size_t> success_func(distance_histogram.size());
  for (uint64_t i = 1; i < distance_histogram.size(); i++) {
    running_count += distance_histogram[i];
    success_func[i] = running_count;
  }

  //for (auto& success : success_func)
  //  success /= running_count;

  return success_func;
}
