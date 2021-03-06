#pragma once

#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QOpenGLWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

// This include is autogenerated by the .ui file in the same folder
// The generated version will be names 'ui_<filename>.h'
#include "software/ai/hl/stp/play_info.h"
#include "software/backend/robot_status.h"
#include "software/geom/rectangle.h"
#include "software/visualizer/drawing/draw_functions.h"
#include "software/visualizer/ui/ui_main_widget.h"
#include "software/visualizer/widgets/ai_control.h"
#include "software/visualizer/widgets/parameters.h"
#include "software/visualizer/widgets/robot_status.h"
#include "software/visualizer/widgets/world_view.h"

// Forward declare the name of the top-level GUI class defined in main_widget.ui
namespace Ui
{
    class AutoGeneratedMainWidget;
}

/**
 * This class acts as a wrapper widget for all the autogenerated components
 * defined in main_widget.ui
 */
class MainWidget : public QWidget
{
    Q_OBJECT

   public:
    explicit MainWidget(QWidget* parent = nullptr);

   public slots:

    /**
     * Draws all the AI information we want to display in the Visualizer. This includes
     * visualizing the state of the world as well as drawing the AI state we want to show,
     * like planned navigator paths.
     *
     * @param world_draw_function The function that tells the Visualizer how to draw the
     * World state
     * @param ai_draw_function The function that tells the Visualizer how to draw the AI
     * state
     */
    void draw(WorldDrawFunction world_draw_function, AIDrawFunction ai_draw_function);

    /**
     * Sets the area of the World being drawn in the Visualizer to the given region
     *
     * @param new_view_area The new area to show in the view
     */
    void setDrawViewArea(const QRectF& new_view_area);

    /**
     * Updates and displays newly provided PlayInfo
     *
     * @param play_info The new PlayInfo to display
     */
    void updatePlayInfo(const PlayInfo& play_info);

    /**
     * Updates and displays the newly provided RobotStatus.
     *
     * @param robot_status The new robot status to display
     */
    void updateRobotStatus(const RobotStatus& robot_status);

   private:
    // The "parent" of each of these widgets is set during construction, meaning that
    // the Qt system takes ownership of the pointer and is responsible for de-allocating
    // it, so we don't have to
    Ui::AutoGeneratedMainWidget* main_widget;
    QGraphicsScene* scene;
    QOpenGLWidget* glWidget;
};
