/*  DYNAMO:- Event driven molecular dynamics simulator 
    http://www.marcusbannerman.co.uk/dynamo
    Copyright (C) 2011  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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

#include "dynamics.hpp"
#include "include.hpp"
#include "../datatypes/vector.hpp"
#include "../datatypes/vector.xml.hpp"
#include "../base/is_simdata.hpp"
#include "NparticleEventData.hpp"
#include "systems/sysTicker.hpp"
#include "../schedulers/scheduler.hpp"

#include <magnet/xmlreader.hpp>
#include <magnet/exception.hpp>
#include <magnet/xmlwriter.hpp>
#include <boost/foreach.hpp>
#include <cmath>

Dynamics::Dynamics(DYNAMO::SimData* tmp): 
  SimBase(tmp,"Dynamics",IC_purple),
  p_BC(NULL), 
  p_liouvillean(NULL)
{}

Dynamics::Dynamics(const magnet::xml::Node& XML, DYNAMO::SimData* tmp): 
  SimBase(tmp, "Dynamics", IC_purple),
  p_BC(NULL)
{ operator<<(XML); }

Dynamics::~Dynamics() {}

magnet::ClonePtr<Topology>& 
Dynamics::getTopology(std::string name)
{
  BOOST_FOREACH(magnet::ClonePtr<Topology>& sysPtr, topology)
    if (sysPtr->getName() == name)
      return sysPtr;
  
  M_throw() << "Could not find the topology " << name;
}

const magnet::ClonePtr<Topology>& 
Dynamics::getTopology(std::string name) const
{
  BOOST_FOREACH(const magnet::ClonePtr<Topology>& sysPtr, topology)
    if (sysPtr->getName() == name)
      return sysPtr;
  
  M_throw() << "Could not find the topology " << name;
}

const Species& 
Dynamics::getSpecies(const Particle& p1) const 
{
  BOOST_FOREACH(const magnet::ClonePtr<Species>& ptr, species)
    if (ptr->isSpecies(p1))
      return *ptr;
  
  M_throw() << "Could not find the requested species"
	    << "\nID = " << p1.getID();
}


xml::XmlStream& operator<<(xml::XmlStream& XML, 
			    const Dynamics& g)
{
  g.outputXML(XML);
  return XML;
}

const Species& 
Dynamics::getSpecies(std::string name) const
{
  BOOST_FOREACH(const magnet::ClonePtr<Species>& ptr, species)
    if (ptr->getName() == name)
      return *ptr;
  
  M_throw() << "Could not find the " << name << " species"; 
}

Species& 
Dynamics::getSpecies(std::string name)
{
  BOOST_FOREACH(magnet::ClonePtr<Species>& ptr, species)
    if (ptr->getName() == name)
      return *ptr;
  
  M_throw() << "Could not find the " << name << " species"; 
}

magnet::ClonePtr<System>&
Dynamics::getSystem(std::string name)
{
  BOOST_FOREACH(magnet::ClonePtr<System>& sysPtr, systems)
    if (sysPtr->getName() == name)
      return sysPtr;
  
  M_throw() << "Could not find system plugin";
}

const magnet::ClonePtr<System>&
Dynamics::getSystem(std::string name) const
{
  BOOST_FOREACH(const magnet::ClonePtr<System>& sysPtr, systems)
    if (sysPtr->getName() == name)
      return sysPtr;
  
  M_throw() << "Could not find system plugin";
}

magnet::ClonePtr<Global>&
Dynamics::getGlobal(std::string name)
{
  BOOST_FOREACH(magnet::ClonePtr<Global>& sysPtr, globals)
    if (sysPtr->getName() == name)
      return sysPtr;
  
  M_throw() << "Could not find global plugin";
}

const magnet::ClonePtr<Global>&
Dynamics::getGlobal(std::string name) const
{
  BOOST_FOREACH(const magnet::ClonePtr<Global>& sysPtr, globals)
    if (sysPtr->getName() == name)
      return sysPtr;
  
  M_throw() << "Could not find global plugin";
}

magnet::ClonePtr<Local>&
Dynamics::getLocal(std::string name)
{
  BOOST_FOREACH(magnet::ClonePtr<Local>& sysPtr, locals)
    if (sysPtr->getName() == name)
      return sysPtr;
  
  M_throw() << "Could not find local plugin";
}

const magnet::ClonePtr<Local>&
Dynamics::getLocal(std::string name) const
{
  BOOST_FOREACH(const magnet::ClonePtr<Local>& sysPtr, locals)
    if (sysPtr->getName() == name)
      return sysPtr;
  
  M_throw() << "Could not find local plugin";
}

magnet::ClonePtr<Interaction>&
Dynamics::getInteraction(std::string name)
{
  BOOST_FOREACH(magnet::ClonePtr<Interaction>& sysPtr, interactions)
    if (sysPtr->getName() == name)
      return sysPtr;
  
  M_throw() << "Could not find interaction plugin";
}

const magnet::ClonePtr<Interaction>&
Dynamics::getInteraction(std::string name) const
{
  BOOST_FOREACH(const magnet::ClonePtr<Interaction>& sysPtr, interactions)
    if (sysPtr->getName() == name)
      return sysPtr;
  
  M_throw() << "Could not find interaction plugin";
}

void 
Dynamics::addSpecies(const magnet::ClonePtr<Species>& sp)
{
  if (Sim->status >= INITIALISED)
    M_throw() << "Cannot add species after simulation initialisation";

  species.push_back(sp);

  BOOST_FOREACH(magnet::ClonePtr<Interaction>& intPtr , interactions)
    if (intPtr->isInteraction(*species.back()))
      {
	species.back()->setIntPtr(intPtr.get_ptr());
	return;
      }

  M_throw() << "Could not find the interaction for the species \"" 
	    << sp->getName() << "\"";
}

void 
Dynamics::addGlobal(Global* newGlobal)
{
  if (Sim->status >= INITIALISED)
    M_throw() << "Cannot add global events after simulation initialisation";

  magnet::ClonePtr<Global> 
    tempPlug(newGlobal);
  
  globals.push_back(tempPlug);
}

void 
Dynamics::addLocal(Local* newLocal)
{
  if (Sim->status >= INITIALISED)
    M_throw() << "Cannot add local events after simulation initialisation";

  magnet::ClonePtr<Local> 
    tempPlug(newLocal);
  
  locals.push_back(tempPlug);
}

void
Dynamics::addSystem(System* newSystem)
{
  if (Sim->status >= INITIALISED)
    M_throw() << "Cannot add system events at this time, system is initialised";
  
  magnet::ClonePtr<System> 
    tempPlug(newSystem);
  
  systems.push_back(tempPlug); 
}

void
Dynamics::addStructure(Topology* newSystem)
{ 
  if (Sim->status >= INITIALISED)
    M_throw() << "Cannot add structure after simulation initialisation";

  magnet::ClonePtr<Topology> 
    tempPlug(newSystem);
  
  topology.push_back(tempPlug); 
}

void 
Dynamics::addSystemTicker()
{
  if (Sim->status >= INITIALISED)
    M_throw() << "Cannot add the system ticker now";

  BOOST_FOREACH(magnet::ClonePtr<System>& ptr, systems)
    if (ptr->getName() == "SystemTicker")
      M_throw() << "System Ticker already exists";
  
    addSystem(new CSTicker(Sim, Sim->lastRunMFT, "SystemTicker"));
}

Interaction* 
Dynamics::addInteraction(Interaction* CInt)
{
  magnet::ClonePtr<Interaction> tempPlug(CInt);
  interactions.push_back(tempPlug);
  return interactions.back().get_ptr();
}

void 
Dynamics::initialise()
{
  BOOST_FOREACH(magnet::ClonePtr<Species>& ptr, species)
    ptr->initialise();
  
  unsigned int count = 0;
  //Now confirm that every species has only one species type!
  BOOST_FOREACH(const Particle& part, Sim->particleList)
    {
      BOOST_FOREACH(magnet::ClonePtr<Species>& ptr, species)
	if (ptr->isSpecies(part)) { count++; break; }
      
      if (count < 1)
	M_throw() << "Particle ID=" << part.getID() << " has no species";

      if (count > 1)
	M_throw() << "Particle ID=" << part.getID() << " has more than one species";
      count = 0;
    }

  //Now confirm that there are not more counts from each species than there are particles
  {
    unsigned long tot = 0;
    BOOST_FOREACH(magnet::ClonePtr<Species>& ptr, species)
      tot += ptr->getCount();
    
    if (tot < Sim->N)
      M_throw() << "The particle count according to the species definition is too low\n"
		<< "discrepancy = " << tot - Sim->N
		<< "\nN = " << Sim->N;
    
    if (tot > Sim->N)
      M_throw() << "The particle count according to the species definition is too high\n"
		<< "discrepancy = " << tot - Sim->N
		<< "\nN = " << Sim->N;
 }

  p_liouvillean->initialise();

  size_t ID=0;

  BOOST_FOREACH(magnet::ClonePtr<Interaction>& ptr, interactions)
    ptr->initialise(ID++);

  ID=0;

  //Must be initialised before globals. Neighbour lists are
  //implemented as globals and must initialise where locals are and their ID.
  BOOST_FOREACH(magnet::ClonePtr<Local>& ptr, locals)
    ptr->initialise(ID++);

  ID=0;

  BOOST_FOREACH(magnet::ClonePtr<Global>& ptr, globals)
    ptr->initialise(ID++);

  ID=0;

  BOOST_FOREACH(magnet::ClonePtr<System>& ptr, systems)
    ptr->initialise(ID++);
}

const magnet::ClonePtr<Interaction>&
Dynamics::getInteraction(const Particle& p1, const Particle& p2) const 
{
  BOOST_FOREACH(const magnet::ClonePtr<Interaction>& ptr, interactions)
    if (ptr->isInteraction(p1,p2))
      return ptr;
  
  M_throw() << "Could not find the interaction requested";
}

Dynamics::Dynamics(const Dynamics &dyn):
  SimBase(dyn),
  p_BC(dyn.p_BC), 
  _units(dyn._units)
{}

void 
Dynamics::stream(const double& dt)
{
  p_BC->update(dt);

  p_liouvillean->stream(dt);

  BOOST_FOREACH(magnet::ClonePtr<System>& ptr, systems)
    ptr->stream(dt);
}


double
Dynamics::calcInternalEnergy() const
{
  double intECurrent = 0.0;

  BOOST_FOREACH(const magnet::ClonePtr<Interaction> & plugptr, 
		Sim->dynamics.getInteractions())
    intECurrent += plugptr->getInternalEnergy();

  return intECurrent;
}

double
Dynamics::getSimVolume() const
{ 
  double vol = 1.0;
  for (size_t iDim = 0; iDim < NDIM; iDim++)
    vol *= Sim->primaryCellSize[iDim];
  return vol;
}


double
Dynamics::getNumberDensity() const
{
  return Sim->N / getSimVolume();
}

double 
Dynamics::getPackingFraction() const
{
  double volume = 0.0;
  
  BOOST_FOREACH(const magnet::ClonePtr<Species>& sp, Sim->dynamics.getSpecies())
    BOOST_FOREACH(const size_t& ID, *(sp->getRange()))
    volume += sp->getIntPtr()->getExcludedVolume(ID);
  
  return  volume / getSimVolume();
}

void 
Dynamics::setCOMVelocity(const Vector COMVelocity)
{  
  Vector sumMV(0,0,0);
 
  long double sumMass(0);

  //Determine the discrepancy VECTOR
  BOOST_FOREACH(Particle & Part, Sim->particleList)
    {
      Vector  pos(Part.getPosition()), vel(Part.getVelocity());
      BCs().applyBC(pos,vel);
      double mass = getSpecies(Part).getMass(Part.getID());
      //Note we sum the negatives!
      sumMV -= vel * mass;
      sumMass += mass;
    }
  
  sumMV /= sumMass;
  
  sumMV += COMVelocity;

  BOOST_FOREACH(Particle & Part, Sim->particleList)
    Part.getVelocity() =  Part.getVelocity() + sumMV;
}

void
Dynamics::operator<<(const magnet::xml::Node& XML)
{
  I_cout() << "Loading dynamics from XML";
  
  magnet::xml::Node xDynamics = XML.getNode("Dynamics");

  //Load the Primary cell's size
  Sim->primaryCellSize << xDynamics.getNode("SimulationSize");
  Sim->primaryCellSize /= Sim->dynamics.units().unitLength();

  //Now load the BC
  p_BC.set_ptr(BoundaryCondition::getClass(xDynamics.getNode("BC"), Sim));
  
  if (xDynamics.getNode("Topology").valid())
    {
      size_t i(0);
      for (magnet::xml::Node node = xDynamics.getNode("Topology").getNode("Structure");
	   node.valid(); ++node, ++i)
	{
	  magnet::ClonePtr<Topology> tempPlug(Topology::getClass(node, Sim, i));
	  topology.push_back(tempPlug);
	}
    }
  
  { 
    size_t i(0);
    for (magnet::xml::Node node = xDynamics.getNode("Genus").getNode("Species");
	 node.valid(); ++node, ++i)
      species.push_back(magnet::ClonePtr<Species>(Species::getClass(node, Sim, i)));
  }

  p_liouvillean.set_ptr(Liouvillean::loadClass(xDynamics.getNode("Liouvillean"), Sim));  
  
  for (magnet::xml::Node node = xDynamics.getNode("Interactions").getNode("Interaction");
       node.valid(); ++node)
    {
      magnet::ClonePtr<Interaction> tempPlug(Interaction::getClass(node, Sim));
      interactions.push_back(tempPlug);
    }  
  
  //Link the species and interactions
  BOOST_FOREACH(magnet::ClonePtr<Species>& sp , species)
    BOOST_FOREACH(magnet::ClonePtr<Interaction>& intPtr , interactions)
    if (intPtr->isInteraction(*sp))
      {
	sp->setIntPtr(intPtr.get_ptr());
	break;
      }
  
  if (xDynamics.getNode("Globals").valid())
    for (magnet::xml::Node node = xDynamics.getNode("Globals").getNode("Global"); 
	 node.valid(); ++node)
      {
	magnet::ClonePtr<Global> tempPlug(Global::getClass(node, Sim));
	globals.push_back(tempPlug);
      }

  if (xDynamics.getNode("Locals").valid())
    for (magnet::xml::Node node = xDynamics.getNode("Locals").getNode("Local"); 
	 node.valid(); ++node)
      {
	magnet::ClonePtr<Local> tempPlug(Local::getClass(node, Sim));
	locals.push_back(tempPlug);
      }
  
  if (xDynamics.getNode("SystemEvents").valid())
    for (magnet::xml::Node node = xDynamics.getNode("SystemEvents").getNode("System"); 
	 node.valid(); ++node)
      {
	magnet::ClonePtr<System> tempPlug(System::getClass(node, Sim));
	systems.push_back(tempPlug);
      }
}

void
Dynamics::outputXML(xml::XmlStream &XML) const
{
  XML << xml::tag("Dynamics")
      << xml::tag("SimulationSize")
      << Sim->primaryCellSize / Sim->dynamics.units().unitLength()
      << xml::endtag("SimulationSize")
      << xml::tag("BC")
      << p_BC
      << xml::endtag("BC")
      << xml::tag("Genus");
  
  BOOST_FOREACH(const magnet::ClonePtr<Species>& ptr, species)
    XML << xml::tag("Species")
	<< ptr
	<< xml::endtag("Species");
  
  XML << xml::endtag("Genus")
      << xml::tag("Topology");
  
  BOOST_FOREACH(const magnet::ClonePtr<Topology>& ptr, topology)
    XML << xml::tag("Structure")
	<< ptr
	<< xml::endtag("Structure");
  
  XML << xml::endtag("Topology")
      << xml::tag("SystemEvents");
  
  BOOST_FOREACH(const magnet::ClonePtr<System>& ptr, systems)
    XML << ptr;
  
  XML << xml::endtag("SystemEvents")
      << xml::tag("Globals");
  
  BOOST_FOREACH(const magnet::ClonePtr<Global>& ptr, globals)
    XML << xml::tag("Global")
	<< ptr
	<< xml::endtag("Global");
  
  XML << xml::endtag("Globals")
      << xml::tag("Locals");
  
  BOOST_FOREACH(const magnet::ClonePtr<Local>& ptr, locals)
    XML << xml::tag("Local")
	<< ptr
	<< xml::endtag("Local");
  
  XML << xml::endtag("Locals")
      << xml::tag("Interactions");
  
  BOOST_FOREACH(const magnet::ClonePtr<Interaction>& ptr, interactions)
    XML << xml::tag("Interaction")
	<< ptr
	<< xml::endtag("Interaction");
  
  XML << xml::endtag("Interactions")
      << xml::tag("Liouvillean")
      << p_liouvillean
      << xml::endtag("Liouvillean")
      << xml::endtag("Dynamics");
}

double 
Dynamics::getLongestInteraction() const
{
  double maxval = 0.0;

  BOOST_FOREACH(const magnet::ClonePtr<Interaction>& ptr, interactions)
    if (ptr->maxIntDist() > maxval)
      maxval = ptr->maxIntDist();

  return maxval;
}


void 
Dynamics::SystemOverlapTest()
{
  p_liouvillean->updateAllParticles();

  std::vector<Particle>::const_iterator iPtr1, iPtr2;
  
  for (iPtr1 = Sim->particleList.begin(); iPtr1 != Sim->particleList.end(); ++iPtr1)
    for (iPtr2 = iPtr1 + 1; iPtr2 != Sim->particleList.end(); ++iPtr2)    
      getInteraction(*iPtr1, *iPtr2)->checkOverlaps(*iPtr1, *iPtr2);

  BOOST_FOREACH(const Particle& part, Sim->particleList)
    BOOST_FOREACH(const magnet::ClonePtr<Local>& lcl, locals)
    if (lcl->isInteraction(part))
      lcl->checkOverlaps(part);
    
}

void 
Dynamics::setLiouvillean(Liouvillean* Uptr) 
{ p_liouvillean.set_ptr(Uptr); }
