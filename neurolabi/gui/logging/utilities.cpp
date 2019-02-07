#include "utilities.h"

#include "zlog.h"
#include "zqslog.h"
#include "common/neutube_def.h"
#include "zwidgetmessage.h"

namespace neutu {

void LogUrlIO(const QString &action, const QString &url)
{
  KLOG << ZLog::Info() << ZLog::Tag("action", action.toStdString())
       << ZLog::Tag("url", url.toStdString());
}

namespace {

void LogLocalMessage(const ZWidgetMessage &msg)
{
  if (msg.hasTarget(ZWidgetMessage::TARGET_LOG_FILE)) {
    QString plainStr = msg.toPlainString();
    switch (msg.getType()) {
    case neutube::EMessageType::INFORMATION:
      LINFO_NLN() << plainStr;
      break;
    case neutube::EMessageType::WARNING:
      LWARN_NLN() << plainStr;
      break;
    case neutube::EMessageType::ERROR:
      LERROR_NLN() << plainStr;
      break;
    case neutube::EMessageType::DEBUG:
      LDEBUG_NLN() << plainStr;
      break;
    }
  }
}

void LogKafkaMessage(const ZWidgetMessage &msg)
{
  if (msg.hasTarget(ZWidgetMessage::TARGET_KAFKA)) {
    std::string plainStr = msg.toPlainString().toStdString();
    switch (msg.getType()) {
    case neutube::EMessageType::INFORMATION:
      KINFO << plainStr;
      break;
    case neutube::EMessageType::WARNING:
      KWARN << plainStr;
      break;
    case neutube::EMessageType::ERROR:
      KERROR << plainStr;
      break;
    case neutube::EMessageType::DEBUG:
      KDEBUG << ZLog::Debug() << ZLog::Description(plainStr);
      break;
    }
  }
}

}

void LogMessage(const ZWidgetMessage &msg)
{
  LogLocalMessage(msg);
  LogKafkaMessage(msg);
}

}
