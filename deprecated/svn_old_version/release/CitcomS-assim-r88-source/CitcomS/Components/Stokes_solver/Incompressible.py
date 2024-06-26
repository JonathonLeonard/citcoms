#!/usr/bin/env python
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
#<LicenseText>
#
# CitcomS.py by Eh Tan, Eun-seo Choi, and Pururav Thoutireddy.
# Copyright (C) 2002-2005, California Institute of Technology.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#</LicenseText>
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#

from CitcomS.Components.CitcomComponent import CitcomComponent


class Incompressible(CitcomComponent):


    def __init__(self, name, facility):
        CitcomComponent.__init__(self, name, facility)

        return



    def run(self):
        from CitcomSLib import general_stokes_solver
        general_stokes_solver(self.all_variables)
	return


    def initialize(self, all_variables):
        CitcomComponent.initialize(self, all_variables)
        from CitcomSLib import set_cg_defaults, set_mg_defaults
        if self.inventory.Solver == "cgrad":
            set_cg_defaults(self.all_variables)
        elif self.inventory.Solver == "multigrid":
            set_mg_defaults(self.all_variables)
        return



    def launch(self):
        from CitcomSLib import general_stokes_solver_setup
        general_stokes_solver_setup(self.all_variables)
        return



    #def fini(self):
	#return



    def setProperties(self, stream):
        from CitcomSLib import Incompressible_set_properties
        Incompressible_set_properties(self.all_variables, self.inventory, stream)
        return



    class Inventory(CitcomComponent.Inventory):

        import pyre.inventory as prop


        Solver = prop.str("Solver", default="cgrad",
                 validator=prop.choice(["cgrad",
                                        "multigrid"]))
        node_assemble = prop.bool("node_assemble", default=True)
        precond = prop.bool("precond", default=True)

        accuracy = prop.float("accuracy", default=1.0e-4)
        inner_accuracy_scale = prop.float("inner_accuracy_scale", default=1.0)

        check_continuity_convergence = prop.bool("check_continuity_convergence", default=True)
        check_pressure_convergence = prop.bool("check_pressure_convergence", default=True)

        mg_cycle = prop.int("mg_cycle", default=1)
        down_heavy = prop.int("down_heavy", default=3)
        up_heavy = prop.int("up_heavy", default=3)

        vlowstep = prop.int("vlowstep", default=1000)
        vhighstep = prop.int("vhighstep", default=3)
        max_mg_cycles = prop.int("max_mg_cycles", default=50)
        piterations = prop.int("piterations", default=1000)

        aug_lagr = prop.bool("aug_lagr", default=True)
        aug_number = prop.float("aug_number", default=2.0e3)

        uzawa = prop.str("uzawa", default="cg",
                         validator=prop.choice(["cg", "bicg"]))
        compress_iter_maxstep = prop.int("compress_iter_maxstep", default=100)

        inner_remove_rigid_rotation = prop.bool("inner_remove_rigid_rotation", default=False)
        remove_rigid_rotation = prop.bool("remove_rigid_rotation", default=True)
        remove_angular_momentum = prop.bool("remove_angular_momentum", default=True)

        only_check_vel_convergence = prop.bool("only_check_vel_convergence", default=False)

        # Not used. Retained here for backward compatibility.
        tole_compressibility = prop.float("tole_compressibility", default=1.0e-7)
        relative_err_accuracy = prop.float("relative_err_accuracy", default=0.001)
# version
__id__ = "$Id: Incompressible.py 16162 2010-01-21 21:23:26Z tan2 $"

# End of file
