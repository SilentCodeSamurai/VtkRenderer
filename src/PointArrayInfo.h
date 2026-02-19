#ifndef POINT_ARRAY_INFO_H
#define POINT_ARRAY_INFO_H

#include <QString>
#include <QVector>

struct PointArrayInfo {
  QString name;
  QVector<QString> componentNames;

  PointArrayInfo(const QString &name, const QVector<QString> &componentNames)
      : name(name), componentNames(componentNames) {}
};

#endif // POINT_ARRAY_INFO_H