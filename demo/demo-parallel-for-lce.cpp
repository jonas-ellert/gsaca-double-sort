#include <string>
#include <iostream>
#include "../gsaca-double-sort-par.hpp"

int main() {
    
  std::string input = "mississippi";
  input[4] = 0; // this version needs no sentinels and allows null-characters
    
  size_t n = input.size();  
  unsigned char *T = (unsigned char *) input.data();
  unsigned int *SA1 = (unsigned int *) malloc(n * sizeof(unsigned int));
  unsigned int *SA2 = (unsigned int *) malloc(n * sizeof(unsigned int));
  unsigned int *SA3 = (unsigned int *) malloc(n * sizeof(unsigned int));

  gsaca_for_lce(T, SA1, n); // default = omp max threads
  gsaca_for_lce(T, SA2, n, 2); // 2 threads
  gsaca_for_lce(T, SA3, n, 4); // 4 threads
  
  for (unsigned int i = 0; i < n; ++i)
    std::cout << SA1[i] << " ";
  std::cout << std::endl;
  
  for (unsigned int i = 0; i < n; ++i)
    std::cout << SA2[i] << " ";
  std::cout << std::endl;  
  
  for (unsigned int i = 0; i < n; ++i)
    std::cout << SA3[i] << " ";
  std::cout << std::endl;  
  
  free(SA1);
  free(SA2);
  free(SA3);

  return 0;
}
