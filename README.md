# Resolve TSP using Genetic Algorithm

## To compile
```
make all
```

## To cleanup
```
make clean
```

## To run genetic algorithm (prerequisite: 8-core CPU)
```
./tsp <number_of_cities> <number_of_ga_iterations>
```
e.g. (Define TSP with 30 cities and run genetic algorithm with 500 iterations)
```
./tsp 30 500
```

## To visualize the result (remember to run algorithm with consistent hyperparameters before visualizing)
```
python gaviz.py <number_of_cities> <number_of_ga_iterations>
```