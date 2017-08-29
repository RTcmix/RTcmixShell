#ifndef LED_H
#define LED_H

#include <QWidget>
#include <QPainter>
#include <QSize>
#include <QtDebug>

// Adapted from VRonin answer: https://forum.qt.io/topic/73724/led-like-buttons-widgets/2
// NB: this is hacked to work for our specific context -- placement within a toolbar.
class Led : public QWidget {
    Q_OBJECT
    Q_PROPERTY(bool power READ power WRITE setPower NOTIFY powerChanged)
    Led(const Led&) = delete;
    Led& operator = (const Led&) = delete;

public:
    explicit Led(QWidget* parent = nullptr)
        : QWidget(parent)
        , m_power(false)
    {
    }

    QSize sizeHint() const override {
        return QSize(24, 16);
    }

    bool power() const
    {
        return m_power;
    }

public slots:
    void setPower(bool power)
    {
        if (power != m_power){
            m_power = power;
            emit powerChanged();
            update();
        }
    }

signals:
    void powerChanged();

protected:
    virtual void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        QPainter ledPainter(this);
        QPen pen;
        pen.setColor(Qt::darkGray);
        pen.setWidth(1);
        ledPainter.setPen(pen);
        if (m_power)
            ledPainter.setBrush(Qt::red);
        else
            ledPainter.setBrush(Qt::NoBrush);
        // qDebug() << "paintEvent: size:" << size() << "rect:" << rect();
        QRect r = QRect(rect());
        r.setSize(QSize(13, 13));
        //r.translate(0, -1);   // causes top edge to disappear
        ledPainter.drawRect(r);
    }

private:
    bool m_power;
};

#endif // LED_H
