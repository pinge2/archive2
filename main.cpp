#include "vv128.hpp"
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <windows.h>


struct TestResult {
    double score;
    double max_score;
    std::string details;
};


using Prng = Prng256;
using PrngMethod = uint64_t (Prng::*)();


TestResult test_chi_square(Prng& prng, PrngMethod method, int num_buckets = 64, int num_samples = 10000000) {
    std::cout << "\n=== Chi-square test ===\n";
    
    std::vector<int> buckets(num_buckets, 0);
    
    for (int i = 0; i < num_samples; ++i) {
        uint64_t value = (prng.*method)();
        int bucket = (value >> (64 - 10)) % num_buckets;
        buckets[bucket]++;
    }
    
    double expected = static_cast<double>(num_samples) / num_buckets;
    double chi2 = 0.0;
    
    for (int count : buckets) {
        double diff = count - expected;
        chi2 += (diff * diff) / expected;
    }
    
    double dof = num_buckets - 1;
    double critical_95 = dof + 2 * sqrt(2.0 * dof);
    double critical_99 = dof + 3 * sqrt(2.0 * dof);
    
    double score;
    if (chi2 < critical_95) score = 10.0;
    else if (chi2 < critical_99) score = 10.0 - 5.0 * (chi2 - critical_95) / (critical_99 - critical_95);
    else score = std::max(0.0, 5.0 - 5.0 * (chi2 - critical_99) / critical_99);
    
    std::cout << "Buckets:    " << std::setw(8) << num_buckets << "\n";
    std::cout << "Samples:    " << std::setw(8) << num_samples << "\n";
    std::cout << "Expected:   " << std::setw(10) << std::fixed << std::setprecision(1) << expected << "\n";
    std::cout << "Chi-square: " << std::setw(10) << std::fixed << std::setprecision(2) << chi2 << "\n";
    std::cout << "95% limit:  " << std::setw(10) << std::fixed << std::setprecision(2) << critical_95 << "\n";
    std::cout << "Score:      " << std::setw(10) << std::fixed << std::setprecision(1) << score << " / 10\n";
    
    std::cout << "\nFirst 20 buckets:\n";
    
    for (int i = 0; i < std::min(20, num_buckets); ++i) {
        std::cout << std::setw(3) << i << ": " << std::setw(6) << buckets[i];
        if ((i + 1) % 5 == 0) std::cout << "\n";
    }
    
    return {score, 10.0, chi2 < critical_95 ? "Excellent distribution" : "Some deviation detected"};
}


TestResult test_runs(Prng& prng, PrngMethod method, int num_samples = 1000000) {
    std::cout << "\n=== Bit Runs test ===\n";
    
    std::vector<uint8_t> bits(num_samples);
    
    for (int i = 0; i < num_samples; ++i) bits[i] = (prng.*method)() & 1;
    
    int runs = 1;
    for (int i = 1; i < num_samples; ++i) {
        if (bits[i] != bits[i-1]) runs++;
    }
    
    int zeros = 0, ones = 0;
    for (uint8_t bit : bits) {
        if (bit == 0) zeros++;
        else ones++;
    }
    
    double n = num_samples;
    double expected_runs = 1 + 2.0 * zeros * ones / n;
    double stddev = sqrt(2.0 * zeros * ones * (2.0 * zeros * ones - n) / (n * n * (n - 1)));
    double z_score = (runs - expected_runs) / stddev;
    
    double score;
    double abs_z = std::abs(z_score);
    
    if (abs_z < 1.0) score = 10.0;
    else if (abs_z < 1.96) score = 10.0 - 4.0 * (abs_z - 1.0) / 0.96;
    else if (abs_z < 2.58) score = 6.0 - 4.0 * (abs_z - 1.96) / 0.62;
    else score = std::max(0.0, 2.0 - 2.0 * (abs_z - 2.58) / 2.58);
    
    std::cout << "Runs:         " << std::setw(8) << runs << "\n";
    std::cout << "Expected:     " << std::setw(8) << std::fixed << std::setprecision(1) << expected_runs << "\n";
    std::cout << "Z-score:      " << std::setw(8) << std::fixed << std::setprecision(3) << z_score << "\n";
    std::cout << "Zeros:        " << std::setw(8) << zeros << "\n";
    std::cout << "Ones:         " << std::setw(8) << ones << "\n";
    std::cout << "Score:        " << std::setw(8) << std::fixed << std::setprecision(1) << score << " / 10\n";
    
    std::string details = abs_z < 1.96 ? "Random sequence" : "Possible pattern detected";
    return {score, 10.0, details};
}


