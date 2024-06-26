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

from pyre.components.Component import Component


class Coupler(Component):


    def __init__(self, name, facility):
        Component.__init__(self, name, facility)

        self.all_variables = None
        self.communicator = None
        self.srcCommList = []
        self.sinkComm = None
        self.remoteSize = 0

        self.synchronized = True
        self.done = False
        self.coupled_steps = 1
        return


    def initialize(self, solver):

        # when monitor_max_T is on, the timestep size might get changed
        # inside the tsolver, which will cause trouble in coupler
        # synchronization.
        assert solver.inventory.tsolver.inventory.monitor_max_T == False, \
               'Error: solver.tsolver.monitor_max_T must be off!'

        # the exchanger doesn't know how to apply boundary conditions
        # for the multigrid solver
        assert solver.inventory.vsolver.inventory.Solver == "cgrad", \
               'Error: solver.vsolver.Solver must be "cgrad"'

        self.communicator = solver.communicator
        self.srcCommList = solver.myPlus

        # number of processors in the remote solver
        self.remoteSize = len(self.srcCommList)

        # only one of remotePlus is sinkComm
        self.sinkComm = solver.remotePlus[self.communicator.rank]

        self.all_variables = solver.all_variables

        # init'd Convertor singleton, this must be done before any other
        # exchanger call
        from ExchangerLib import initConvertor
        initConvertor(self.inventory.si_unit,
                      self.inventory.cartesian,
                      self.all_variables)

        return


    def launch(self, solver):
        # these functions are defined in the subclass
        self.createMesh()
        self.createSourceSink()
        self.createBC()

        if self.inventory.two_way_communication:
            self.createII()
        return


    def modifyT(self, bbox):
        # modify read-in temperature field
        from ExchangerLib import modifyT
        modifyT(bbox, self.all_variables)
        return


    def preVSolverRun(self):
        # do nothing, overridden by EmbeddedCoupler
        return


    def postVSolverRun(self):
        # do nothing, overridden by ContainingCoupler
        return


    def endTimestep(self, steps, done):
        # exchange predefined signal btwn couplers
        # the signal is used to sync the timesteps
        KEEP_WAITING_SIGNAL = 0
        NEW_STEP_SIGNAL = 1
        END_SIMULATION_SIGNAL = 2

        if done:
            sent = END_SIMULATION_SIGNAL
        elif self.synchronized:
            sent = NEW_STEP_SIGNAL
        else:
            sent = KEEP_WAITING_SIGNAL

        while 1:
            recv = self.exchangeSignal(sent)

            if done or (recv == END_SIMULATION_SIGNAL):
                done = True
                break
            elif recv == KEEP_WAITING_SIGNAL:
                pass
            elif recv == NEW_STEP_SIGNAL:
                if self.synchronized:
                    #print self.name, 'exchanging timestep =', steps
                    self.coupled_steps = self.exchangeSignal(steps)
                    #print self.name, 'exchanged timestep =', self.coupled_steps
                break
            else:
                raise ValueError, \
                      "Unexpected signal value, singnal = %d" % recv

        return done



    class Inventory(Component.Inventory):

        import pyre.inventory as prop

        # updating the temperature field in the containing solver or not
        two_way_communication = prop.bool("two_way_communication", default=True)

        # ensuring consistent inititial temperature fields at the overlapping
        # domain or not
        exchange_initial_temperature = prop.bool("exchange_initial_temperature",
                                                 default=True)

        # pressure from the csolver can be used as a good initial guess
        # for pressure of the esolver
        exchange_pressure = prop.bool("exchange_pressure",
                                      default=True)

        # if si_unit is True, quantities exchanged are in SI units
        si_unit = prop.bool("si_unit", default=False)

        # if cartesion is True, quantities exchanged are in standard
        # (ie. Cartesian) coordinate system
        cartesian = prop.bool("cartesian", default=False)



# version
__id__ = "$Id: Coupler.py 15108 2009-06-02 22:56:46Z tan2 $"

# End of file
