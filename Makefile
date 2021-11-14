all:
	g++ -fopenmp -o tsp tsp.cc ga_tsp.cc
clean:
	rm tsp