// Copyright (c) 2019 Luigi Pertoldi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <vector>
#include <chrono>
#include <iostream>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <cassert>
#include <limits>

#ifndef _BAYESIAN_BLOCKS_HH
#define _BAYESIAN_BLOCKS_HH

namespace BayesianBlocks {

    // handy aliases
    namespace bb {
        // data containers
        using array         = std::vector<double>;
        using data_array    = std::vector<double>;
        using weights_array = std::vector<int>;
        using pair          = std::pair<double, int>;

        // time
        using clock = std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using us = std::chrono::microseconds;
    }

    // core utility
    bb::array blocks(bb::data_array data, bb::weights_array weights, const double p = 0.01,
                     bool counter = false, bool benchmark = false);

    bb::array blocks(bb::data_array data, const double p = 0.01,
                     bool counter = false, bool benchmark = false);
}

namespace BayesianBlocks {

    // core utility
    bb::array blocks(bb::data_array data, bb::weights_array weights, const double p,
                     bool counter, bool benchmark) {

        auto start = bb::clock::now();

        // sanity checks
        if (data.size() != weights.size()) {
            throw std::domain_error("ERROR: data and weights vectors are of different sizes");
        }

        if (data.size() == 0) {
            throw std::invalid_argument("ERROR: empty arrays provided as input");
        }

        if (std::find_if(weights.begin(), weights.end(),
                         [](int& v) { return v <= 0; }) != weights.end()) {
            throw std::domain_error("ERROR: invalid weights found in input");
        }

        if (std::unique(data.begin(), data.end()) != data.end()) {
            throw std::invalid_argument("ERROR: duplicated values found in input");
        }

        const auto N = data.size();

        // sort and copy data
        std::vector<bb::pair> hist; hist.reserve(N);
        for (std::size_t i = 0; i < N; ++i) hist.emplace_back(data[i], weights[i]);
        std::sort(hist.begin(), hist.end(), [](bb::pair a, bb::pair b) { return a.first < b.first; });
        for (size_t i = 0; i < N; ++i) {
            data[i]    = hist[i].first;
            weights[i] = hist[i].second;
        }

        // build up array with all possible bin edges
        bb::array edges(N+1);
        edges[0]   = data[0];
        for (std::size_t i = 0; i < N-1; ++i) edges[i+1] = (data[i]+data[i+1])/2.;
        edges[N]   = data[N-1];

        assert(std::unique(edges.begin(), edges.end()) == edges.end());

        // let's use here Cash statistics and calibrated prior on number of change points
        auto cash = [](int N_k, double T_k) { return N_k * std::log(N_k/T_k); };
        auto ncp_prior = std::log(73.53 * p * std::pow(N, -0.478)) - 4;

        // arrays to store results
        bb::array last(N);
        bb::array best(N);

        auto init_time = bb::duration_cast<bb::us>(bb::clock::now() - start).count();
        start = bb::clock::now();

        // do the actual recursive computation
        for (std::size_t k = 0; k < N; ++k ) {
            bb::array A(k+1);
            for (std::size_t r = 0; r == 0 or r <= k; ++r) {
                A[r] = cash(
                            std::accumulate(weights.begin()+r, weights.begin()+k+1, 0),
                            edges[k+1] - edges[r]
                       ) + ncp_prior + (r == 0 ? 0 : best[r-1]);
            }
            last[k] = std::distance(A.begin(), std::max_element(A.begin(), A.end()));
            best[k] = *(std::max_element(A.begin(), A.end()));

            if (counter) std::cout << '\r' << k << '/' << N << std::flush;
        }
        if (counter) std::cout << std::endl;

        auto loop_time = bb::duration_cast<bb::us>(bb::clock::now() - start).count();
        start = bb::clock::now();

        // iteratively find the change points
        std::vector<int> cp;
        for (auto i = N; i != 0; i = last[i-1]) cp.push_back(i);
        cp.push_back(0);

        std::reverse(cp.begin(), cp.end());
        bb::array result(cp.size(), 0);
        std::transform(cp.begin(), cp.end(), result.begin(),
                       [edges](size_t pos) { return edges[pos]; });

        auto end_time = bb::duration_cast<bb::us>(bb::clock::now() - start).count();

        if (benchmark) {
            std::cout << "init: ";
            init_time > 1000 ?
                std::cout << init_time/1.E3 << " s" :
                std::cout << init_time      << " us";
            std::cout << std::endl;
            std::cout << "loop: ";
            loop_time > 1000 ?
                std::cout << loop_time/1.E3 << " s" :
                std::cout << loop_time      << " us";
            std::cout << std::endl;
            std::cout << "end: ";
            end_time > 1000 ?
                std::cout << end_time/1.E3 << " s" :
                std::cout << end_time      << " us";
            std::cout << std::endl;
        }

        return result;
    }

    bb::array blocks(bb::data_array data, const double p,
                     bool counter, bool benchmark) {

        // compute weights
        std::map<double, int> hist;
        for (auto& i : data) {
            if (hist.find(i) == hist.end()) hist.emplace(i, 1);
            else hist[i]++;
        }

        bb::data_array x;
        bb::weights_array weights;
        for (auto& i : hist) {
            x      .push_back(i.first);
            weights.push_back(i.second);
        }

        return BayesianBlocks::blocks(x, weights, p, counter, benchmark);
    }
}

#endif