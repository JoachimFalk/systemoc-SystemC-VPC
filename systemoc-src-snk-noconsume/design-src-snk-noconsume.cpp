// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

template <typename T>
class m_h_src: public smoc_actor {
public:
  smoc_port_out<T> out;
private:
  T i;
  
  void src() {
    std::cout << name() << ": " << i << std::endl;
    out[0] = i++;
  }
  smoc_firing_state start;
public:
  m_h_src(sc_module_name name)
    : smoc_actor(name, start),
      i(1) {
    start = out(1) >> call(&m_h_src::src) >> start;
  }
};


template <typename T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<T> in;
private:
  int i;
  
  void sink(void) { std::cout << name() << ": " << in[0] << std::endl; }
  
  smoc_firing_state s0, s1;
public:
  m_h_sink(sc_module_name name)
    : smoc_actor(name, s0) {
    s0 = in(0,1) >> call(&m_h_sink::sink) >> s1;
    s1 = in(1)   >> call(&m_h_sink::sink) >> s0;
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_src<int>     src;
  m_h_sink<int>    snk;
public:
  m_h_top( sc_module_name name )
    : smoc_graph(name),
      src("src"), snk("snk") {
    connectNodePorts(src.out, snk.in);
  }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_h_top> top("top");
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, top);
  } else {
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}
