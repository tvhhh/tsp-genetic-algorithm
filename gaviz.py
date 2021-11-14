import sys
import matplotlib.pyplot as plt

processors = range(8)

def get_genetic_output(cities, generations):
  outputs = []
  
  # Read parallel outputs
  for processor in processors:
    with open(f'outputs/parallel_process_{processor}_{cities}_{generations}.txt') as parallel_output_file:
      parallel_outs = []
      for out in parallel_output_file:
        parallel_outs.append(int(out))
      outputs.append(parallel_outs)
      parallel_output_file.close()
  
  # Read sequential outputs
  with open(f'outputs/sequential_process_{cities}_{generations}.txt') as sequential_output_file:
    sequential_outs = []
    for out in sequential_output_file:
      sequential_outs.append(int(out))
    outputs.append(sequential_outs)
    sequential_output_file.close()
  
  return outputs

def visualize_genetic_output(cities, generations):
  outputs = get_genetic_output(cities, generations)
  colors = ['orange', 'green', 'red', 'purple', 'brown', 'pink', 'olive', 'cyan', 'blue']
  plt.figure(figsize=(16,16))
  for i, output in enumerate(outputs):
    color = colors.pop(0)
    label = 'Single Thread' if i == 8 else f'OpenMP Thread {i}'
    plt.plot(output, color=color, label=label, linewidth=0.75)
  plt.xlabel('Iteration')
  plt.ylabel('Found optimal cost')
  plt.legend()
  plt.show()

def main():
  cities = int(sys.argv[1])
  generations = int(sys.argv[2])
  visualize_genetic_output(cities, generations)

if __name__ == "__main__":
  main()
