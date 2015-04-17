#include "zdvidreader.h"

#include <vector>
#include <ctime>

#include <QThread>
#include <QElapsedTimer>

#include "zdvidbuffer.h"
#include "zstackfactory.h"
#include "zswctree.h"
#include "zdvidinfo.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidfilter.h"
#include "dvid/zdvidbufferreader.h"
#include "dvid/zdvidurl.h"
#include "zarray.h"
#include "zstring.h"
#include "flyem/zflyemneuronbodyinfo.h"
#include "dvid/zdvidtile.h"
#include "zdvidtileinfo.h"
#include "zobject3dscan.h"
#include "zsparsestack.h"
#include "zdvidversiondag.h"

#include "dvid/libdvidheader.h"

ZDvidReader::ZDvidReader(QObject *parent) :
  QObject(parent)
{
  m_eventLoop = new QEventLoop(this);
  m_dvidClient = new ZDvidClient(this);
  m_timer = new QTimer(this);
  //m_timer->setInterval(1000);

  m_isReadingDone = false;

  //connect(m_dvidClient, SIGNAL(noRequestLeft()), m_eventLoop, SLOT(quit()));
  connect(m_dvidClient, SIGNAL(noRequestLeft()), this, SLOT(endReading()));
  connect(this, SIGNAL(readingDone()), m_eventLoop, SLOT(quit()));
  //connect(m_dvidClient, SIGNAL(requestFailed()), m_eventLoop, SLOT(quit()));
  connect(m_dvidClient, SIGNAL(requestCanceled()), this, SLOT(endReading()));

  connect(m_timer, SIGNAL(timeout()), m_dvidClient, SLOT(cancelRequest()));
}

void ZDvidReader::slotTest()
{
  qDebug() << "ZDvidReader::slotTest";
  qDebug() << QThread::currentThread();

  m_eventLoop->quit();
}

void ZDvidReader::startReading()
{
  m_isReadingDone = false;
}

void ZDvidReader::endReading()
{
  m_isReadingDone = true;

  emit readingDone();
}

bool ZDvidReader::open(
    const QString &serverAddress, const QString &uuid, int port)
{
  m_dvidClient->reset();

  if (serverAddress.isEmpty()) {
    return false;
  }

  if (uuid.isEmpty()) {
    return false;
  }

  m_dvidClient->setServer(serverAddress, port);
  m_dvidClient->setUuid(uuid);

  /*
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(serverAddress.toStdString(), uuid.toStdString(), port);
  if (!bufferReader.isReadable(dvidUrl.getHelpUrl().c_str())) {
    return false;
  }
  */


  m_dvidTarget.set(serverAddress.toStdString(), uuid.toStdString(), port);

  return true;
}

bool ZDvidReader::open(const ZDvidTarget &target)
{
  m_dvidClient->reset();

  if (!target.isValid()) {
    return false;
  }

  /*
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(target);
  if (!bufferReader.isReadable(dvidUrl.getHelpUrl().c_str())) {
    return false;
  }
  */

  m_dvidClient->setDvidTarget(target);

  m_dvidTarget = target;

  return true;
}

bool ZDvidReader::open(const QString &sourceString)
{
  ZDvidTarget target;
  target.setFromSourceString(sourceString.toStdString());
  return open(target);
}

void ZDvidReader::waitForReading()
{
#ifdef _DEBUG_
  std::cout << "Start waiting ..." << std::endl;
  qDebug() << QThread::currentThread();
#endif
  if (!isReadingDone()) {
    m_eventLoop->exec();
  }
}

ZObject3dScan *ZDvidReader::readBody(int bodyId, ZObject3dScan *result)
{
  if (result == NULL) {
    result = new ZObject3dScan;
  }

  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearBodyArray();

  ZDvidRequest request;
  request.setGetObjectRequest(bodyId);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  waitForReading();

  const QVector<ZObject3dScan>& bodyArray = dvidBuffer->getBodyArray();

  if (!bodyArray.empty()) {
    *result = bodyArray[0];
  }

  return result;
}

