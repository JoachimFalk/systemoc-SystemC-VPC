// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

#include "callib.hpp"

#include "IDCT1d.hpp"

#define INAMEblk "test_in.dat"
#define ONAMEblk "test_out.dat"

class m_source_idct: public smoc_actor {
  public:
    smoc_port_out<int> out0;
    smoc_port_out<int> out1;
    smoc_port_out<int> out2;
    smoc_port_out<int> out3;
    smoc_port_out<int> out4;
    smoc_port_out<int> out5;
    smoc_port_out<int> out6;
    smoc_port_out<int> out7;
  private:
    int counter;
#ifndef KASCPAR_PARSING    
    std::ifstream i1; 
#endif

    void process() {
      int myOut;
#ifndef KASCPAR_PARSING
# ifndef NDEBUG
      if (i1.good()) {
# endif
# ifdef NDEBUG
	myOut = counter++;
# else
	i1 >> myOut;
	std::cout << name() << "  write " << myOut << std::endl;
# endif
	out0[0] = myOut;
# ifdef NDEBUG
	myOut = counter++;
# else
	i1 >> myOut;
	std::cout << name() << "  write " << myOut << std::endl;
# endif
	out1[0] = myOut;
# ifdef NDEBUG
	myOut = counter++;
# else
	i1 >> myOut;
	std::cout << name() << "  write " << myOut << std::endl;
# endif
	out2[0] = myOut;
# ifdef NDEBUG
	myOut = counter++;
# else
	i1 >> myOut;
	std::cout << name() << "  write " << myOut << std::endl;
# endif
	out3[0] = myOut;
# ifdef NDEBUG
	myOut = counter++;
# else
	i1 >> myOut;
	std::cout << name() << "  write " << myOut << std::endl;
# endif
	out4[0] = myOut;
# ifdef NDEBUG
	myOut = counter++;
# else
	i1 >> myOut;
	std::cout << name() << "  write " << myOut << std::endl;
# endif
	out5[0] = myOut;
# ifdef NDEBUG
	myOut = counter++;
# else
	i1 >> myOut;
	std::cout << name() << "  write " << myOut << std::endl;
# endif
	out6[0] = myOut;
# ifdef NDEBUG
	myOut = counter++;
# else
	i1 >> myOut;
	std::cout << name() << "  write " << myOut << std::endl;
# endif
	out7[0] = myOut;
# ifndef NDEBUG
      } else {
        std::cout << "File empty! Please create a file with name test_in.dat!" << std::endl;
        exit (1) ;
      }
# endif
#endif //KASCPAR_PARSING
    }

    smoc_firing_state start;
  public:
    m_source_idct( sc_module_name name
#ifndef KASCPAR_PARSING
, size_t periods
#endif
)
      :smoc_actor( name, start ), counter(0) {
//#ifndef KASCPAR_PARSING
      i1.open(INAMEblk);
      start =
	  ((out0.getAvailableSpace() >= 1)  &&
	   (out1.getAvailableSpace() >= 1)  &&
	   (out2.getAvailableSpace() >= 1)  &&
	   (out3.getAvailableSpace() >= 1)  &&
	   (out4.getAvailableSpace() >= 1)  &&
	   (out5.getAvailableSpace() >= 1)  &&
	   (out6.getAvailableSpace() >= 1)  &&
	   (out7.getAvailableSpace() >= 1)  &&
	   (VAR(counter)     < periods*64))
         >> CALL(m_source_idct::process)
         >> start;
//#endif
  }
  ~m_source_idct( ){
//#ifndef KASCPAR_PARSING
        i1.close();
//#endif
  }
};

class m_sink_idct: public smoc_actor {
  public:
    smoc_port_in<int> in0;
    smoc_port_in<int> in1;
    smoc_port_in<int> in2;
    smoc_port_in<int> in3;
    smoc_port_in<int> in4;
    smoc_port_in<int> in5;
    smoc_port_in<int> in6;
    smoc_port_in<int> in7;
  private:
//#ifndef KASCPAR_PARSING
    std::ofstream fo; 
//#endif
    int           foo;
    
    void process() {
#ifndef KASCPAR_PARSING
#ifndef NDEBUG
      std::cout << name() << " receiving " << in[0] << std::endl;
      fo << in[0] << std::endl;
#else
      foo = in[0];
#endif
#endif
    }
    
    smoc_firing_state start;
  public:
    m_sink_idct( sc_module_name name )
      : smoc_actor( name, start ),
        fo(ONAMEblk) {
      start = in(1) >> CALL(m_sink_idct::process)  >> start;
    }
    
    ~m_sink_idct() {
      fo.close();
    
    }
};

class IDCT1D_TEST
: public smoc_graph {
private:
  m_source_idct src_idct;
  m_idct        idct1d;
  m_sink_idct   snk_idct;
public:
  IDCT2d_TEST(sc_module_name name
#ifndef KASCPAR_PARSING
     , size_t periods
#endif
)
    : smoc_graph(name),
      src_idct("src_idct", periods),
      idct1d("idct1d"),
      snk_idct("snk_idct") {
#ifndef KASCPAR_PARSING    
    // src_idct -> idct1d
    connectNodePorts( src_idct.out0, idct1d.i0,    smoc_fifo<int>(2));
    connectNodePorts( src_idct.out1, idct1d.i1,    smoc_fifo<int>(2));
    connectNodePorts( src_idct.out2, idct1d.i2,    smoc_fifo<int>(2));
    connectNodePorts( src_idct.out3, idct1d.i3,    smoc_fifo<int>(2));
    connectNodePorts( src_idct.out4, idct1d.i4,    smoc_fifo<int>(2));
    connectNodePorts( src_idct.out5, idct1d.i5,    smoc_fifo<int>(2));
    connectNodePorts( src_idct.out6, idct1d.i6,    smoc_fifo<int>(2));
    connectNodePorts( src_idct.out7, idct1d.i7,    smoc_fifo<int>(2));
    // idct1d -> snk_idct
    connectNodePorts( idct1d.o0,     snk_idct.in0, smoc_fifo<int>(2));
    connectNodePorts( idct1d.o1,     snk_idct.in1, smoc_fifo<int>(2));
    connectNodePorts( idct1d.o2,     snk_idct.in2, smoc_fifo<int>(2));
    connectNodePorts( idct1d.o3,     snk_idct.in3, smoc_fifo<int>(2));
    connectNodePorts( idct1d.o4,     snk_idct.in4, smoc_fifo<int>(2));
    connectNodePorts( idct1d.o5,     snk_idct.in5, smoc_fifo<int>(2));
    connectNodePorts( idct1d.o6,     snk_idct.in6, smoc_fifo<int>(2));
    connectNodePorts( idct1d.o7,     snk_idct.in7, smoc_fifo<int>(2));
#endif
      }
};

#ifndef KASCPAR_PARSING
int sc_main (int argc, char **argv) {
  bool generateProblemgraph =
    (argc > 1) && !strcmp(argv[1], "--generate-problemgraph");
  size_t periods            =
    (argc > 1 && !generateProblemgraph)
    ? atoi(argv[1])
    : 100;
  
  smoc_top_moc<IDCT1D_TEST> top("top", periods);
  
  if (generateProblemgraph) {
    smoc_modes::dump(std::cout, top);
  } else {  
    sc_start(-1);
  }
  return 0;
}
#endif
