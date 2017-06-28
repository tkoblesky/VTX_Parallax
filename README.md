# VTX_Parallax

This is a repository for the code to simulate the effect of parallax on a detector for particle collision vertices.

Two explain this, let me paint the picture:
Two particles come in and collide along the collision axis z. 
We have a particle detector perpedicular to this collision axis, only cm away from the collision.
As the collision happens at different locations along the beam axis relative the static location of the detector,
we have parralax effects. We can account for these effects by mathematically combining restuls of collisions
in beam axis collision location bins. This code determines how many bins are necessary (how fine or coarse).

This code simulates a spray of particles output from these collisions and calculates the degree of distortion
due to parallax when accounted for in different amounts of beam axis collision location bins.

This code is written in C++ and meant to be run with ROOT v5

These files:
dead_config.cpp
dead_config_349369.cpp
dead_config_416892.cpp

hold configurations of very inefficient elements in the simulated detector based on some real data.

These files:
event.h
sensor.h
track.h

are the header files for some useful objects in the simulation

These files:
run_sim.cpp
run_sum_original.cpp

calls the objects and runs the simulation based on input parameters

This file:
run_sim_program.C

is a macro which runs the suite of simulations to get a result

This file:
plot_fluctuations.C

plots the results of the simulation