#pragma once

#include "gsaca-double-sort/parallel/gsaca-ds-par.hpp"

template<typename index_type, typename value_type>
static void gsaca_ds1_par(value_type const *const text, 
                          index_type *const sa,
                          size_t const n,
                          size_t const threads = 0) {
  gsaca_lyndon::gsaca_ds1_par(text, sa, n, threads);
}

template<typename index_type, typename value_type>
static void gsaca_ds2_par(value_type const *const text, 
                          index_type *const sa,
                          size_t const n,
                          size_t const threads = 0) {
  gsaca_lyndon::gsaca_ds2_par(text, sa, n, threads);
}

template<typename index_type, typename value_type>
static void gsaca_ds3_par(value_type const *const text, 
                          index_type *const sa,
                          size_t const n,
                          size_t const threads = 0) {
  gsaca_lyndon::gsaca_ds3_par(text, sa, n, threads);
}


