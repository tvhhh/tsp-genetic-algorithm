#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <random>
#include "ga_tsp.h"

using namespace std;

int **initialization(int population_size, int cities) {
  int **population = new int *[population_size];

  int *sequence = new int[cities];
  for (int i = 0; i < cities; i++) sequence[i] = i;

  // Obtain a time-based seed
  unsigned seed = chrono::system_clock::now().time_since_epoch().count();

  for (int i = 0; i < population_size; i++) {
    population[i] = new int[cities];
    for (int j = 0; j < cities; j++) population[i][j] = sequence[j];
    // https://www.cplusplus.com/reference/algorithm/shuffle/
    shuffle(population[i], population[i] + cities, default_random_engine(random_device{}()));
  }

  delete[] sequence;

  return population;
}

int cost(int **map, int *sequence, int cities) {
  int path_cost = 0;
  for (int i = 0; i < cities-1; i++) {
    path_cost += map[sequence[i]][sequence[i+1]];
  }
  path_cost += map[sequence[cities-1]][sequence[0]];
  return path_cost;
}

double *distribution(int **map, int **population, int population_size, int cities) {
  double *density = new double[population_size];

  // Compute the path costs
  int *costs = new int[population_size];
  for (int i = 0; i < population_size; i++) {
    costs[i] = cost(map, population[i], cities);
  }

  // Scale to [0,1] range and take the exponential terms
  int *max = max_element(costs, costs + population_size);
  for (int i = 0; i < population_size; i++) {
    density[i] = exp(-1 * (double)costs[i] / (double)(*max));
  }

  // Free allocated memory
  delete[] costs;
  
  return density;
}

int *sampling(double *density, int population_size, double fraction) {
  int sampling_size = fraction * population_size;

  double *prob = new double[population_size];
  for (int i = 0; i < population_size; i++) {
    prob[i] = density[i];
  }

  // https://www.cplusplus.com/reference/random/discrete_distribution/
  default_random_engine generator(random_device{}());
  discrete_distribution<> distr(prob, prob + population_size);

  // Sampling without replacement
  int *sampling_ids = new int [sampling_size];
  for (int i = 0; i < sampling_size; i++) {
    int sampled_id = distr(generator);
    sampling_ids[i] = sampled_id;
    prob[sampled_id] = 0;
    distr = discrete_distribution<>(prob, prob + population_size);
  }

  delete[] prob;

  return sampling_ids;
}

void crossover(int *&seq_a, int *&seq_b, int cities) {
  unsigned seed = chrono::system_clock::now().time_since_epoch().count();
  int cut_point = rand_r(&seed) % (cities-1);

  int *offstring_a = new int[cities];
  int *offstring_b = new int[cities];
  for (int i = 0; i < cities; i++) {
    offstring_a[i] = seq_a[i];
    offstring_b[i] = seq_b[i];
  }

  // PMX crossover operator
  for (int i = 0; i <= cut_point; i++) {
    for (int j = 0; j < cities; j++) {
      if (offstring_a[j] == seq_b[i]) swap(offstring_a[i], offstring_a[j]);
      if (offstring_b[j] == seq_a[i]) swap(offstring_b[i], offstring_b[j]);
    }
  }

  // Reassign chromosomes
  delete[] seq_a;
  delete[] seq_b;
  seq_a = offstring_a;
  seq_b = offstring_b;
}

void mutation(int *&sequence, int cities) {
  unsigned seed = chrono::system_clock::now().time_since_epoch().count();

  // Twors mutation operator
  int first_point = rand_r(&seed) % cities;
  int second_point = rand_r(&seed) % cities;
  swap(sequence[first_point], sequence[second_point]);
}

int find_optimal_index(int **map, int **population, int population_size, int cities) {
  int min_cost = INT32_MAX;
  int min_id = 0;
  for (int i = 0; i < population_size; i++) {
    int path_cost = cost(map, population[i], cities);
    if (path_cost < min_cost) {
      min_cost = path_cost;
      min_id = i;
    }
  }
  return min_id;
}

void cleanup(int **population, int population_size) {
  for (int i = 0; i < population_size; i++) {
    delete[] population[i];
  }
  delete[] population;
}

int *genetic_algorithm(
  int **map,
  int cities,
  int population_size,
  int generations,
  double selection_rate,
  double mutation_rate,
  const char *output_filename
) {
  ofstream output_file(output_filename, ios::trunc);

  // Initialize population
  int **population = initialization(population_size, cities);

  for (int iter = 0; iter < generations; iter++) {
    double *density = distribution(map, population, population_size, cities);

    // Selection
    int selected_size = selection_rate * population_size;
    int *selected_ids = sampling(density, population_size, selection_rate);
    int **selected_individuals = new int *[selected_size];
    for (int i = 0; i < selected_size; i++) {
      int id = selected_ids[i];
      selected_individuals[i] = new int[cities];
      for (int j = 0; j < cities; j++) {
        selected_individuals[i][j] = population[id][j];
      }
    }
    delete[] selected_ids;

    // Crossover
    int breeding_size = (1 - selection_rate) * population_size;
    int *breeding_ids = sampling(density, population_size, 1 - selection_rate);
    int **next_generation = new int *[breeding_size];
    for (int i = 0; i < breeding_size / 2; i++) {
      // Take the parents
      int id_a = breeding_ids[2*i], id_b = breeding_ids[2*i+1];
      next_generation[2*i] = new int[cities];
      next_generation[2*i+1] = new int[cities];
      for (int j = 0; j < cities; j++) {
        next_generation[2*i][j] = population[id_a][j];
        next_generation[2*i+1][j] = population[id_b][j];
      }
      crossover(next_generation[2*i], next_generation[2*i+1], cities);
    }
    delete[] breeding_ids;

    delete[] density;

    // Mutation
    double *next_generation_density = distribution(map, next_generation, breeding_size, cities);
    int mutation_size = mutation_rate * breeding_size;
    int *mutation_ids = sampling(next_generation_density, breeding_size, mutation_rate);
    for (int i = 0; i < mutation_size; i++) {
      int id = mutation_ids[i];
      mutation(next_generation[id], cities);
    }
    delete[] mutation_ids;

    delete[] next_generation_density;

    // Generate the new population
    for (int i = 0; i < population_size; i++) {
      delete[] population[i];
      population[i] = (i < selected_size) ? selected_individuals[i] : next_generation[i-selected_size];
    }

    // Track the process
    int *costs = new int[population_size];
    for (int i = 0; i < population_size; i++) {
      costs[i] = cost(map, population[i], cities);
    }
    int *min = min_element(costs, costs + population_size);
    output_file << *min << "\n";
    delete[] costs;
  }

  // Find the optimal path
  int min_id = find_optimal_index(map, population, population_size, cities);

  // Copy the optimal path
  int *returned_path = new int[cities];
  for (int i = 0; i < cities; i++) {
    returned_path[i] = population[min_id][i];
  }

  // Free allocated memory
  cleanup(population, population_size);

  output_file.close();

  return returned_path;
}
