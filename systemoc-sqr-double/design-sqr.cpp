// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>
#include <vector>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

class m_disp: public smoc_actor {
  public:
    
    smoc_port_in<double> in;
    smoc_port_out<double> out1;
    smoc_port_out<double> out2;
    
  private:
    smoc_firing_state start;

    void copy1() {
      std::cout << "DISP: 1 -> 1" << std::endl;
      out1[0] = in[0];
    }
    
    void copy2() {
      std::cout << "DISP: 1 -> 2" << std::endl;
      out2[0] = in[0];
    }
    
  public:
    m_disp(sc_module_name name, int np_in = 1, int np_out = 1) :
      smoc_actor(name, start) 
    {
    
      smoc_transition_list lst;
      
      lst |=
	     in(1)
	  >> out1(1)
	  >> CALL(m_disp::copy1)
	  >> start;
	
	lst |=
	     in(1)
	  >> out2(1)
	  >> CALL(m_disp::copy2) 
	  >> start;
	
	start = lst;
    }
};

class m_coll: public smoc_actor {
  public:
    smoc_port_in<double> in1;
    smoc_port_in<double> in2;
    smoc_port_out<double> out;
  private:
    smoc_firing_state start;

    void copy1() {
      std::cout << "COLL: 1 -> 1" << std::endl;
      out[0] = in1[0];
    }

    void copy2() {
      std::cout << "COLL: 2 -> 1" << std::endl;
      out[0] = in2[0];
    }
  public:
    m_coll(sc_module_name name)
      : smoc_actor(name, start) {
	
	start = in1(1) >> out(1) >> CALL(m_coll::copy1) >> start
	      | in2(1) >> out(1) >> CALL(m_coll::copy2) >> start;
      }
};

class m_src: public smoc_actor {
public:
  smoc_port_out<double> out;
private:
  int i;
  int n;
  
  bool nTimesTrue() const { return n != 0 ;}
  
  void src() {
    std::cout << "src: " << i << std::endl;
    out[0] = i++;
    if(n>0) n--;
  }
  
  smoc_firing_state start;
public:
  m_src(sc_module_name name, int times)
    : smoc_actor(name, start), i(1), n(times)
    { start = ( out(1) && GUARD(m_src::nTimesTrue) ) >>
              CALL(m_src::src)                       >> start; }
};

class m_approx: public smoc_actor {
public:
  smoc_port_in<double>  i1, i2;
  smoc_port_out<double> o1;
private:
  void approx(void) { o1[0] = (i1[0] / i2[0] + i2[0]) / 2; }
  
  smoc_firing_state start;
public:
  m_approx(sc_module_name name)
    : smoc_actor(name, start)
    { start = (i1(1) && i2(1)) >> o1(1) >>
              CALL(m_approx::approx)   >> start; }
};

class m_dup: public smoc_actor {
public:
  smoc_port_in<double>  i1;
  smoc_port_out<double> o1, o2;

private:
  void dup() {
    double in = i1[0];
    o1[0] = in;
    o2[0] = in;
  }
  
  smoc_firing_state start;
public:
  m_dup(sc_module_name name)
    : smoc_actor(name, start)
    { start = i1(1) >> (o1(1) && o2(1)) >>
              CALL(m_dup::dup)         >> start; }
};

class m_sqrloop: public smoc_actor {
public:
  smoc_port_in<double>  i1, i2;
  smoc_port_out<double> o1, o2;
private:
  double tmp_i1;
  
  // action functions for FSM defined in constructor
  void store() { tmp_i1 = i1[0]; }
  void copy1() { o1[0] = tmp_i1; }
  void copy2() { o1[0] = tmp_i1; o2[0] = i2[0]; }
  
  // guard  functions for FSM defined in constructor
  bool check() const {
    std::cout << name() << "-check: " << tmp_i1 << ", " << i2[0] << std::endl;
    return fabs(tmp_i1 - i2[0]*i2[0]) < 0.0001;
  }
  
  smoc_firing_state start;
  smoc_firing_state loop;
public:
  m_sqrloop(sc_module_name name)
    : smoc_actor( name, start ) {
    start = i1(1)                                 >>
            CALL(m_sqrloop::store)               >> loop;
    loop  = (i2(1) &&  GUARD(m_sqrloop::check))  >>
            (o1(1) && o2(1))                      >>
            CALL(m_sqrloop::copy2)               >> start
          | (i2(1) && !GUARD(m_sqrloop::check))  >>
            o1(1)                                 >>
            CALL(m_sqrloop::copy1)               >> loop;
  }
};

class m_sink: public smoc_actor {
public:
  smoc_port_in<double> in;
private:
  void sink(void) { std::cout << "sink: " << in[0] << std::endl; }
  
  smoc_firing_state start;
public:
  m_sink(sc_module_name name)
    : smoc_actor(name, start)
    { start = in(1) >> CALL(m_sink::sink) >> start; }
};

class m_approx_loop
: public smoc_graph {
  public:
    smoc_port_in<double>  i1;
    smoc_port_out<double> o1;
  protected:
    m_sqrloop sqrloop;
    m_approx  approx;
    m_dup     dup;
  public:
    m_approx_loop( sc_module_name name )
      : smoc_graph(name),
        sqrloop("sqrloop"),
        approx("approx"),
        dup("dup") {
      sqrloop.i1(i1);
      connectNodePorts(sqrloop.o1, approx.i1);
      connectNodePorts(approx.o1, dup.i1, smoc_fifo<double>() << 2 );
      connectNodePorts(dup.o1, approx.i2);
      connectNodePorts(dup.o2, sqrloop.i2);
      sqrloop.o2(o1);
    }
};

class m_top
: public smoc_graph {
  public:
  protected:
    m_src           src;
    m_approx_loop   a1;
    m_approx_loop   a2;
    m_sink          sink;
    m_disp          disp;
    m_coll          coll;
  public:
    m_top( sc_module_name name )
      : smoc_graph(name),
        src("src", 20),
        a1("a1"),
	a2("a2"),
        sink("sink"),
        disp("disp"),
        coll("coll") {
      connectNodePorts(src.out, disp.in);
      
      connectNodePorts(disp.out1, a1.i1);
      connectNodePorts(disp.out2, a2.i1);
      
      connectNodePorts(a1.o1, coll.in1);
      connectNodePorts(a2.o1, coll.in2);

      connectNodePorts(coll.out, sink.in);
    }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, top);
  } else {
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}