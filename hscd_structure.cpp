#include <hscd_structure.hpp>
#include <systemc/kernel/sc_object_manager.h>

template <typename T_node_type, typename T_chan_init>
void hscd_structure<T_node_type, T_chan_init>::pgAssemble( const sc_module *m, hscd_modes::PGWriter &pgw ) const {
  pgw << "<problemgraph name=\"" << m->name() << "_pg\" id=\"" << pgw.getId() << "\">" << std::endl;
  {
    pgw.indentUp();
    for ( typename nodes_ty::const_iterator iter = getNodes().begin();
          iter != getNodes().end();
          ++iter )
      (*iter)->assemble(pgw);
    for ( typename chan2ports_ty::const_iterator c_iter = getChans().begin();
          c_iter != getChans().end();
          ++c_iter ) {
      for ( typename ports_ty::const_iterator ps_iter = c_iter->second.begin();
            ps_iter != c_iter->second.end();
            ++ps_iter ) {
        if ( !(*ps_iter)->isInput ) {
          for ( typename ports_ty::const_iterator pd_iter = c_iter->second.begin();
                pd_iter != c_iter->second.end();
                ++pd_iter ) {
            if ( (*pd_iter)->isInput ) {
              pgw << "<edge name=\"" << c_iter->first->name() << "\" "
                  << "source=\"" << pgw.getId(*ps_iter) << "\" " 
                  << "target=\"" << pgw.getId(*pd_iter) << "\" "
                  << "id=\"" << pgw.getId(c_iter->first) << "\"/>" << std::endl;
            }
          }
        }
      }
    }
    for ( typename iobind_ty::const_iterator iter = iobind.begin();
          iter != iobind.end();
          ++iter )
      pgw << "<portmapping "
          << "from=\"" << pgw.getId(iter->first) << "\" "
          << "to=\"" << pgw.getId(iter->second) << "\" "
          << "id=\"" << pgw.getId() << "\"/>" << std::endl;
    pgw.indentDown();
  }
  pgw << "</problemgraph>" << std::endl;
}

template <typename T_node_type, typename T_chan_init>
void hscd_structure<T_node_type, T_chan_init>::assemble( const sc_module *m, hscd_modes::PGWriter &pgw ) const {
  if ( iobind.empty() )
    return pgAssemble(m,pgw);
  else {
    pgw << "<process name=\"" << m->name() << "\" id=\"" << pgw.getId(this) << "\">" << std::endl;
    {
      pgw.indentUp();
      for ( sc_pvector<sc_object*>::const_iterator iter = m->get_child_objects().begin();
            iter != m->get_child_objects().end();
            ++iter ) {
  //      if ( *iter == &fire_port )
  //        continue;
        
        const hscd_root_port *port = dynamic_cast<const hscd_root_port *>(*iter);
        
        if ( !port )
          continue;
        pgw << "<port name=\"" << (*iter)->name() << "\" "
            << "type=\"" << (port->isInput ? "in" : "out") << "\" "
            << "id=\"" << pgw.getId(port) << "\"/>" << std::endl;
      }
      pgAssemble(m,pgw);
      pgw.indentDown();
    }
    pgw << "</process>" << std::endl;
  }
}

template void hscd_structure<hscd_choice_node, hscd_fifo>::assemble
  (const sc_module *, hscd_modes::PGWriter&) const;
template void hscd_structure<hscd_transact_node, hscd_fifo>::assemble
  (const sc_module *, hscd_modes::PGWriter&) const;
template void hscd_structure<hscd_fixed_transact_node, hscd_fifo>::assemble
  (const sc_module *, hscd_modes::PGWriter&) const;
