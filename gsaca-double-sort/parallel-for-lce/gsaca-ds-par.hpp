#pragma once

#include <omp.h>
#include "../ips4o/ips4o.hpp"

#include "../extract.hpp"
#include "../wrappers.hpp"
#include "phase_1.hpp"
#include "phase_2.hpp"

namespace gsaca_lyndon {

namespace double_sort_internal {

template<typename buffer_type, typename F,
         typename index_type, typename value_type>
auto sort_by_prefix_parallel(text_type<value_type> const &text, 
                             sa_type<index_type> &sa,
                             size_t const threads) {
  using count_type = get_count_type<index_type, buffer_type>;
  using p1_stack_type = phase_1_stack_type<buffer_type>;
  using p1_group_type = typename p1_stack_type::value_type;
  
  count_type const n = text.n;
  
  p1_stack_type result;
  
  // fill sa with values
  sa[0] = 0;
  sa[1] = n - 1;
  
  #pragma omp parallel for
  for (count_type i = 1; i < n - 1; ++i) {
      sa[i + 1] = i;
  }

  // sort sa by first character
  auto comp = [&](auto a, auto b) {
       auto extracted1 = text[a];
       auto extracted2 = text[b];
       return (extracted1 < extracted2) || ((extracted1 == extracted2) && a < b);
  };
  ips4o::parallel::sort(&(sa.sa[0]), &(sa.sa[n - 2]), comp);

  // determine gsizes
  // TODO: Parallelize
  count_type left_border = 2;
  count_type gsize = 1;
  for (count_type i = 2; i < n-1; ++i) {
      if (text[sa[i]] == text[sa[i+1]]) {
          ++gsize;
      }
      else {
          result.emplace_back(p1_group_type{left_border, gsize, 1, true, false});
          left_border = i+1;
          gsize = 1;
      }
  }
  result.emplace_back(p1_group_type{left_border, gsize, 1, true, false});

  // add flags
  #pragma omp parallel for
  for (count_type i = 0; i < n; ++i) {
      auto idx = sa[i];
      sa[i] = (idx != 0) ? F::conditional_add_flag(text[idx - 1] < text[idx], idx) : idx;
  }
  
  sa[0] = n - 1;
  sa[1] = 0;
  return result;
}

}

template<typename buffer_type = auto_buffer_type,
    bool use_flags = true,
    typename index_type, // auto deduce
    typename value_type, // auto deduce
    typename used_buffer_type = get_buffer_type <buffer_type, index_type>>
static void
gsaca_for_lce_wrapped(text_type<value_type> const &text, sa_type<index_type> &sa, size_t const threads) {
  static_assert(std::is_unsigned<value_type>::value);
  static_assert(std::is_unsigned<index_type>::value);
  static_assert(std::is_unsigned<used_buffer_type>::value);
  static_assert(check_buffer_type<buffer_type, index_type, used_buffer_type>);
    
  size_t const n = text.n;

  using F = flag_type<use_flags>;

  size_t const p_max = omp_get_max_threads();
  size_t const p = (threads == 0) ? p_max : threads;
  omp_set_dynamic(0);
  omp_set_num_threads(p);

  auto p1_input_groups =
      double_sort_internal::sort_by_prefix_parallel<used_buffer_type, F>
            (text, sa, p);
  used_buffer_type *const isa = (used_buffer_type *) malloc(n * sizeof(used_buffer_type));

  auto p2_input_groups = phase_1_by_sorting_parallel_for_lce<F>(sa, isa, p1_input_groups, p);
  
  phase_2_by_sorting_stable_parallel_for_lce<F>(sa, isa, p2_input_groups.data(),
                     p2_input_groups.size(), p);
                     
  #pragma omp parallel for
  for (size_t i = 0; i < n; ++i) {
      --sa[i];
  }

  free(isa);

  omp_set_num_threads(p_max);
}

template<typename buffer_type = auto_buffer_type,
    typename index_type, // auto deduce
    typename value_type>
static void gsaca_for_lce(value_type const *const text, index_type *const sa,
                          size_t const n, size_t const threads) {
  text_type<value_type> textprime { n + 2, text };
  sa_type<index_type> saprime { n + 2, sa };
  gsaca_for_lce_wrapped<buffer_type, false>(textprime, saprime, threads);
}


}
