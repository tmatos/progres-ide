#ifndef SIGNALVIEW_H
#define SIGNALVIEW_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QRect>

class SignalView : public QWidget
{
    Q_OBJECT
public:
    explicit SignalView(QWidget *parent = nullptr);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QRect r = event->rect();

        QPainter p(this);

        p.drawLine(10,10,100,10);

        return;
    }

    void mousePressEvent(QMouseEvent *event) override
    {

        if(event->button() == Qt::LeftButton) {
            QPoint p = event->pos();
        }

        return;
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        return;
    }
};

#endif // SIGNALVIEW_H
