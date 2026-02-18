#ifndef NESTEDSELECTORWIDGET_H
#define NESTEDSELECTORWIDGET_H

#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "../../NestedArrayInfo.h"

class NestedSelectorWidget : public QWidget {
  Q_OBJECT

public:
  explicit NestedSelectorWidget(QWidget *parent = nullptr);
  void setGroupBoxTitle(const QString &title);
  void setArrayLabel(const QString &label);
  void setComponentLabel(const QString &label);
  void setEnabled(bool enabled);
  void clear();

public slots:
  void setNestedArrays(const QVector<NestedArrayInfo> &nestedArrays);
  void setCurrentArrayIndex(int index);
  void setCurrentComponentIndex(int componentIndex);

signals:
  void selectionChanged(int arrayIndex, int componentIndex);

private slots:
  void onArrayIndexChanged(int index);
  void onComponentIndexChanged(int index);

private:
  void setupUi();
  void setupConnections();

  void updateArrayComboItems();
  void updateComponentComboItems(int arrayIndex);

private:
  QVector<NestedArrayInfo> arrays;

  QGroupBox *groupBox;
  QLabel *arrayLabel;
  QComboBox *arrayCombo;
  QLabel *componentLabel;
  QComboBox *componentCombo;
  QHBoxLayout *componentLayout;

  QString groupBoxTitle;
  QString arrayLabelText;
  QString componentLabelText;
};

#endif // NESTEDSELECTORWIDGET_H
