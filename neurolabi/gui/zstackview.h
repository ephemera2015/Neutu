/**@file zstackview.h
 * @brief Stack view
 * @author Ting Zhao
 */

#ifndef _ZSTACKVIEW_H_
#define _ZSTACKVIEW_H_

#include <QImage>
#include <QWidget>
#include <QPixmap>
#include <vector>

#include "zstackframe.h"
#include "zparameter.h"
#include "tz_image_lib_defs.h"
#include "neutube.h"
#include "zpaintbundle.h"
#include "zsharedpointer.h"
#include "zmessageprocessor.h"
#include "zpainter.h"
#include "zmultiscalepixmap.h"
//#include "zstackdoc.h"

class ZStackDoc;
class ZStackPresenter;
class QSlider;
class ZImageWidget;
class QVBoxLayout;
class QHBoxLayout;
class QSpinBox;
class ZImage;
class QLabel;
class ZSlider;
class QMenu;
class QPushButton;
class QProgressBar;
class QRadioButton;
class ZStack;
class ZStackViewParam;
class ZMessageManager;
class ZBodySplitButton;
class ZStackMvc;
class ZPixmap;
class ZLabeledSpinBoxWidget;
class QSpacerItem;
class ZWidgetMessage;
class ZStTransform;
class ZScrollSliceStrategy;

/*!
 * \brief The ZStackView class shows 3D data slice by slice
 *
 * ZStackView provides a widget of showing 3D data slice by slice, which can be
 * along any of the three axes. It displays
 * the data in an image screen with the support of slice change and zooming. It
 * is designed to be the view component of the MVC framework, which can be
 * represented by the ZStackFrame class (and its derivatives) or the
 * ZStackMvc class.
 */
class ZStackView : public QWidget {
  Q_OBJECT

public:
  explicit ZStackView(ZStackFrame *parent = 0);
  explicit ZStackView(QWidget *parent = 0);
  virtual ~ZStackView();

public:
  /*!
   * \brief Reset the view status
   *
   * This function is for synchorinizing the view controls such as depth slider
   * and channel controls with stack update in the document.
   */
  void reset(bool updatingScreen = true);

  enum EUpdateOption {
    UPDATE_NONE, //No update
    UPDATE_QUEUED, //Put updating request in a queue
    UPDATE_DIRECT //Update immediately
  };

  /*!
   * \brief Update image screen
   *
   * Update the screen by assuming that all the canvas buffers are ready.
   */
  void updateImageScreen(EUpdateOption option);

  /*!
   * \brief Restore the view from a bad setting
   *
   * The definition of a bad setting is provided by ZImageWidget::isBadView().
   */
  void restoreFromBadView();

  /*!
   * \brief Get the parent frame
   *
   * \return NULL if the parent is not ZStackFrame.
   */
  ZStackFrame* getParentFrame() const;

  /*!
   * \brief Get the parent MVC widget
   *
   * \return NULL if the parent is not ZStackMvc.
   */
  ZStackMvc* getParentMvc() const;

  ZSharedPointer<ZStackDoc> buddyDocument() const;
  ZStackPresenter* buddyPresenter() const;

  QSize sizeHint() const;

  /*!
   * \brief Get the widget of data display
   */
  inline ZImageWidget* imageWidget() const { return m_imageWidget; }

  inline QProgressBar* progressBar() { return m_progress; }

  /*!
   * \brief Get the current slice index.
   *
   * The slice index is the z offset from the current slice to the start slice.
   * Therefore the index of the first slice is always 0.
   */
  int sliceIndex() const;

  /*!
   * \brief Get the max slice index
   */
  int maxSliceIndex();

  /*!
   * \brief Set the current slice index.
   *
   * The value will be clipped if \a slice is out of range.
   */
  void setSliceIndex(int slice);

  /*!
   * \brief Set the slice index without emitting signal.
   */
  void setSliceIndexQuietly(int slice);

  /*!
   * \brief Increase or descrease of the slice index with a certain step.
   *
   * The slice index is set to the sum of the current slice and \a step, which
   * can be either positive or negative.
   */
  void stepSlice(int step);

  /*!
   * \brief Get the current Z position.
   *
   * \return The current Z position, which is defined as the slice index plus
   * the Z coordinate of the stack offset.
   */
  int getCurrentZ() const;

  //int threshold();

