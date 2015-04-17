#ifndef ZPAINTER_H
#define ZPAINTER_H

#include <vector>

#include "zqtheader.h"

#ifdef _QT_GUI_USED_
#include <QPainter>
#endif

#include "zpoint.h"
#include "zsttransform.h"

class ZIntPoint;
class ZImage;
class ZPixmap;
class QPointF;
class QRectF;
class QRect;
class QTransform;

/*!
 * \brief The painter class using QPainter to draw objects with extended options
 */
class ZPainter
{
public:
  ZPainter();
  explicit ZPainter(QPaintDevice * device);
  explicit ZPainter(ZImage *image);
  explicit ZPainter(ZPixmap *pixmap);

  bool begin(ZImage *image);
  bool begin(ZPixmap *pixmap);
  bool begin(QPaintDevice *device);
  bool end();

  void save();
  void restore();


  void setStackOffset(int x, int y, int z);
  void setStackOffset(const ZIntPoint &offset);
  void setStackOffset(const ZPoint &offset);
  void setZOffset(int z);

  inline int getZOffset() { return m_z; }

  void setPainted(bool painted) {
    m_isPainted = painted;
  }

  inline bool isPainted() {
    return m_isPainted;
  }

  //inline ZPoint getOffset() { return m_transform.getOffset(); }

  void drawImage(
      const QRectF &targetRect, const ZImage &image, const QRectF &sourceRect);
  void drawImage(int x, int y, const ZImage &image);

  void drawPixmap(
      const QRectF &targetRect, const ZPixmap &image, const QRectF &sourceRect);
  void drawPixmap(int x, int y, const ZPixmap &image);


  void setPen(const QColor &color);
  void setPen(const QPen &pen);
  void setPen(Qt::PenStyle style);

  void setBrush(const QColor &color);
  void setBrush(const QBrush &pen);
  void setBrush(Qt::BrushStyle style);

  const QBrush& getBrush() const;
  const QPen& getPen() const;
  QColor getPenColor() const;

  const QTransform& getTransform() const;
  void setTransform(const QTransform &t, bool combine = false);

  void drawPoint(const QPointF &pt);
  void drawPoint(const QPoint &pt);

  void drawPoints(const QPointF *points, int pointCount);
  void drawPoints(const QPoint *points, int pointCount);
  void drawPoints(const std::vector<QPoint> &pointArray);
  void drawPoints(const std::vector<QPointF> &pointArray);

  void drawLine(int x1, int y1, int x2, int y2);
  void drawLine(const QPointF &pt1, const QPointF &pt2);
  void drawLines(const QLine *lines, int lineCount);
  void drawLines(const std::vector<QLine> &lineArray);

  void	drawEllipse(const QRectF & rectangle);
  void	drawEllipse(const QRect & rectangle);
  void	drawEllipse(int x, int y, int width, int height);
  void	drawEllipse(const QPointF & center, double rx, double ry);
  void	drawEllipse(const QPoint & center, int rx, int ry);

  void	drawRect(const QRectF & rectangle);
  void	drawRect(const QRect & rectangle);
  void	drawRect(int x, int y, int width, int height);

  void	drawPolyline(const QPointF * points, int pointCount);
  void	drawPolyline(const QPoint * points, int pointCount);

  void setCompositionMode(QPainter::CompositionMode mode);
  void setRenderHints(QPainter::RenderHints hints, bool on = true);
  void setRenderHint(QPainter::RenderHint hint, bool on = true);

  void fillRect(const QRect &r, Qt::GlobalColor color);
  void setOpacity(double alpha);

  /*
  const QRect& getFieldOfView() const {
    return m_projRegion;
  }
  */

private:
  QPainter m_painter;
  int m_z;
  bool m_isPainted;

  //ZStTransform m_transform; //world coordinates to canvas coordinates
//  ZPoint m_offset;
  //QRect m_projRegion;
};

#endif // ZPAINTER_H
