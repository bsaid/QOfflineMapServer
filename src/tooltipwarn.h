/**********************************************
**    This file is inspired by Lorris
**    http://tasssadar.github.com/Lorris/
***********************************************/

#ifndef TOOLTIPWARN_H
#define TOOLTIPWARN_H

#include <QFrame>

class QPushButton;
class QLabel;

class ToolTipWarn : public QFrame
{
    Q_OBJECT
public:
    ToolTipWarn(const QString& text, QWidget *posTo = NULL, QWidget *parent = NULL, int delay = 3000, QString icon = QString());

    void setButton(QPushButton *btn);
    void showSpinner();
    void toRightBottom();

private:
    QLabel *m_lorrLabel;
};

#endif // TOOLTIPWARN_H
