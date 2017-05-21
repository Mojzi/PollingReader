#include "pollingview.h"

PollingView::PollingView(QWidget *parent) : QGraphicsView(parent)
{

}

void PollingView::wheelEvent(QWheelEvent *event)
{
    /*
     * Zoom in/out with mouse
     */
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    double scaleFactor = 1.15;

    if(event->delta() > 0)
    {
        scale(scaleFactor, scaleFactor);
    }
    else
    {
        scale(1/scaleFactor, 1/scaleFactor);
    }
}
