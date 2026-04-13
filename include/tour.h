#ifndef TOUR_H
#define TOUR_H

#include <vector>
#include <iostream>

class Tour {
private:
    mutable double cached_cost;
    mutable double cached_value;

public:
    std::vector<int> vertices;

    Tour(int n = 0);
    Tour(const Tour& other);
    Tour& operator=(const Tour& other);


    double compute_distance(const class InputData& input, const std::vector<int>& new1_to_old0) const;
    double compute_cost(const class InputData& input, const std::vector<int>& new1_to_old0) const;
    double compute_value(const class InputData& input, const std::vector<int>& new1_to_old0) const;  
    void print() const;
    bool validate(int n) const;
    Tour copy() const;
    void invalidate_cache();
};

#endif