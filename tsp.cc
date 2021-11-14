#include <chrono>
#include <cstdio>
#include <fstream>
#include <omp.h>
#include <random>
#include <sys/stat.h>
#include "ga_tsp.h"

#define OMP_NUM_THREADS 8

using namespace std;

int cities = 30;
int population_size = 1000;
int generations = 500;
double selection_rate = 0.5;
double mutation_rate = 0.05;

bool exists(const char *filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

int **initialize_tsp_problem(int cities) {
  int **map = new int *[cities];
  for (int i = 0; i < cities; i++) {
    map[i] = new int[cities];
  }
  char buff[100];
  snprintf(buff, sizeof(buff), "inputs/tsp_%d.txt", cities);
  const char *tsp_filename = buff;
  if (exists(tsp_filename)) {
    ifstream file(tsp_filename);
    for (int i = 0; i < cities; i++) {
      for (int j = 0; j < cities; j++) {
        file >> map[i][j];
      }
    }
    file.close();
  } else {
    ofstream file(tsp_filename, ios::trunc);
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    for (int i = 0; i < cities-1; i++) {
      for (int j = i; j < cities; j++) {
        map[i][j] = (i == j) ? 0 : rand_r(&seed) % 100;
        map[j][i] = map[i][j];
      }
    }
    for (int i = 0; i < cities; i++) {
      for (int j = 0; j < cities; j++) {
        file << map[i][j] << "\n";
      }
    }
    file.close();
  }
  
  return map;
}

int *sequential_genetic_algorithm(int **map) {
  char buff[100];
  snprintf(buff, sizeof(buff), "outputs/sequential_process_%d_%d.txt", cities, generations);
  const char *filename = buff;

  int *optimal_path = genetic_algorithm(
    map,
    cities,
    population_size,
    generations,
    selection_rate,
    mutation_rate,
    filename
  );
  int optimal_cost = cost(map, optimal_path, cities);

  printf("%d", optimal_path[0]);
  for (int i = 1; i <= cities; i++) {
    printf("->%d", optimal_path[i % cities]);
  }
  printf("\nCost: %d\n", optimal_cost);

  return optimal_path;
}

int *parallel_genetic_algorithm(int **map) {
  int *optimal_path = new int[cities];
  int optimal_cost = INT32_MAX;

  #pragma omp parallel shared(optimal_path, optimal_cost)
  {
    int tid = omp_get_thread_num();
    char buff[100];
    snprintf(buff, sizeof(buff), "outputs/parallel_process_%d_%d_%d.txt", tid, cities, generations);
    const char *filename = buff;
    int *suboptimal_path = genetic_algorithm(
      map,
      cities,
      population_size,
      generations,
      selection_rate,
      mutation_rate,
      filename
    );
    int suboptimal_cost = cost(map, suboptimal_path, cities);
    #pragma omp critical
    {
      if (suboptimal_cost < optimal_cost) {
        optimal_cost = suboptimal_cost;
        for (int i = 0; i < cities; i++) optimal_path[i] = suboptimal_path[i];
      }
    }
    delete[] suboptimal_path;
  }

  printf("%d", optimal_path[0]);
  for (int i = 1; i <= cities; i++) {
    printf("->%d", optimal_path[i % cities]);
  }
  printf("\nCost: %d\n", optimal_cost);

  return optimal_path;
}

void cleanup(int **map) {
  for (int i = 0; i < cities; i++) {
    delete[] map[i];
  }
  delete[] map;
}

void write_exec_time(ofstream &file, double sequential_time, double parallel_time) {
  file << "sequential " << cities << " " << generations << " " << sequential_time << "s\n";
  file << "parallel " << cities << " " << generations << " " << parallel_time << "s\n";
}

int main(int argc, char **argv) {
  if (argc <= 2) exit(1);

  cities = stoi(argv[1]);
  generations = stoi(argv[2]);

  int **map = initialize_tsp_problem(cities);
  for (int i = 0; i < cities; i++) {
    for (int j = 0; j < cities; j++) {
      printf("%3d", map[i][j]);
    }
    printf("\n");
  }

  ofstream exec_time("outputs/exec_time.txt", ios::app);
  
  printf("\nSequential genetic algorithm\n");
  double sequential_time = omp_get_wtime();
  int *sequential_optimal_path = sequential_genetic_algorithm(map);
  sequential_time = omp_get_wtime() - sequential_time;
  printf("Execution time: %fs\n", sequential_time);

  printf("\nParallel genetic algorithm (8 threads)\n");
  double parallel_time = omp_get_wtime();
  int *parallel_optimal_path = parallel_genetic_algorithm(map);
  parallel_time = omp_get_wtime() - parallel_time;
  printf("Execution time: %fs\n", parallel_time);

  write_exec_time(exec_time, sequential_time, parallel_time);
  exec_time.close();

  cleanup(map);
  delete[] sequential_optimal_path;
  delete[] parallel_optimal_path;

  return 0;
}