ZObject3dScan ZDvidReader::readBody(int bodyId)
{
  ZObject3dScan obj;
  readBody(bodyId, &obj);
  return obj;
#if 0
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearBodyArray();

  ZDvidRequest request;
  request.setGetObjectRequest(bodyId);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  waitForReading();

  const QVector<ZObject3dScan>& bodyArray = dvidBuffer->getBodyArray();

  ZObject3dScan obj;

  if (!bodyArray.empty()) {
    obj = bodyArray[0];
  }

  return obj;
#endif
}

ZSwcTree* ZDvidReader::readSwc(int bodyId)
{
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearTreeArray();

  ZDvidRequest request;
  request.setGetSwcRequest(bodyId);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  waitForReading();

  const QVector<ZSwcTree*>& treeArray = dvidBuffer->getSwcTreeArray();

  if (!treeArray.empty()) {
    return treeArray[0]->clone();
  }

  return NULL;
}

ZStack* ZDvidReader::readThumbnail(int bodyId)
{
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearImageArray();

  ZDvidRequest request;
  request.setGetThumbnailRequest(bodyId);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();


  waitForReading();

  const QVector<ZStack*>& imageArray = dvidBuffer->getImageArray();

  ZStack *stack = NULL;
  if (!imageArray.isEmpty()) {
    stack = imageArray[0]->clone();
  }

  return stack;
}

ZStack* ZDvidReader::readGrayScale(const ZIntCuboid &cuboid)
{
  return readGrayScale(cuboid.getFirstCorner().getX(),
                       cuboid.getFirstCorner().getY(),
                       cuboid.getFirstCorner().getZ(),
                       cuboid.getWidth(), cuboid.getHeight(),
                       cuboid.getDepth());
}

std::vector<ZStack*> ZDvidReader::readGrayScaleBlock(
    const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo,
    int blockNumber)
{
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(getDvidTarget());
#ifdef _DEBUG_2
  tic();
#endif

  bufferReader.read(dvidUrl.getGrayScaleBlockUrl(blockIndex.getX(),
                                                 blockIndex.getY(),
                                                 blockIndex.getZ(),
                                                 blockNumber).c_str());
#ifdef _DEBUG_2
  std::cout << "reading time:" << std::endl;
  ptoc();
#endif

#ifdef _DEBUG_2
  tic();
#endif

  std::vector<ZStack*> stackArray(blockNumber, NULL);

  if (bufferReader.getStatus() == ZDvidBufferReader::READ_OK) {
    const QByteArray &data = bufferReader.getBuffer();
    if (data.length() > 0) {
//      int realBlockNumber = *((int*) data.constData());

      ZIntCuboid currentBox = dvidInfo.getBlockBox(blockIndex);
      for (int i = 0; i < blockNumber; ++i) {
        //stackArray[i] = ZStackFactory::makeZeroStack(GREY, currentBox);
        stackArray[i] = new ZStack(GREY, currentBox, 1);
#ifdef _DEBUG_2
        std::cout << data.length() << " " << stack->getVoxelNumber() << std::endl;
#endif
        stackArray[i]->loadValue(data.constData() + i * currentBox.getVolume(),
                         currentBox.getVolume(), stackArray[i]->array8());
        currentBox.translateX(currentBox.getWidth());
      }
    }
  }

#ifdef _DEBUG_2
  std::cout << "parsing time:" << std::endl;
  ptoc();
#endif

  return stackArray;
}