TestResult test_autocorrelation(Prng& prng, PrngMethod method, int max_lag = 20, int num_samples = 500000) {
    std::cout << "\n=== Autocorrelation test ===\n";
    
    std::vector<double> samples(num_samples);
    
    for (int i = 0; i < num_samples; ++i) {
        samples[i] = static_cast<double>((prng.*method)()) / 18446744073709551615.0;
    }
    
    double mean = 0.0;
    for (double val : samples) mean += val;
    mean /= num_samples;
    
    double var = 0.0;
    for (double val : samples) {
        double diff = val - mean;
        var += diff * diff;
    }
    var /= num_samples;
    
    double total_penalty = 0.0;
    int violations = 0;
    
    std::cout << "Lag   Autocorr    95% limit   Status\n";
    std::cout << "----- ----------- ----------- ------\n";
    
    for (int lag = 1; lag <= max_lag; ++lag) {
        double autocorr = 0.0;
        for (int i = 0; i < num_samples - lag; ++i) {
            autocorr += (samples[i] - mean) * (samples[i + lag] - mean);
        }
        autocorr /= (num_samples - lag) * var;
        
        double critical = 1.96 / sqrt(num_samples);
        double abs_corr = std::abs(autocorr);
        
        if (abs_corr > critical) {
            violations++;
            total_penalty += (abs_corr - critical) / critical;
        }
        
        std::cout << std::setw(3) << lag << "   "
                  << std::setw(11) << std::fixed << std::setprecision(6) << autocorr << " "
                  << std::setw(11) << std::fixed << std::setprecision(4) << critical << " "
                  << (abs_corr > critical ? " FAIL" : "  OK ") << "\n";
    }
    
    double score;
    if (violations == 0) score = 10.0;
    else if (violations <= 2) score = 10.0 - violations * 1.5 - total_penalty * 0.5;
    else score = std::max(0.0, 7.0 - (violations - 2) * 1.0 - total_penalty * 0.5);
    
    std::cout << "Violations:  " << std::setw(8) << violations << "/" << max_lag << "\n";
    std::cout << "Score:       " << std::setw(8) << std::fixed << std::setprecision(1) << score << " / 10\n";
    
    std::string details = violations == 0 ? "No correlations detected" : violations <= 2 ? "Minor correlations" : "Significant correlations found";
    return {score, 10.0, details};
}


