/*  DYNAMO:- Event driven molecular dynamics simulator 
    http://www.marcusbannerman.co.uk/dynamo
    Copyright (C) 2008  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

    This program is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 3 as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "compressionhack.hpp"
#include "../dynamics.hpp"
#include "../units/units.hpp"
#include "../../schedulers/cells.hpp"
#include "../../base/is_simdata.hpp"
#include "../NparticleEventData.hpp"

CSCellHack::CSCellHack(DYNAMO::SimData* nSim, Iflt nGR):
  CSystem(nSim),
  growthRate(nGR)
{
  sysName = "CellularCompressionHack";

  type = NON_EVENT;

  maxOrigDiam = Sim->Dynamics.getLongestInteraction();

  size_t minDiam = 0;

#ifdef DYNAMO_DEBUG
  if (dynamic_cast<CSCells*>(Sim->ptrScheduler) == NULL)
    D_throw() << "Not a cellular scheduler!";
#endif
    
  CVector<> cellDimensions = dynamic_cast<CSCells*>
    (Sim->ptrScheduler)->getCellDimensions();
  
  for (size_t i = 1; i < NDIM; ++i)
    if (cellDimensions[i] < cellDimensions[minDiam])
      minDiam = i;

  dt = (cellDimensions[minDiam] / maxOrigDiam - 1.0) / growthRate;

  I_cout() << "Compression Hack Loaded"
	   << "\nCompression rate = " 
	   << growthRate / Sim->Dynamics.units().unitTime()
	   << "\nSim Units compression rate = " << growthRate
	   << "\nMax diameter of interaction = " 
	   << maxOrigDiam / Sim->Dynamics.units().unitLength()
	   << "\nMinimum cell dimension = "
	   << cellDimensions[minDiam] / Sim->Dynamics.units().unitLength()
	   << "\nFirst halt scheduled for " 
	   << dt / Sim->Dynamics.units().unitTime();
}

void 
CSCellHack::stream(Iflt ndt)
{
  dt -= ndt;
}

CNParticleData 
CSCellHack::runEvent()
{
#ifdef DYNAMO_DEBUG
  if (dynamic_cast<CSCells*>(Sim->ptrScheduler) == NULL)
    D_throw() << "Not a cellular scheduler!";
#endif

  I_cout() << "Rebuilding the cell list, coll = " << Sim->lNColl; 

  CVector<> cellDimensions = dynamic_cast<CSCells*>
    (Sim->ptrScheduler)->getCellDimensions();
  
  int minDiam = 0;
  for (int i = 1; i < NDIM; i++)
    if (cellDimensions[i] < cellDimensions[minDiam])
      minDiam = i;
  
  dynamic_cast<CSCells*>(Sim->ptrScheduler)->reinitialise(1.0001 * cellDimensions[minDiam]);
  
  dt = (cellDimensions[minDiam]/maxOrigDiam - 1.0) / growthRate - Sim->dSysTime;  
  
  return CNParticleData();
}

void
CSCellHack::initialise(size_t nID)
{
  ID = nID;
}