ZStack* ZDvidReader::readGrayScaleBlock(
    const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo)
{
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(getDvidTarget());
  bufferReader.read(dvidUrl.getGrayScaleBlockUrl(blockIndex.getX(),
                                                 blockIndex.getY(),
                                                 blockIndex.getZ()).c_str());
  ZStack *stack = NULL;
  if (bufferReader.getStatus() == ZDvidBufferReader::READ_OK) {
    const QByteArray &data = bufferReader.getBuffer();
    int realBlockNumber = *((int*) data.constData());

    if (!data.isEmpty() && realBlockNumber == 1) {
      ZIntCuboid box = dvidInfo.getBlockBox(blockIndex);
      stack = ZStackFactory::makeZeroStack(GREY, box);
#ifdef _DEBUG_
      std::cout << data.length() << " " << stack->getVoxelNumber() << std::endl;
#endif
      stack->loadValue(data.constData() + 4, data.length() - 4, stack->array8());
    }
  }

  return stack;
}

ZSparseStack* ZDvidReader::readSparseStack(int bodyId)
{
  ZSparseStack *spStack = NULL;

  ZObject3dScan *body = readBody(bodyId, NULL);

  //ZSparseObject *body = new ZSparseObject;
  //body->append(reader.readBody(bodyId));
  //body->canonize();
#ifdef _DEBUG_2
  tic();
#endif

  if (!body->isEmpty()) {
    spStack = new ZSparseStack;
    spStack->setObjectMask(body);

    ZDvidInfo dvidInfo;
    dvidInfo.setFromJsonString(readInfo("grayscale").toStdString());
    ZObject3dScan blockObj = dvidInfo.getBlockIndex(*body);;
    ZStackBlockGrid *grid = new ZStackBlockGrid;
    spStack->setGreyScale(grid);
    grid->setMinPoint(dvidInfo.getStartCoordinates());
    grid->setBlockSize(dvidInfo.getBlockSize());
    grid->setGridSize(dvidInfo.getGridSize());

    /*
    for (ZIntPointArray::const_iterator iter = blockArray.begin();
         iter != blockArray.end(); ++iter) {
         */
    size_t stripeNumber = blockObj.getStripeNumber();
    for (size_t s = 0; s < stripeNumber; ++s) {
      const ZObject3dStripe &stripe = blockObj.getStripe(s);
      int segmentNumber = stripe.getSegmentNumber();
      int y = stripe.getY();
      int z = stripe.getZ();
      for (int i = 0; i < segmentNumber; ++i) {
        int x0 = stripe.getSegmentStart(i);
        int x1 = stripe.getSegmentEnd(i);
        //tic();
#if 0
        const ZIntPoint blockIndex =
            ZIntPoint(x0, y, z) - dvidInfo.getStartBlockIndex();
        std::vector<ZStack*> stackArray =
            readGrayScaleBlock(blockIndex, dvidInfo, x1 - x0 + 1);
        grid->consumeStack(blockIndex, stackArray);
#else

        for (int x = x0; x <= x1; ++x) {
          const ZIntPoint blockIndex =
              ZIntPoint(x, y, z) - dvidInfo.getStartBlockIndex();
          //ZStack *stack = readGrayScaleBlock(blockIndex, dvidInfo);
          //const ZIntPoint blockIndex = *iter - dvidInfo.getStartBlockIndex();
          ZIntCuboid box = grid->getBlockBox(blockIndex);
          ZStack *stack = readGrayScale(box);
          grid->consumeStack(blockIndex, stack);
        }
#endif
        //ptoc();
      }
    }
    //}
  } else {
    delete body;
  }

#ifdef _DEBUG_2
  ptoc();
#endif

  return spStack;
}

