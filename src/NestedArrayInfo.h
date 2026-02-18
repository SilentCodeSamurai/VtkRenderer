#ifndef NESTED_ARRAY_INFO_H
#define NESTED_ARRAY_INFO_H

#include <QString>
#include <QVector>

struct NestedArrayInfo {
  QString name;
  QVector<QString> componentNames;

  NestedArrayInfo(const QString &name, const QVector<QString> &componentNames)
      : name(name), componentNames(componentNames) {}
};

#endif // NESTED_ARRAY_INFO_H