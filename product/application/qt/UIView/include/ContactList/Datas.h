//datas.h 头文件
#ifndef DATAS_H
#define DATAS_H

#include <QObject>
#include <QtQml>

#include <QString>
#include <iostream>
class Datas : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString m_age READ getAge WRITE setAge NOTIFY ageChanged)
    QML_ELEMENT
public:
    explicit Datas(QObject *parent = nullptr);

    QString getAge(){
        return m_age;
    }

    void setAge(const QString &age)
    {
        m_age = age;
        emit ageChanged();
    }
Q_INVOKABLE QString getData(){return "123";}
signals:
    void ageChanged();

private:
    QString m_age;
};

#endif // DATAS_H