ZStack* ZDvidReader::readGrayScale(
    int x0, int y0, int z0, int width, int height, int depth)
{
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearImageArray();

  ZDvidRequest request;

  if (depth == 1) {
    request.setGetImageRequest(x0, y0, z0, width, height);
    m_dvidClient->appendRequest(request);
    m_dvidClient->postNextRequest();
  } else {
    std::vector<std::pair<int, int> > partition =
        partitionStack(x0, y0, z0, width, height, depth);
    for (std::vector<std::pair<int, int> >::const_iterator
         iter = partition.begin(); iter != partition.end(); ++iter) {
      request.setGetImageRequest(x0, y0, iter->first, width, height, iter->second);
      m_dvidClient->appendRequest(request);
      m_dvidClient->postNextRequest();
    }
  }

  waitForReading();

  const QVector<ZStack*>& imageArray = dvidBuffer->getImageArray();

  ZStack *stack = NULL;
  if (!imageArray.isEmpty()) {
    //stack = imageArray[0]->clone();
    if (!imageArray.isEmpty()) {
      stack = ZStackFactory::composite(imageArray.begin(), imageArray.end());
    }
  }

  dvidBuffer->clearImageArray();

  return stack;

#if 0 //old version
  ZDvidRequest request;
  for (int z = 0; z < depth; ++z) {
    request.setGetImageRequest(x0, y0, z0 + z, width, height);
    m_dvidClient->appendRequest(request);
  }
  m_dvidClient->postNextRequest();

  m_eventLoop->exec();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();

  const QVector<ZStack*>& imageArray = dvidBuffer->getImageArray();

  ZStack *stack = NULL;

  if (!imageArray.isEmpty()) {
    stack = ZStackFactory::composite(imageArray.begin(), imageArray.end());
  }

  stack->setOffset(x0, y0, z0);
  dvidBuffer->clearImageArray();

  return stack;
#endif
}

bool ZDvidReader::isReadingDone()
{
  return m_isReadingDone;
}

QString ZDvidReader::readInfo(const QString &dataType)
{
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clear();

  ZDvidRequest request;
  request.setGetInfoRequest(dataType);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  waitForReading();

  const QStringList& infoArray = dvidBuffer->getInfoArray();

  QString info = infoArray.join(" ");
  dvidBuffer->clearInfoArray();

  return info;
}

std::set<int> ZDvidReader::readBodyId(
    const ZIntPoint &firstCorner, const ZIntPoint &lastCorner)
{
  return readBodyId(firstCorner.getX(), firstCorner.getY(), firstCorner.getZ(),
                    lastCorner.getX() - firstCorner.getX() + 1,
                    lastCorner.getY() - firstCorner.getY() + 1,
                    lastCorner.getZ() - firstCorner.getZ() + 1);
}

std::set<int> ZDvidReader::readBodyId(
    int x0, int y0, int z0, int width, int height, int depth)
{
  ZStack *stack = readBodyLabel(x0, y0, z0, width, height, depth);

  std::set<int> bodySet;

  size_t voxelNumber = stack->getVoxelNumber();

  FlyEm::TBodyLabel *labelArray =
      (FlyEm::TBodyLabel*) (stack->array8());
  for (size_t i = 0; i < voxelNumber; ++i) {
    bodySet.insert((int) labelArray[i]);
  }

  delete stack;

  return bodySet;
}

std::set<int> ZDvidReader::readBodyId(const QString sizeRange)
{
  std::set<int> bodySet;

  if (!sizeRange.isEmpty()) {
    std::vector<int> idArray;
    startReading();

    ZDvidRequest request;
    request.setGetStringRequest("sp2body");

    request.setParameter(QVariant("sizerange/" + sizeRange));
    m_dvidClient->appendRequest(request);
    m_dvidClient->postNextRequest();

    waitForReading();

    ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();

    const QStringList& infoArray = dvidBuffer->getInfoArray();

    if (infoArray.size() > 0) {
      ZJsonArray array;
      //qDebug() << infoArray[0];
      array.decode(infoArray[0].toStdString());
      idArray = array.toIntegerArray();
      bodySet.insert(idArray.begin(), idArray.end());
    }

    dvidBuffer->clearInfoArray();
  }

  return bodySet;
}