  void setSliceAxis(NeuTube::EAxis axis);
  NeuTube::EAxis getSliceAxis() const { return m_sliceAxis; }

  /*!
   * \brief Get stack data from the buddy document
   */
  ZStack *stackData() const;

  /*!
   * \brief set up the view after the document is ready
   */
  void prepareDocument();

  virtual void resizeEvent(QResizeEvent *event);

  /*!
   * \brief Get the information of the view as a list of strings.
   */
  QStringList toStringList() const;

  //void setImageWidgetCursor(const QCursor &cursor);
  /*!
   * \brief Set the cursor of the image screen
   */
  void setScreenCursor(const QCursor &cursor);

  /*!
   * \brief Set up the scroll strategy.
   *
   * The scroll strategy \a strategy controls the scrolling behavior of the view.
   * The \a strategy pointer is owned by the view.
   */
  void setScrollStrategy(ZScrollSliceStrategy *strategy);

  /*!
   * \brief Intensity threshold for stack data.
   */
  int getIntensityThreshold();

  /*!
   * \brief Save the current scene in an image file.
   */
  void takeScreenshot(const QString &filename);

  void paintStackBuffer();
  void paintMaskBuffer();

  /*!
   * \brief Update the buffer of object canvas.
   */
  void paintObjectBuffer();
  bool paintTileCanvasBuffer();

  //void paintObjectBuffer(ZImage *canvas, ZStackObject::ETarget target);

  void paintObjectBuffer(ZStackObject::ETarget target);
  void paintObjectBuffer(ZPainter &painter, ZStackObject::ETarget target);

  void paintActiveDecorationBuffer();
  void paintDynamicObjectBuffer();

  ZStack* getObjectMask(uint8_t maskValue);

  /*!
   * \brief Get object mask of a certain color
   */
  ZStack* getObjectMask(NeuTube::EColor color, uint8_t maskValue);

  ZStack* getStrokeMask(uint8_t maskValue);
  ZStack* getStrokeMask(NeuTube::EColor color);


  void exportObjectMask(const std::string &filePath);
  void exportObjectMask(NeuTube::EColor color, const std::string &filePath);

  inline void setSizeHintOption(NeuTube::ESizeHintOption option) {
    m_sizeHintOption = option;
  }

  inline void blockRedraw(bool state) {
    m_isRedrawBlocked = state;
  }

public:
  bool isViewPortFronzen() const;

  /*!
   * \brief Check if the view depth is frozen.
   *
   * The depth cannot be changed if it is frozen.
   */
  bool isDepthFronzen() const;

  bool isViewChangeEventBlocked() const;

  /*!
   * \brief Check if the view is scrollable on depth.
   *
   * If the depth is not scrollable, the user will not be able to scroll the
   * slice with mouse wheels.
   */
  bool isDepthScrollable();


  void setViewPortFrozen(bool state);
  void setDepthFrozen(bool state);
  void blockViewChangeEvent(bool state);

  void zoomTo(int x, int y, int z);
  void zoomTo(const ZIntPoint &pt);

  void zoomTo(int x, int y, int z, int w);

  void printViewParam() const;

public: //Message system implementation
  class MessageProcessor : public ZMessageProcessor {
  public:
    void processMessage(ZMessage *message, QWidget *host) const;
  };

  void enableMessageManager();

public slots:
  /*!
   * \brief Update view settings from the stack box.
   *
   * It resets view parameters according the current bounding box of the stack.
   */
  void updateViewBox();

  /*!
   * \brief Redraw the whole scene.
   */
  void redraw(EUpdateOption option = UPDATE_QUEUED);

  /*!
   * \brief Redraw objects.
   *
   * It redraws objects in the object canvases.
   */
  void redrawObject();

  void processStackChange(bool rangeChanged);
  //void updateData(int nslice, int threshold = -1);
  //void updateData();
  //void updateSlice(int nslide);
  //void viewThreshold(int threshold);
  void updateThresholdSlider();
  void updateSlider();
  void updateChannelControl();
  void processDepthSliderValueChange();
  void processDepthSliderValueChange(int sliceIndex);

  void paintStack();
  void paintMask();
  void paintObject();
  void paintObject(QList<ZStackObject *> selected,
                   QList<ZStackObject *> deselected);
  void paintActiveDecoration();
  void paintActiveTile();

