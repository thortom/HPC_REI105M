#!/bin/bash
#SBATCH -J hello-world
#SBATCH -n 12
#SBATCH --mail-user=tht33@hi.is
#SBATCH --mail-type=end
module load gnu openmpi
mpirun /home/tht33/HPC_REI105M/assignment1/hello
