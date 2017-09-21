#!/bin/bash
#SBATCH -J PingPong
#SBATCH -n 2
#SBATCH --mail-user=tht33@hi.is
#SBATCH --mail-type=end
module load gnu openmpi
mpirun /home/tht33/assignment1/pingpong
