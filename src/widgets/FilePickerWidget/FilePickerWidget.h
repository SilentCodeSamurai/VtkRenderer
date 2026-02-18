#ifndef FILEPICKERWIDGET_H
#define FILEPICKERWIDGET_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>

class FilePickerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FilePickerWidget(QWidget *parent = nullptr);

    // Public API - Getters
    QString currentFile() const { return m_currentFile; }
    bool hasFile() const { return !m_currentFile.isEmpty(); }
    QString fileFilter() const { return m_fileFilter; }
    QFileInfo fileInfo() const { return QFileInfo(m_currentFile); }
    QString fileName() const { return QFileInfo(m_currentFile).fileName(); }
    QString filePath() const { return m_currentFile; }

    // Public API - Setters (with signals)
    void setFileFilter(const QString &filter);
    void setPlaceholderText(const QString &text);
    void setLabelStyle(const QString &style);
    void setButtonStyle(const QString &openStyle, const QString &closeStyle);

    // Public slots - can be connected to external signals
public slots:
    void openFile();
    void closeFile();
    bool setFile(const QString &filePath);
    void clear();

signals:
    // Core signals
    void fileOpened(const QString &filePath);
    void fileClosed();

    // Error signals
    void error(const QString &message);

private slots:
    void onOpenClicked();
    void onCloseClicked();
    void updateUi();

private:
    void setupUi();
    void setupConnections();

private:
    // UI Elements
    QLabel *m_fileLabel;
    QPushButton *m_openButton;
    QPushButton *m_closeButton;

    // State
    QString m_currentFile;
    QString m_fileFilter;
    QString m_placeholderText;
};

#endif // FILEPICKERWIDGET_H