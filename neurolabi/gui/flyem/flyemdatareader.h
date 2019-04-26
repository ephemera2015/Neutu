#ifndef FLYEMDATAREADER_H
#define FLYEMDATAREADER_H

#include <vector>
#include <cstdint>
#include <string>

#include "dvid/zdviddef.h"

class ZDvidReader;
class FlyEmDataConfig;
class ZFlyEmNeuronBodyInfo;
class ZFlyEmBodyAnnotation;
class ZMesh;

/*!
 * \brief The class for wrapping functions of reading flyem data.
 */
class FlyEmDataReader
{
public:
  FlyEmDataReader();

public:
  static FlyEmDataConfig ReadDataConfig(const ZDvidReader &reader);
  static ZFlyEmNeuronBodyInfo ReadBodyInfo(
      const ZDvidReader &reader, uint64_t bodyId);
  static ZFlyEmBodyAnnotation ReadBodyAnnotation(
      const ZDvidReader &reader, uint64_t bodyId);
  static ZMesh* ReadRoiMesh(
      const ZDvidReader &reader, const std::string &roiName);

private:
  static ZMesh* LoadRoi(
      const ZDvidReader &reader, const std::string &roiName,
      const std::string &key, const std::string &source);
  static ZMesh* LoadRoi(
      const ZDvidReader &reader, const std::string &roiName,
      const std::vector<std::string> &keyList, const std::string &source);

#if 0
  static std::vector<ZDvidSynapse> ReadSynapse(
      const ZDvidReader &reader, const ZIntCuboid &box,
      dvid::EAnnotationLoadMode mode = dvid::EAnnotationLoadMode::NO_PARTNER);
  static std::vector<ZDvidSynapse> ReadSynapse(
      const ZDvidReader &reader, uint64_t label,
      dvid::EAnnotationLoadMode mode = dvid::EAnnotationLoadMode::NO_PARTNER);
  static std::vector<ZDvidSynapse> ReadSynapse(
      const ZDvidReader &reader, uint64_t label, const ZDvidRoi &roi,
      dvid::EAnnotationLoadMode mode);

  static ZDvidSynapse ReadSynapse(
      const ZDvidReader &reader, int x, int y, int z,
      dvid::EAnnotationLoadMode mode = dvid::EAnnotationLoadMode::NO_PARTNER);
  static ZDvidSynapse ReadSynapse(
      const ZDvidReader &reader, const ZIntPoint &pt,
      dvid::EAnnotationLoadMode mode = dvid::EAnnotationLoadMode::NO_PARTNER);
#endif
};

#endif // FLYEMDATAREADER_H