TestResult test_bit_balance(Prng& prng, PrngMethod method, int num_samples = 2000000) {
    std::cout << "\n=== Bit Balance Test ===\n";
    
    std::vector<int> bit_counts(64, 0);
    
    for (int i = 0; i < num_samples; ++i) {
        uint64_t value = (prng.*method)();
        for (int bit = 0; bit < 64; ++bit) {
            if (value & (1ULL << bit)) bit_counts[bit]++;
        }
    }
    
    double expected = num_samples / 2.0;
    double max_bias = 0.0;
    int worst_bit = 0;
    
    std::cout << "Bit   Ones     Expected   Bias     Status\n";
    std::cout << "----  -------  --------  -------  ------\n";
    
    for (int bit = 0; bit < 64; ++bit) {
        double bias = std::abs(bit_counts[bit] - expected);
        double max_allowed = 3 * sqrt(num_samples) / 2;
        double relative_bias = bias / expected;
        
        if (relative_bias > max_bias) {
            max_bias = relative_bias;
            worst_bit = bit;
        }
        
        if (bit < 16 || bit >= 48) {
            std::cout << std::setw(4) << bit << " "
                      << std::setw(7) << bit_counts[bit] << " "
                      << std::setw(8) << std::fixed << std::setprecision(0) << expected << " "
                      << std::setw(7) << std::fixed << std::setprecision(1) << bias << " "
                      << (bias < max_allowed ? "  OK " : " FAIL") << "\n";
        } else if (bit == 16) {
            std::cout << " ...   .....    ......   ......   ....\n";
        }
    }
    
    double score;
    if (max_bias < 0.005) score = 10.0;
    else if (max_bias < 0.010) score = 10.0 - (max_bias - 0.005) * 400.0;
    else if (max_bias < 0.020) score = 8.0 - (max_bias - 0.010) * 300.0;
    else if (max_bias < 0.040) score = 5.0 - (max_bias - 0.020) * 150.0;
    else score = std::max(0.0, 2.0 - (max_bias - 0.040) * 50.0);
    
    std::cout << "Max bias:    " << std::setw(7) << std::fixed << std::setprecision(4) << max_bias << " (bit " << worst_bit << ")\n";
    std::cout << "Score:       " << std::setw(8) << std::fixed << std::setprecision(1) << score << " / 10\n";
    
    std::string details;
    if (max_bias < 0.005) details = "Perfect balance";
    else if (max_bias < 0.010) details = "Very good balance";
    else if (max_bias < 0.020) details = "Good balance";
    else if (max_bias < 0.040) details = "Acceptable balance";
    else details = "Significant bias";
    
    return {score, 10.0, details};
}


TestResult test_birthday_spacing(Prng& prng, PrngMethod method, int num_samples = 100000) {
    std::cout << "\n=== Birthday Spacing test ===\n";
    
    std::vector<uint64_t> samples(num_samples);
    
    for (int i = 0; i < num_samples; ++i) samples[i] = (prng.*method)();
    
    std::sort(samples.begin(), samples.end());
    
    uint64_t min_gap = UINT64_MAX;
    uint64_t max_gap = 0;
    double sum_gaps = 0.0;
    
    for (int i = 1; i < num_samples; ++i) {
        uint64_t gap = samples[i] - samples[i-1];
        if (gap < min_gap) min_gap = gap;
        if (gap > max_gap) max_gap = gap;
        sum_gaps += gap;
    }
    
    double avg_gap = sum_gaps / (num_samples - 1);
    double expected_gap = 18446744073709551615.0 / (num_samples * num_samples);
    double ratio = min_gap / expected_gap;
    
    double score;
    if (ratio > 1.0) score = 10.0;
    else if (ratio > 0.1) score = 5.0 + 5.0 * (ratio - 0.1) / 0.9;
    else if (ratio > 0.01) score = 2.0 + 3.0 * (ratio - 0.01) / 0.09;
    else score = std::max(0.0, 2.0 * ratio / 0.01);
    
    std::cout << "Min gap:     " << std::setw(20) << min_gap << "\n";
    std::cout << "Expected:    " << std::setw(20) << std::fixed << std::setprecision(0) << expected_gap << "\n";
    std::cout << "Ratio:       " << std::setw(20) << std::fixed << std::setprecision(6) << ratio << "\n";
    std::cout << "Avg gap:     " << std::setw(20) << std::fixed << std::setprecision(0) << avg_gap << "\n";
    std::cout << "Score:       " << std::setw(20) << std::fixed << std::setprecision(1) << score << " / 10\n";
    
    std::string details = ratio > 1.0 ? "Excellent spacing" : ratio > 0.1 ? "Acceptable spacing" : "Suspicious clustering";
    return {score, 10.0, details};
}


