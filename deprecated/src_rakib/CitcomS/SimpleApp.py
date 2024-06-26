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


from BaseApplication import BaseApplication
import journal


class SimpleApp(BaseApplication):


    def __init__(self, name="CitcomS"):
        BaseApplication.__init__(self, name)

        self.solver = None
        self.solverCommunicator = None
        return



    def getNodes(self):
        '''Compute the required # of processors for MPI.
        '''
        s = self.inventory.solver.inventory.mesher.inventory
        nproc = s.nproc_surf * s.nprocx * s.nprocy * s.nprocz
        return nproc



    def initialize(self):
        '''Setup the problem.
        '''
        self.findLayout()

        self.controller.initialize(self)
        return



    def findLayout(self):
        '''Assign controller/solver/communicator to this process.
        '''
        self.controller = self.inventory.controller
        self.solver = self.inventory.solver
        import mpi
        self.solverCommunicator = mpi.world()
        return



    def reportConfiguration(self):

        import mpi
        rank = mpi.world().rank

        if rank != 0:
            return

        self._info.line("configuration:")
        self._info.line("  properties:")
        #self._info.line("    name: %r" % self.inventory.name)
        #self._info.line("    full name: %r" % self.inventory.fullname)

        self._info.line("  facilities:")
        self._info.line("    launcher: %r" % self.inventory.launcher.name)

        self._info.line("    solver: %r" % self.solver.name)
        self._info.log("    controller: %r" % self.controller.name)

        return


    class Inventory(BaseApplication.Inventory):

        import pyre.inventory

        import Controller
        import Solver

        controller = pyre.inventory.facility("controller", factory=Controller.controller)

        # solver can be either "regional" (component name for Solver.regionalSolver,
        # defined in ../etc/regional.odb) or "full" (component name for
        # Solver.fullSolver, defined in ../etc/full.odb)
        solver = pyre.inventory.facility("solver", factory=Solver.regionalSolver)




# version
__id__ = "$Id: SimpleApp.py 7594 2007-07-02 22:21:21Z tan2 $"

# End of file
