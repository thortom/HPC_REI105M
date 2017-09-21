#!/bin/bash
#SBATCH -J Broadcast-Test
#SBATCH -n 4
#SBATCH --mail-user=tht33@hi.is
#SBATCH --mail-type=end
module load gnu openmpi
mpirun /home/tht33/assignment1/broadcast