double run_all_tests(PrngMethod method, uint64_t seed = 123456789) {
    std::cout << "\n╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║            64-BIT PRNG QUALITY TEST SUITE              ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n";
    std::cout << "Seed: " << seed << " (with warmup)\n";

    Prng prng;
    
    std::vector<TestResult> results;
    
    // Setup PRNG for each test
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "TEST 1/5: Distribution Uniformity\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    prng.setstate(seed, true); // warmup = true
    results.push_back(test_chi_square(prng, method));
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "TEST 2/5: Bit Runs Analysis\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    prng.setstate(seed, true);
    results.push_back(test_runs(prng, method));
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "TEST 3/5: Autocorrelation\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    prng.setstate(seed, true);
    results.push_back(test_autocorrelation(prng, method));
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "TEST 4/5: Bit Balance\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    prng.setstate(seed, true);
    results.push_back(test_bit_balance(prng, method));
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "TEST 5/5: Birthday Spacing\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    prng.setstate(seed, true);
    results.push_back(test_birthday_spacing(prng, method));

    double weights[] = {
        2.5, 1.5, 1.0, 2.0, 3.0
    };
    double total_sum = 10.0;
    
    double weighted_score = 0.0;

    for (int i = 0; i < 5; i++){
        double val = std::max(results[i].score, 0.001);
        weighted_score += weights[i] * log(val);
    }

    weighted_score = exp(weighted_score / total_sum);
    
    std::cout << "\n╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║              FINAL QUALITY REPORT                      ║\n";
    std::cout << "╠════════════════════════════════════════════════════════╣\n";
    std::cout << "║ Test              Weight  Score   Details              ║\n";
    std::cout << "╠════════════════════════════════════════════════════════╣\n";
    
    const char* test_names[] = {
        "Chi-square", "Bit Runs", "Autocorrelation", "Bit Balance", "Birthday"
    };
    
    for (int i = 0; i < 5; ++i) {
        std::cout << "║ " << std::setw(15) << std::left << test_names[i]
                  << " " << std::setw(5) << std::fixed << std::setprecision(1) << weights[i]
                  << "  " << std::setw(4) << std::fixed << std::setprecision(1) << results[i].score
                  << "   " << std::setw(20) << std::left << results[i].details.substr(0, 20) << " ║\n";
    }
    
    std::cout << "╠════════════════════════════════════════════════════════╣\n";
    std::cout << "║ WEIGHTED SCORE: " << std::setw(6) << std::fixed << std::setprecision(1) << weighted_score << " / 10";
    
    const char* rating;
    if (weighted_score >= 9.5) rating = "★★★★★ EXCEPTIONAL";
    else if (weighted_score >= 8.5) rating = "★★★★☆ EXCELLENT";
    else if (weighted_score >= 7.5) rating = "★★★★  VERY GOOD";
    else if (weighted_score >= 6.5) rating = "★★★☆  GOOD";
    else if (weighted_score >= 5.0) rating = "★★★   ADEQUATE";
    else if (weighted_score >= 3.0) rating = "★★    POOR";
    else rating = "★     FAILED";
    
    std::cout << "                          ║\n";
    std::cout << "║ " << std::setw(41) << std::left << rating << " ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n";
    
    if (weighted_score < 5.0) {
        std::cout << "\n⚠ WARNING: This PRNG shows significant statistical weaknesses.\n";
        std::cout << "  Not suitable even general use or scientific applications.\n";
    } else if (weighted_score < 7.5) {
        std::cout << "\n⚡ NOTE: Acceptable for general use but consider alternatives\n";
        std::cout << "  for applications requiring high-quality randomness.\n";
    } else {
        std::cout << "\n✓ This PRNG demonstrates high-quality statistical properties.\n";
    }
    
    return weighted_score;
}



int main(){
    SetConsoleOutputCP(CP_UTF8);

    Prng256 prng;

    /*for (int i = 0; i < 8; ++i){
        print("Rand " << i << ": " << hex(prng.wqrand()));

        print('\t' << hex(prng.getstate(0)));
        print('\t' << hex(prng.getstate(1)));
        print('\t' << hex(prng.getstate(2)));
        print('\t' << hex(prng.getstate(3)));
    }*/
    print(run_all_tests(&Prng256::wqrand) << " score");
}