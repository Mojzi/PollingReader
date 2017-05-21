#ifndef POLLINGVIEW_H
#define POLLINGVIEW_H

#include <QGraphicsView>
#include <QWidget>
#include <QWheelEvent>

class PollingView : public QGraphicsView
{
public:
    PollingView(QWidget *parent = 0);
protected:
    virtual void wheelEvent(QWheelEvent *event);
};

#endif // POLLINGVIEW_H
