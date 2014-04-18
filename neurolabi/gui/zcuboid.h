#ifndef ZCUBOID_H
#define ZCUBOID_H

#include <vector>
#include "zpoint.h"

/*************
 *
 *        4________5       z /
 *       /|        /|       /
 *    0 /_______1 / |       x--->
 *     |  .      |  |     y |
 *     |  |      |  |       v
 *     | 6._ . _ |_ |7
 *     | /       | /
 *     |_________|/
 *    2          3
 *
 * The order is determined by binary increment: 000 (zyx)->111, where 0 and 1
 * indicate min and max repectively.
 ************/

class ZCuboid
{
public:
  ZCuboid();
  ZCuboid(double x1, double y1, double z1, double x2, double y2, double z2);
  ZCuboid(const ZCuboid &cuboid);

  void set(double x1, double y1, double z1, double x2, double y2, double z2);
  void set(const double *corner);

  /*!
   * \brief Test if the bound box is valid
   *
   * \return true iff the locations of corners conform to the definition.
   */
  bool isValid() const;

  /*!
   * \brief Make the cuboid invalid
   */
  void invalidate();

  double width();
  double height();
  double depth();
  double volume();
  void intersect(const ZCuboid &cuboid);
  void bind(const ZCuboid &cuboid); //union

  double moveOutFrom(ZCuboid &cuboid, double margin = 0.0);

  void layout(std::vector<ZCuboid> *cuboidArray, double margin = 0.0);

  ZCuboid& operator= (const ZCuboid &cuboid);
  double& operator[] (int index);
  const double& operator[] (int index) const;

  double estimateSeparateScale(const ZCuboid &cuboid, const ZPoint &vec) const;

  void scale(double s);
  void expand(double margin);

  void joinX(double x);
  void joinY(double y);
  void joinZ(double z);
  /*!
   * \brief Expand the box minimally to include a point.
   */
  void include(const ZPoint &point);

  void print();

  ZPoint corner(int index) const;
  ZPoint center() const;

  inline ZPoint firstCorner() const { return m_firstCorner; }
  inline ZPoint lastCorner() const { return m_lastCorner; }

  double computeDistance(const ZCuboid &box) const;

private:
  static double computeDistance(double minX1, double maxX1,
                                double minX2, double maxX2);

private:
  ZPoint m_firstCorner;
  ZPoint m_lastCorner;
};

#endif // ZCUBOID_H
