#ifndef ZACTIONFACTORY_H
#define ZACTIONFACTORY_H

#include <QAction>

class ZStackDoc;
class QWidget;
class ZActionActivator;
class ZStackPresenter;

class ZActionFactory
{
public:
  ZActionFactory();
  virtual ~ZActionFactory() {}

  enum EAction {
    ACTION_EXTEND_SWC_NODE, ACTION_SMART_EXTEND_SWC_NODE,
    ACTION_CONNECT_TO_SWC_NODE, ACTION_ADD_SWC_NODE,
    ACTION_TOGGLE_SWC_SKELETON,
    ACTION_LOCK_SWC_NODE_FOCUS, ACTION_CHANGE_SWC_NODE_FOCUS,
    ACTION_MOVE_SWC_NODE,
    ACTION_ESTIMATE_SWC_NODE_RADIUS,
    ACTION_PAINT_STROKE, ACTION_ERASE_STROKE,
    ACTION_LOCATE_SELECTED_SWC_NODES_IN_3D,
    ACTION_SPLIT_DATA, ACTION_SHOW_BODY_IN_3D,
    ACTION_BODY_SPLIT_START, ACTION_ADD_SPLIT_SEED,
    ACTION_BODY_MERGE, ACTION_BODY_UNMERGE,
    ACTION_BODY_ANNOTATION, ACTION_BODY_CHECKIN, ACTION_BODY_CHECKOUT,
    ACTION_BODY_FORCE_CHECKIN, ACTION_BODY_DECOMPOSE,
    ACTION_BODY_CROP, /*ACTION_BODY_CHOP_Z,*/
    ACTION_BODY_CHOP,
    ACTION_BOOKMARK_CHECK, ACTION_BOOKMARK_UNCHECK,
    ACTION_MEASURE_SWC_NODE_LENGTH, ACTION_MEASURE_SCALED_SWC_NODE_LENGTH,
    ACTION_SWC_SUMMARIZE,
    ACTION_CHNAGE_SWC_NODE_SIZE, ACTION_TRANSLATE_SWC_NODE,
    ACTION_SET_SWC_ROOT, ACTION_INSERT_SWC_NODE,
    ACTION_RESET_BRANCH_POINT, ACTION_SET_BRANCH_POINT,
    ACTION_CONNECTED_ISOLATED_SWC,
    ACTION_DELETE_SWC_NODE, ACTION_CONNECT_SWC_NODE,
    ACTION_DELETE_UNSELECTED_SWC_NODE,
    ACTION_MERGE_SWC_NODE, ACTION_BREAK_SWC_NODE,
    ACTION_SELECT_DOWNSTREAM, ACTION_SELECT_UPSTREAM,
    ACTION_SELECT_NEIGHBOR_SWC_NODE,
    ACTION_SELECT_SWC_BRANCH, ACTION_SELECT_CONNECTED_SWC_NODE,
    ACTION_SELECT_ALL_SWC_NODE,
    ACTION_CHANGE_SWC_TYPE, ACTION_CHANGE_SWC_SIZE, ACTION_REMOVE_TURN,
    ACTION_RESOLVE_CROSSOVER, ACTION_SWC_Z_INTERPOLATION,
    ACTION_SWC_RADIUS_INTERPOLATION, ACTION_SWC_POSITION_INTERPOLATION,
    ACTION_SWC_INTERPOLATION,
    ACTION_SYNAPSE_ADD_PRE, ACTION_SYNAPSE_ADD_POST, ACTION_SYNAPSE_MOVE,
    ACTION_SYNAPSE_DELETE, ACTION_SYNAPSE_LINK, ACTION_SYNAPSE_UNLINK,
    ACTION_SYNAPSE_VERIFY, ACTION_SYNAPSE_UNVERIFY,
    ACTION_SYNAPSE_FILTER, ACTION_SYNAPSE_HLPSD,
    ACTION_SYNAPSE_REPAIR,
    ACTION_TRACE, ACTION_FITSEG, ACTION_DROPSEG, ACTION_FIT_ELLIPSE,
    ACTION_PUNCTA_MARK, ACTION_PUNCTA_ENLARGE, ACTION_PUNCTA_NARROW,
    ACTION_PUNCTA_MEANSHIFT, ACTION_PUNCTA_MEANSHIFT_ALL,
    ACTION_DELETE_SELECTED,
    ACTION_UNDO, ACTION_REDO,
    ACTION_SHOW_ORTHO, ACTION_ADD_TODO_ITEM, ACTION_ADD_TODO_MERGE,
    ACTION_ADD_TODO_SPLIT,
    ACTION_CHECK_TODO_ITEM,
    ACTION_ADD_TODO_ITEM_CHECKED, ACTION_TODO_ITEM_ANNOT_NORMAL,
    ACTION_TODO_ITEM_ANNOT_MERGE, ACTION_TODO_ITEM_ANNOT_SPLIT,
    ACTION_UNCHECK_TODO_ITEM, ACTION_REMOVE_TODO_ITEM,
    ACTION_ENTER_RECT_ROI_MODE, ACTION_CANCEL_RECT_ROI,
    ACTION_SELECT_BODY_IN_RECT, ACTION_ZOOM_TO_RECT,
    ACTION_REWRITE_SEGMENTATION, ACTION_FLYEM_UPDATE_BODY,
    ACTION_SAVE_STACK,
    ACTION_SEPARATOR
  };

  static QAction* makeAction(
      EAction item, const ZStackDoc *doc, QWidget *parent,
      ZActionActivator *activator = NULL, bool positive = true);
/*
  static QAction* makeAction(
      EAction item, const ZStackPresenter *presenter, QWidget *parent,
      ZActionActivator *activator = NULL, bool positive = true);
*/
  static QAction *MakeAction(EAction actionKey, QObject *parent);

  virtual QAction* makeAction(EAction actionKey, QObject *parent) const;
};

#endif // ZACTIONFACTORY_H