  void mouseReleasedInImageWidget(QMouseEvent *event);
  void mousePressedInImageWidget(QMouseEvent *event);
  void mouseMovedInImageWidget(QMouseEvent *event);
  void mouseDoubleClickedInImageWidget(QMouseEvent *event);
  void mouseRolledInImageWidget(QWheelEvent *event);

  bool popLeftMenu(const QPoint &pos);
  bool popRightMenu(const QPoint &pos);

  bool showContextMenu(QMenu *menu, const QPoint &pos);

  QMenu* leftMenu();
  QMenu* rightMenu();

  void setInfo(QString info);
  void autoThreshold();
  void setThreshold(int thre);
  void setZ(int z);

  void displayActiveDecoration(bool display = true);
  void request3DVis();
  void requestQuick3DVis();
  void requestHighresQuick3DVis();
  void requestMerge();
  void requestSetting();
  void closeChildFrame();


  void setView(const ZStackViewParam &param);
  void setViewPort(const QRect &rect);
  void maximizeViewPort();

  void updateZSpinBoxValue();

  void paintObject(ZStackObject::ETarget target);
  void paintObject(const QSet<ZStackObject::ETarget> &targetSet);

  void dump(const QString &msg);

  void hideThresholdControl();

  void setDynamicObjectAlpha(int alpha);
  void resetViewProj();


signals:
//  void currentSliceChanged(int);
  void viewChanged(ZStackViewParam param);
//  void viewPortChanged();
  void messageGenerated(const ZWidgetMessage &message);
  void changingSetting();
  void sliceSliderPressed();
  void sliceSliderReleased();
  void closingChildFrame();

public:
  static QImage::Format stackKindToImageFormat(int kind);
  double getCanvasWidthZoomRatio() const;
  double getCanvasHeightZoomRatio() const;
  double getProjZoomRatio() const;
  void setInfo();
  bool isImageMovable() const;

  int getZ(NeuTube::ECoordinateSystem coordSys) const;
  ZIntPoint getCenter(
      NeuTube::ECoordinateSystem coordSys = NeuTube::COORD_STACK) const;

  QRect getViewPort(NeuTube::ECoordinateSystem coordSys) const;
  ZStackViewParam getViewParameter(
      NeuTube::ECoordinateSystem coordSys = NeuTube::COORD_STACK,
      NeuTube::View::EExploreAction action = NeuTube::View::EXPLORE_UNKNOWN) const;

  QRectF getProjRegion() const;
  ZViewProj getViewProj() const;

  //Get transform from view port to proj region
  ZStTransform getViewTransform() const;

  /*!
   * \brief Set the viewport offset
   *
   * (\a x, \a y) is supposed to the world coordinates of the first corner of
   * the viewport. The real position will be adjusted to fit in the canvas.
   */
  void setViewPortOffset(int x, int y);

  void setViewPortCenter(int x, int y, int z, NeuTube::EAxisSystem system);
  void setViewPortCenter(const ZIntPoint &center, NeuTube::EAxisSystem system);

  void setViewProj(int x0, int y0, double zoom);
  void setViewProj(const QPoint &pt, double zoom);
  void setViewProj(const ZViewProj &vp);

  ZIntPoint getViewCenter() const;

  void paintMultiresImageTest(int resLevel);
  void customizeWidget();

  void addHorizontalWidget(QWidget *widget);
  void addHorizontalWidget(QSpacerItem *spacer);

//  void notifyViewPortChanged();

  bool isViewChanged(const ZStackViewParam &param) const;
  void processViewChange(bool redrawing, bool depthChanged);
  void processViewChange(bool redrawing);
//  void processViewChange(const ZStackViewParam &param);

  void setHoverFocus(bool on);
  void setSmoothDisplay(bool on);

  void notifyViewChanged(const ZStackViewParam &param);
  void notifyViewChanged();


public: //Change view parameters
  void move(const QPoint& src, const QPointF &dst);
  void moveViewPort(int dx, int dy);

  void increaseZoomRatio();
  void decreaseZoomRatio();
  void increaseZoomRatio(int x, int y, bool usingRef = true);
  void decreaseZoomRatio(int x, int y, bool usingRef = true);

//  void zoomWithWidthAligned(int x0, int x1, int cy);
//  void zoomWithWidthAligned(int x0, int x1, double pw, int cy, int cz);
//  void zoomWithHeightAligned(int y0, int y1, double ph, int cx, int cz);
//  void notifyViewChanged(
//      NeuTube::View::EExploreAction action = NeuTube::View::EXPLORE_UNKNOWN);
  void highlightPosition(int x, int y, int z);
  void highlightPosition(const ZIntPoint &pt);

