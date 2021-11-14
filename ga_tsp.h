#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

int *genetic_algorithm(
  int **map,
  int cities,
  int population_size,
  int generations,
  double selection_rate,
  double mutation_rate,
  const char *output_filename
);

int cost(int **map, int *sequence, int cities);

#endif
