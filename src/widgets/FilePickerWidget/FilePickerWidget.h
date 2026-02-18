#ifndef FILEPICKERWIDGET_H
#define FILEPICKERWIDGET_H

#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

class FilePickerWidget : public QWidget {
  Q_OBJECT

public:
  explicit FilePickerWidget(QWidget *parent = nullptr);

  // Public API - Getters
  QString fileFilter() const { return m_fileFilter; }

  // Public API - Setters (with signals)
  void setFileFilter(const QString &filter);
  void setPlaceholderText(const QString &text);
  void setLabelStyle(const QString &style);
  void setButtonStyle(const QString &openStyle, const QString &closeStyle);

  // Public slots - can be connected to external signals
public slots:
  void setSelectedFile(const QString &filePath);
  void clear();

signals:
  void fileSelectionChanged(const QString &filePath);
  void fileSelectionCleared();
  void fileSelectionErrorOccurred(const QString &message);

private slots:
  void onOpenClicked();
  void onCloseClicked();

private:
  void setupUi();
  void setupConnections();

private:
  // UI Elements
  QLabel *m_fileLabel;
  QPushButton *m_openButton;
  QPushButton *m_closeButton;

  // State
  QString m_fileFilter;
  QString m_placeholderText;
};

#endif // FILEPICKERWIDGET_H