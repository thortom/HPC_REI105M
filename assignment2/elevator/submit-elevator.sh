#!/bin/bash
#SBATCH -J Elevator_At_Work
#SBATCH -n 8
#SBATCH --mail-user=tht33@hi.is
#SBATCH --mail-type=end
module load gnu openmpi
mpirun /home/tht33/HPC_REI105M_2/assignment2/elevator/work_place
