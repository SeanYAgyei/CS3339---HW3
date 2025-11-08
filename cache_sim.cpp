#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
//#include <iomanip>

class CacheSimulator {
private:
    struct CacheEntry {
        bool valid;
        unsigned long tag;
        
        CacheEntry() : valid(false), tag(0) {}
    };
    
    int num_entries;
    int associativity;
    int num_sets;
    int index_bits;
    int offset_bits;
    std::vector<std::vector<CacheEntry>> cache;
    std::string input_file;
    
public:
    CacheSimulator(int entries, int assoc, const std::string& file) 
        : num_entries(entries), associativity(assoc), input_file(file) {
        
        // This calculates the number of sets
        num_sets = num_entries / associativity;
        
        // This calculates the index bits
        index_bits = 0;
        int temp = num_sets;
        while (temp > 1) {
            temp >>= 1;
            index_bits++;
        }
        
        // This is for the word-addressable cache with 1 word per block, offset bits = 0
        offset_bits = 0;
        
        // This initializes the cache
        cache.resize(num_sets, std::vector<CacheEntry>(associativity));
    }
    
    void simulate() {
        std::ifstream infile(input_file);
        std::ofstream outfile("cache_sim_output");
        
        if (!infile.is_open()) {
            std::cerr << "Error: Could not open input file " << input_file << std::endl;
            return;
        }
        
        if (!outfile.is_open()) {
            std::cerr << "Error: Could not create output file cache_sim_output" << std::endl;
            return;
        }
        
        unsigned long addr;
        while (infile >> addr) {
            bool hit = accessCache(addr);
            outfile << addr << " : " << (hit ? "HIT" : "MISS") << std::endl;
        }
        
        infile.close();
        outfile.close();
    }
    
private:
    bool accessCache(unsigned long addr) {
        
        // his extracts the index and tag
        unsigned long index = (addr >> offset_bits) & (num_sets - 1);
        unsigned long tag = addr >> (index_bits + offset_bits);
        
        // This checks to see if there's a hit
        for (int i = 0; i < associativity; i++) {
            if (cache[index][i].valid && cache[index][i].tag == tag) {
                
                // If there's a hit - move to the MRU position (front of the list)
                CacheEntry temp = cache[index][i];
                for (int j = i; j > 0; j--) {
                    cache[index][j] = cache[index][j-1];
                }
                cache[index][0] = temp;
                return true;
            }
        }
        
        // If there's a miss - insert at MRU position, take out the LRU
        for (int i = associativity - 1; i > 0; i--) {
            cache[index][i] = cache[index][i-1];
        }
        cache[index][0].valid = true;
        cache[index][0].tag = tag;
        
        return false;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: ./cache_sim num_entries associativity input_file" << std::endl;
        return 1;
    }
    
    int num_entries = std::stoi(argv[1]);
    int associativity = std::stoi(argv[2]);
    std::string input_file = argv[3];
    
    // This validates the inputs
    if (num_entries <= 0 || associativity <= 0) {
        std::cerr << "Error: num_entries and associativity must be positive integers" << std::endl;
        return 1;
    }
    
    if (num_entries % associativity != 0) {
        std::cerr << "Error: num_entries must be divisible by associativity" << std::endl;
        return 1;
    }
    
    // This checks to see if num_entries is power of two
    if ((num_entries & (num_entries - 1)) != 0) {
        std::cerr << "Error: num_entries must be a power of two" << std::endl;
        return 1;
    }
    
    CacheSimulator simulator(num_entries, associativity, input_file);
    simulator.simulate();
    
    std::cout << "Simulation completed. Results written to cache_sim_output" << std::endl;
    
    return 0;
}
