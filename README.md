# CitcomS with Data Assimilation

## Installation

This version of CitcomS only requires a valid MPI distribution (no more python / pyre).  Most clusters can load an MPI distribute using modules

e.g., in Bern I use:

```module load openmpi/1.10.2-intel```

For reasons relating to compilers, the cluster and node setup, some mpi versions may be preferred for your particular cluster.  I was recommended by the Bern sys admin to use the intel compiler with openmpi.  You might also want to find out which openmpi is preferred on your cluster (and perhaps add this above, so we have complete documentation of what works and what doesn't).

Then, to install CitcomS v3.1.1 with data assimilation go into the src/ directory and execute

```./mymake.py```

[Yes OK, for the step above you do need a python distribution, but this is only to run the commands that configure and make Citcoms.  Python is not actually used for running the code]

Example input and output configuration files are provided in inputeg/

This example is a good one to try and run first: src/examples/Full

You will need to setup your job submission script.  See a slurm example in jobsubmit/  Please add your own submission scripts to the same directory so we have more examples

For currently implemented features, scroll down below.

## Dan's work area follows

### Rakib's code

Sent to me by Sabin (12/09/17) CitcomSModDtopoRelease\_Rakib.zip
See diff/ directory for complete record
None of these changes have been implemented in the new code yet

Advection\_diffusion.c
  -- scaled visc and adiabatic heating by Di
     I gave him this code originally, so can include it
convection\_variables.h
  -- blob_profile, silo parameters, mantle_temp_adiabatic_increase
global\_defs.h
  -- Shell-output facility
Initial\_temperature.c
  -- bunch of silo / blob related functions
     evidentally seeding plumes for the IC
Material\_properties.c
  -- refstate updates.  I gave him this code so have it (notably Di depth dependent)
Output.c
  -- outputs of shells (theta, phi, r, temperature, vr)
Output\_vtk.c
  -- perhaps not related to Rakib's work
Viscosity\_structures.c
  -- some new viscosity structures (case 112, 113, 117, 118)
     these should be straightforward to merge in


### Code features implemented

#### Slab and lithosphere assimilation (grep for 'DJB SLAB')
    lith_age_depth_function
    lith_age_exponent
    lith_age_min
    lith_age_stencil_value
    slab_assim
    slab_assim_file
    sten_temp output
    
#### Composition (grep for 'DJB COMP')
    hybrid_method
    increased memory for tracer arrays

#### Viscosity structures (grep for 'DJB VISC')
    case 20, used in Flament et al. (2013, 2014)
    case 21, used in Flament et al. (2014), model TC8
    case 22, used in Flament et al. (2014), model TC7
    case 23, used in Flament et al. (2014), model TC9
    case 24, used in Zhang et al. (2010) and Bower et al. (2013)

#### Output time (grep for 'DJB TIME')
    output time modification [NOT IMPLEMENTED, ONLY PLACEHOLDERS]

### Code features NOT implemented

Not implemented (yet):
  -- internal velocity bcs (I don't think these are used by anyone anyway)
  -- outputs of heating terms, divv
  -- tracer density for elements and nodes output (added by Ting, see svn r52 through r55)
  -- buoyancy restart for dynamic topography (exclude buoyancy) (see svn r85)
  -- time exit routine (added by Ting, see svn r57)
  -- composition and temperature spherical harmonics
  -- anything from Rakib
  -- reverse gravity acceleraton (added by Ting, see svn r76) for SBI
  -- turn off itracer_warnings

Legacy code
-----------

Adv_diff -> done
BC_util -> ivels
Composition_related.c -> tracer density
composition_related.h -> tracer density
Convection.c -> done
Element_calculations -> buoy restart for dyn topo
Full_boundary_conditions -> ivels
Full_lith_age_read_files -> done
Full_read_input_from_file -> ivels
Full_solver.c -> done
global_defs.h -> dyn topo restarts, ivels
Instructions.c -> outputs, heating terms, divv
Lith_age.c -> done
output.c -> various outputs, and time exit routine (check with Nico)
Pan_problem.c -> dyn topo
Problem_related.c -> ivels
Regional_bcs.c -> ivels
Regional_lith_age_read_files -> done
Regional_read_input_files -> ivels
Regional_solver -> done
solver.h -> done
Stokes_flow_incom -> divv calculation for output
tracer_defs.h -> output tracer density
Viscosity_structure.c -> done