std::set<int> ZDvidReader::readBodyId(const ZDvidFilter &filter)
{
  std::set<int> bodyIdSet;

  if (filter.hasUpperBodySize()) {
    bodyIdSet = readBodyId(filter.getMinBodySize(), filter.getMaxBodySize());
  } else {
    bodyIdSet = readBodyId(filter.getMinBodySize());
  }

  if (filter.hasExclusion()) {
    std::set<int> newBodySet;
    for (std::set<int>::const_iterator iter = bodyIdSet.begin();
         iter != bodyIdSet.end(); ++iter) {
      int bodyId = *iter;
      if (!filter.isExcluded(bodyId)) {
        newBodySet.insert(bodyId);
      }
    }
    return newBodySet;
  }

  return bodyIdSet;
}

std::set<int> ZDvidReader::readBodyId(size_t minSize)
{
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);
  bufferReader.read(dvidUrl.getBodyListUrl(minSize).c_str());

  std::set<int> bodySet;

  QString idStr = bufferReader.getBuffer().data();

  if (!idStr.isEmpty()) {
    std::vector<int> idArray;
    ZJsonArray array;
      //qDebug() << infoArray[0];
    array.decode(idStr.toStdString());
    idArray = array.toIntegerArray();
    bodySet.insert(idArray.begin(), idArray.end());
  }

  return bodySet;
}

std::set<int> ZDvidReader::readBodyId(size_t minSize, size_t maxSize)
{
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);
  bufferReader.read(dvidUrl.getBodyListUrl(minSize, maxSize).c_str());

  std::set<int> bodySet;

  QString idStr = bufferReader.getBuffer().data();

  if (!idStr.isEmpty()) {
    std::vector<int> idArray;
    ZJsonArray array;
      //qDebug() << infoArray[0];
    array.decode(idStr.toStdString());
    idArray = array.toIntegerArray();
    bodySet.insert(idArray.begin(), idArray.end());
  }

  return bodySet;
}

QByteArray ZDvidReader::readKeyValue(const QString &dataName, const QString &key)
{
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clear();

  ZDvidRequest request;
  request.setGetKeyValueRequest(dataName, key);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  waitForReading();

  const QVector<QByteArray> &array = dvidBuffer->getKeyValueArray();

  QByteArray keyValue;
  if (!array.isEmpty()) {
    keyValue = array[0];
  }

  dvidBuffer->clearKeyValueArray();

  return keyValue;
}

QStringList ZDvidReader::readKeys(
    const QString &dataName, const QString &minKey)
{
  ZDvidBufferReader reader;
  ZDvidUrl dvidUrl(m_dvidTarget);
  const std::string &maxKey = "\xff";

  reader.read(dvidUrl.getKeyRangeUrl(
                dataName.toStdString(), minKey.toStdString(), maxKey).c_str());
  QByteArray keyBuffer = reader.getBuffer();

  QStringList keys;

  if (!keyBuffer.isEmpty()) {
    ZJsonArray obj;
    obj.decode(keyBuffer.data());
    for (size_t i = 0; i < obj.size(); ++i) {
      keys << ZJsonParser::stringValue(obj.at(i));
    }
  }

  return keys;
}

QStringList ZDvidReader::readKeys(
    const QString &dataName, const QString &minKey, const QString &maxKey)
{
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clear();

  ZDvidRequest request;
  request.setGetKeysRequest(dataName, minKey, maxKey);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  waitForReading();

  const QVector<QByteArray> &array = dvidBuffer->getKeysArray();

  QStringList keys;
  QByteArray keyBuffer;
  if (!array.isEmpty()) {
    keyBuffer = array[0];
  }

  dvidBuffer->clearKeysArray();

  if (!keyBuffer.isEmpty()) {
    ZJsonArray obj;
    obj.decode(keyBuffer.data());
    for (size_t i = 0; i < obj.size(); ++i) {
      keys << ZJsonParser::stringValue(obj.at(i));
    }
  }

  return keys;
}

