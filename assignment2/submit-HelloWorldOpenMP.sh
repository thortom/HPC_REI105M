#!/bin/bash
#SBATCH -J HelloWorld-OpenMP
#SBATCH -n 1
#SBATCH --mail-user=tht33@hi.is
#SBATCH --mail-type=end
module load gnu

export OMP_NUM_THREADS=4

/home/tht33/HPC_REI105M_2/assignment2/HelloWorldOpenMP
