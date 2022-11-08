#pragma once

#include "uint_types.hpp"

namespace gsaca_lyndon {
    
template<typename index_type>
struct sa_type {
    size_t const n; // this is the already adjusted n, i.e., text len + 2
    index_type *const sa;
    index_type sa01 [2] = {};
    
    inline index_type &operator[] (size_t const i) {
      if(i > 1) [[likely]] return sa[i - 2];
      else return sa01[i];
    }
};


template<typename value_type>
struct text_type {
    size_t const n; // this is the already adjusted n, i.e., text len + 2
    value_type const *const text;
    
    inline uint128_t operator[] (size_t const i) const {
      if(i > 0 && i < n - 1) [[likely]] return (((uint128_t)text[i - 1]) + 1);
      else return 0;
    }
};

}