  void updateContrastProtocal();

protected:
  ZIntCuboid getViewBoundBox() const;
  virtual int getDepth() const;

  void clearCanvas();
  template<typename T>
  void resetCanvasWithStack(T &canvas, ZPainter *painter);

  virtual void resetCanvasWithStack(
      ZMultiscalePixmap &canvas, ZPainter *painter);

  void reloadTileCanvas();
  bool reloadObjectCanvas(bool repaint = false);
  void reloadCanvas();

  virtual void updateImageCanvas();
  void updateMaskCanvas();
  void clearObjectCanvas();
  void clearTileCanvas();
  void updateObjectCanvas();
  void updateTileCanvas();
  void updateDynamicObjectCanvas();
  void updateActiveDecorationCanvas();
  void updatePaintBundle();

  ZPixmap* updateProjCanvas(ZPixmap *canvas);
  ZPixmap* updateViewPortCanvas(ZPixmap *canvas);

  void connectSignalSlot();

  QSize getCanvasSize() const;

  //help functions
  virtual void paintSingleChannelStackSlice(ZStack *stack, int slice);
  void paintMultipleChannelStackSlice(ZStack *stack, int slice);
  void paintSingleChannelStackMip(ZStack *stack);
  void paintMultipleChannelStackMip(ZStack *stack);

  QSet<ZStackObject::ETarget> updateViewData(const ZStackViewParam &param);

  void init();

  ZPainter* getPainter(ZStackObject::ETarget target);
  ZPixmap* getCanvas(ZStackObject::ETarget target);
  void setCanvasVisible(ZStackObject::ETarget target, bool visible);
//  void resetDepthControl();

  void setSliceRange(int minSlice, int maxSlice);

  bool event(QEvent *event);

private:
  void updateSliceFromZ(int z);
  void recordViewParam();

protected:
  //ZStackFrame *m_parent;
  ZSlider *m_depthControl;
  //QSpinBox *m_spinBox;
  QLabel *m_infoLabel;
  QLabel *m_msgLabel;
  QLabel *m_activeLabel;
  ZImage *m_image;
//  ZPainter m_imagePainter;
  ZImage *m_imageMask;
//  ZPixmap *m_objectCanvas;
  ZPixmap *m_dynamicObjectCanvas;
  double m_dynamicObjectOpacity;
//  ZPainter m_dynamicObjectCanvasPainter;

  ZMultiscalePixmap m_objectCanvas;
  ZPainter m_objectCanvasPainter;

  NeuTube::EAxis m_sliceAxis;

  ZPainter m_tileCanvasPainter;
  ZPixmap *m_activeDecorationCanvas;
  ZMultiscalePixmap m_tileCanvas;
//  ZPixmap *m_tileCanvas;
  ZImageWidget *m_imageWidget;
  ZLabeledSpinBoxWidget *m_zSpinBox;

  QVBoxLayout *m_layout;
  QHBoxLayout *m_topLayout;
  QHBoxLayout *m_secondTopLayout;
  QHBoxLayout *m_channelControlLayout;
  QHBoxLayout *m_ctrlLayout;
  QHBoxLayout *m_zControlLayout;
  bool m_scrollEnabled;

  QProgressBar *m_progress;
  ZSlider *m_thresholdSlider;
  QPushButton *m_autoThreButton;

  // used to turn on or off each channel
  std::vector<ZBoolParameter*> m_chVisibleState;

  NeuTube::ESizeHintOption m_sizeHintOption;

  ZPaintBundle m_paintBundle;
  bool m_isRedrawBlocked;
//  QMutex m_mutex;

  ZBodySplitButton *m_splitButton;
  ZMessageManager *m_messageManager;

  bool m_depthFrozen;
  bool m_viewPortFrozen;
  bool m_viewChangeEventBlocked;

  ZScrollSliceStrategy *m_sliceStrategy;

  ZStackViewParam m_oldViewParam;
//  ZStackDoc::ActiveViewObjectUpdater m_objectUpdater;
};

#endif