ZStack* ZDvidReader::readBodyLabel(
    int x0, int y0, int z0, int width, int height, int depth)
{
  startReading();
  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearImageArray();

  ZDvidRequest request;
  std::vector<std::pair<int, int> > partition =
      partitionStack(x0, y0, z0, width, height, depth);
  for (std::vector<std::pair<int, int> >::const_iterator
       iter = partition.begin(); iter != partition.end(); ++iter) {
    request.setGetBodyLabelRequest(
          m_dvidTarget.getBodyLabelName().c_str(),
          x0, y0, iter->first, width, height, iter->second);
    m_dvidClient->appendRequest(request);
    m_dvidClient->postNextRequest();
  }
#if 0
  size_t voxelNumber = (size_t) width * height * depth;
  size_t dvidSizeLimit = MAX_INT32 / 2;
  //if (voxelNumber > dvidSizeLimit) {
    int nseg = voxelNumber / dvidSizeLimit + 1;
    int z = 0;
    int subdepth = depth / nseg;
    while (z < depth) {
      int leftDepth = depth - z;
      if (leftDepth < subdepth) {
        subdepth = leftDepth;
      }
      request.setGetBodyLabelRequest(x0, y0, z + z0, width, height, subdepth);
      m_dvidClient->appendRequest(request);
      m_dvidClient->postNextRequest();
      z += subdepth;
    }
  /*} else {
    request.setGetBodyLabelRequest(x0, y0, z0, width, height, depth);
    m_dvidClient->appendRequest(request);
    m_dvidClient->postNextRequest();
  }*/
#endif

  waitForReading();

  const QVector<ZStack*>& imageArray = dvidBuffer->getImageArray();
  ZStack *stack = NULL;
  if (!imageArray.isEmpty()) {
    //stack = imageArray[0]->clone();
    if (!imageArray.isEmpty()) {
      if (imageArray.size() == 1) {
        stack = imageArray[0]->clone();
      } else {
        stack = ZStackFactory::composite(imageArray.begin(), imageArray.end());
      }
    }
  }


  return stack;
}

std::vector<std::pair<int, int> > ZDvidReader::partitionStack(
    int x0, int y0, int z0, int width, int height, int depth)
{
  UNUSED_PARAMETER(x0);
  UNUSED_PARAMETER(y0);
  std::vector<std::pair<int, int> > partition;
  size_t voxelNumber = (size_t) width * height * depth;
  size_t dvidSizeLimit = MAX_INT32 / 2;
  int nseg = voxelNumber / dvidSizeLimit + 1;
  int z = 0;
  int subdepth = depth / nseg;
  while (z < depth) {
    int leftDepth = depth - z;
    if (leftDepth < subdepth) {
      subdepth = leftDepth;
    }
    partition.push_back(std::pair<int, int>(z + z0, subdepth));
    z += subdepth;
  }

  return partition;
}

ZClosedCurve* ZDvidReader::readRoiCurve(
    const std::string &key, ZClosedCurve *result)
{
  if (result != NULL) {
    result->clear();
  }

  QByteArray byteArray = readKeyValue("roi_curve", key.c_str());
  if (!byteArray.isEmpty()) {
    ZJsonObject obj;
    obj.decode(byteArray.constData());

    if (!obj.isEmpty()) {
      if (result == NULL) {
        result = new ZClosedCurve;
      }
      result->loadJsonObject(obj);
    }
  }

  return result;
}

ZIntCuboid ZDvidReader::readBoundBox(int z)
{
  QByteArray byteArray = readKeyValue("bound_box", QString("%1").arg(z));
  ZIntCuboid cuboid;

  if (!byteArray.isEmpty()) {
    ZJsonArray obj;
    obj.decode(byteArray.constData());
    if (obj.size() == 6) {
      cuboid.set(ZJsonParser::integerValue(obj.at(0)),
                 ZJsonParser::integerValue(obj.at(1)),
                 ZJsonParser::integerValue(obj.at(2)),
                 ZJsonParser::integerValue(obj.at(3)),
                 ZJsonParser::integerValue(obj.at(4)),
                 ZJsonParser::integerValue(obj.at(5)));
    }
  }

  return cuboid;
}

