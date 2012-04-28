#ifndef GUI_UTILS_H
#define GUI_UTILS_H

#include <climits>
#include <QString>
#include <QTextStream>

template<typename T> QString hexstr(T value)
{
    QString str;
    QTextStream stream(&str);
    stream << hex << qSetFieldWidth(sizeof(T)*CHAR_BIT/4) << qSetPadChar('0') << value;
    return "0x" + str.toUpper();
}

#endif // GUI_UTILS_H
