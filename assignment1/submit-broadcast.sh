#!/bin/bash
#SBATCH -J ScatterGather-Test
#SBATCH -n 4
#SBATCH --mail-user=tht33@hi.is
#SBATCH --mail-type=end
module load gnu openmpi
mpirun /home/tht33/HPC_REI105M/assignment1/broadcast