ZDvidInfo ZDvidReader::readGrayScaleInfo()
{
  QString infoString = readInfo("grayscale");
  ZDvidInfo dvidInfo;
  if (!infoString.isEmpty()) {
    dvidInfo.setFromJsonString(infoString.toStdString());
  }

  return dvidInfo;
}

bool ZDvidReader::hasData(const std::string &key) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZDvidBufferReader bufferReader;
  return bufferReader.isReadable(dvidUrl.getInfoUrl(key).c_str());
}

ZArray* ZDvidReader::readLabels64(
    const std::string &dataName, int x0, int y0, int z0,
    int width, int height, int depth) const
{

  ZArray *array = NULL;

#if defined(_ENABLE_LIBDVIDCPP_2)
  qDebug() << "Using libdvidcpp";

  const ZDvidTarget &target = getDvidTarget();
  if (!target.getUuid().empty()) {
    libdvid::DVIDNodeService service(
          target.getAddressWithPort(), target.getUuid());

    libdvid::Dims_t dims(3);
    dims[0] = width;
    dims[1] = height;
    dims[2] = depth;

    std::vector<unsigned int> offset(3);
    offset[0] = x0;
    offset[1] = y0;
    offset[2] = z0;

    std::vector<unsigned int> channels(3);
    channels[0] = 0;
    channels[1] = 1;
    channels[2] = 2;

    libdvid::Labels3D labels = service.get_labels3D(
          dataName, dims, offset, channels, false);

    mylib::Dimn_Type arrayDims[3];
    arrayDims[0] = width;
    arrayDims[1] = height;
    arrayDims[2] = depth;
    array = new ZArray(mylib::UINT64_TYPE, 3, arrayDims);
    array->copyDataFrom(labels.get_raw());
    array->setStartCoordinate(0, x0);
    array->setStartCoordinate(1, y0);
    array->setStartCoordinate(2, z0);
  }
#else
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZDvidBufferReader bufferReader;
//  tic();
//  clock_t start = clock();
  QElapsedTimer timer;
  timer.start();
  bufferReader.read(dvidUrl.getLabels64Url(
                      dataName, width, height, depth, x0, y0, z0).c_str());
  std::cout << "label reading time: " << timer.elapsed() << std::endl;

//  qDebug() << timer.elapsed();
//  clock_t finish = clock();

//  std::cout << "label reading time: " << (finish - start) / CLOCKS_PER_SEC << std::endl;
//  std::cout << "label reading time: " << toc() << std::endl;

  if (bufferReader.getStatus() == ZDvidBufferReader::READ_OK) {
    //bufferReader.getBuffer();
    int dims[3];
    dims[0] = width;
    dims[1] = height;
    dims[2] = depth;
    array = new ZArray(mylib::UINT64_TYPE, 3, dims);

    array->setStartCoordinate(0, x0);
    array->setStartCoordinate(1, y0);
    array->setStartCoordinate(2, z0);

    array->copyDataFrom(bufferReader.getBuffer().constData());
  }
#endif

  return array;
}

bool ZDvidReader::hasSparseVolume() const
{
  return hasData(m_dvidTarget.getBodyLabelName());
  //return true;
  //return hasData(ZDvidData::getName(ZDvidData::ROLE_SP2BODY));
}

bool ZDvidReader::hasSparseVolume(int bodyId) const
{
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);

  return  bufferReader.isReadable(
        dvidUrl.getSparsevolUrl(bodyId, getDvidTarget().getBodyLabelName()).c_str());
}

bool ZDvidReader::hasBodyInfo(int bodyId) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);

  ZDvidBufferReader bufferReader;

  return  bufferReader.isReadable(
        dvidUrl.getBodyInfoUrl(bodyId, m_dvidTarget.getBodyLabelName()).c_str());
}

