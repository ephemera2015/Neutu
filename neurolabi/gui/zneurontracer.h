#ifndef ZNEURONTRACER_H
#define ZNEURONTRACER_H

#include "zswcpath.h"
#include "tz_trace_defs.h"
#include "tz_trace_utils.h"
#include "neutube.h"
#include "zstackgraph.h"
#include "tz_locseg_chain.h"

class ZStack;
class ZSwcTree;
class ZSwcConnector;

class ZNeuronTraceSeeder {
public:
  ZNeuronTraceSeeder();
  ~ZNeuronTraceSeeder();

  void sortSeed(Geo3d_Scalar_Field *seedPointArray, Stack *signal,
                Trace_Workspace *ws);

  inline std::vector<Local_Neuroseg>& getSeedArray() { return m_seedArray; }
  inline std::vector<double>& getScoreArray() { return m_seedScoreArray; }

private:
  std::vector<Local_Neuroseg> m_seedArray;
  std::vector<double> m_seedScoreArray;
};

class ZNeuronConstructor {
public:
  ZNeuronConstructor();

  inline void setWorkspace(Connection_Test_Workspace *ws) {
    m_connWorkspace = ws;
  }

  inline void setSignal(Stack *signal) {
    m_signal = signal;
  }

  ZSwcTree *reconstruct(std::vector<Locseg_Chain*> &chainArray);

private:
  Connection_Test_Workspace *m_connWorkspace;
  Stack *m_signal;
};

class ZNeuronTracer
{
public:
  ZNeuronTracer();
  ~ZNeuronTracer();

public:
  ZSwcPath trace(double x, double y, double z);
  void updateMask(const ZSwcPath &branch);
  void setIntensityField(Stack *stack);
  void updateMask(Swc_Tree *tree);
  void setTraceWorkspace(Trace_Workspace *workspace);
  void setConnWorkspace(Connection_Test_Workspace *workspace);

  Swc_Tree* trace(double x1, double y1, double z1, double r1,
                 double x2, double y2, double z2, double r2);

  inline void setBackgroundType(NeuTube::EImageBackground bg) {
    m_backgroundType = bg;
  }

  inline void setResolution(double x, double y, double z) {
    m_resolution[0] = x;
    m_resolution[1] = y;
    m_resolution[2] = z;
  }

  inline void setVertexOption(ZStackGraph::EVertexOption vertexOption) {
    m_vertexOption = vertexOption;
  }

  /*!
   * \brief Auto trace
   */
  ZSwcTree* trace(Stack *stack);

  //Autotrace configuration


  //Helper functions
  static double findBestTerminalBreak(
      const ZPoint &terminalCenter, double terminalRadius,
      const ZPoint &innerCenter, double innerRadius,
      const Stack *stack);

private:
  //Helper functions
  Stack* binarize(const Stack *stack);
  Stack* bwsolid(Stack *stack);
  Stack* enhanceLine(const Stack *stack);
  Geo3d_Scalar_Field* extractSeed(const Stack *mask);
  ZSwcTree *reconstructSwc(const Stack *stack,
                           std::vector<Locseg_Chain*> &chainArray);
  std::vector<Locseg_Chain*> trace(const Stack *stack,
                                   std::vector<Local_Neuroseg> &locsegArray, std::vector<double> &values);
  std::vector<Locseg_Chain*> screenChain(const Stack *stack,
                                         std::vector<Locseg_Chain*> &chainArray);

private:
  Stack *m_stack;
  Trace_Workspace *m_traceWorkspace;
  Connection_Test_Workspace *m_connWorkspace;
  ZSwcConnector *m_swcConnector;
  NeuTube::EImageBackground m_backgroundType;
  ZStackGraph::EVertexOption m_vertexOption;
  double m_resolution[3];

  //Intermedite buffer
  std::vector<Locseg_Chain*> m_chainArray;

};

#endif // ZNEURONTRACER_H