ZFlyEmNeuronBodyInfo ZDvidReader::readBodyInfo(int bodyId)
{
  ZJsonObject obj;

  QByteArray byteArray = readKeyValue(
        ZDvidData::getName(ZDvidData::ROLE_BODY_INFO,
                           ZDvidData::ROLE_BODY_LABEL,
                           m_dvidTarget.getBodyLabelName()).c_str(),
        ZString::num2str(bodyId).c_str());
  if (!byteArray.isEmpty()) {
    obj.decode(byteArray.constData());
  }

  ZFlyEmNeuronBodyInfo bodyInfo;
  bodyInfo.loadJsonObject(obj);

  return bodyInfo;
}

#define MAX_BODY_ID_START 50000000
int ZDvidReader::readMaxBodyId()
{
  ZJsonObject obj;

  QByteArray byteArray = readKeyValue(
        ZDvidData::getName(ZDvidData::ROLE_MAX_BODY_ID),
        m_dvidTarget.getBodyLabelName().c_str());
  if (!byteArray.isEmpty()) {
    obj.decode(byteArray.constData());
  }

  int id = MAX_BODY_ID_START;
  if (obj.hasKey("max_body_id")) {
    id = ZJsonParser::integerValue(obj["max_body_id"]);
  }

  return id;
}

ZDvidTile* ZDvidReader::readTile(int resLevel, int xi0, int yi0, int z0) const
{
  ZDvidTile *tile = new ZDvidTile;
  tile->setResolutionLevel(resLevel);
  tile->setDvidTarget(getDvidTarget());
  tile->setTileIndex(xi0, yi0);
  tile->update(z0);

  return tile;
}

#if 0
ZDvidTile* ZDvidReader::readTile(
    const std::string &dataName, int resLevel, int xi0, int yi0, int z0) const
{
  ZDvidTile *tile = NULL;

//  ZDvidUrl dvidUrl(getDvidTarget());
//  ZDvidBufferReader bufferReader;
//  bufferReader.read(dvidUrl.getTileUrl(dataName, resLevel, xi0, yi0, z0).c_str());
//  QByteArray buffer = bufferReader.getBuffer();

//  ZDvidTileInfo tileInfo = readTileInfo(dataName);

  if (!buffer.isEmpty()) {
    tile = new ZDvidTile;
    tile->setResolutionLevel(resLevel);
    ZDvidTarget target = dataName;
    tile->setDvidTarget(getDvidTarget());
    tile->update(z0);
    /*
    tile->loadDvidPng(buffer);
    tile->setResolutionLevel(resLevel);
    tile->setTileOffset(
          xi0 * tileInfo.getWidth(), yi0 * tileInfo.getHeight(), z0);
          */
  }

  return tile;
}
#endif


ZDvidTileInfo ZDvidReader::readTileInfo(const std::string &dataName) const
{
  ZDvidTileInfo tileInfo;
  ZDvidUrl dvidUrl(getDvidTarget());

  ZDvidBufferReader bufferReader;
  bufferReader.read(dvidUrl.getInfoUrl(dataName).c_str());

  ZJsonObject infoJson;
  infoJson.decodeString(bufferReader.getBuffer().data());
  tileInfo.load(infoJson);

  return tileInfo;
}

ZDvidVersionDag ZDvidReader::readVersionDag() const
{
  return readVersionDag(getDvidTarget().getUuid());
}

ZDvidVersionDag ZDvidReader::readVersionDag(const std::string &uuid) const
{
  ZDvidVersionDag dag;

  ZDvidUrl dvidUrl(getDvidTarget());

  ZDvidBufferReader bufferReader;
  bufferReader.read(dvidUrl.getRepoInfoUrl().c_str());

  ZJsonObject infoJson;
  infoJson.decodeString(bufferReader.getBuffer().data());

  dag.load(infoJson, uuid);

  return dag;
}
