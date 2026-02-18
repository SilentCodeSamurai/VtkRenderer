#include "FilePickerWidget.h"
#include <QFile>
#include <QFileDialog>
#include <QPointer>
#include <QTimer>

FilePickerWidget::FilePickerWidget(QWidget *parent)
    : QWidget(parent), m_fileFilter("All files (*.*)"),
      m_placeholderText("No file selected") {
  setupUi();
  setupConnections();
  clear();
}

void FilePickerWidget::setupUi() {
  // Main layout
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(4);

  // File label
  m_fileLabel = new QLabel(m_placeholderText, this);
  m_fileLabel->setWordWrap(true);
  m_fileLabel->setStyleSheet("QLabel {"
                             "   color: #a7b4c2;"
                             "   background-color: #151d26;"
                             "   border: 1px solid #2a3a4b;"
                             "   border-radius: 0px;"
                             "   padding: 8px;"
                             "   font-family: monospace;"
                             "}");

  // Buttons layout
  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->setSpacing(8);

  m_openButton = new QPushButton("📂 Open File", this);
  m_openButton->setStyleSheet("QPushButton {"
                              "   background-color: #121820;"
                              "   color: #d9e7f5;"
                              "   border: 2px solid #00bcd4;"
                              "   border-radius: 0px;"
                              "   padding: 8px 16px;"
                              "   font-weight: 600;"
                              "}"
                              "QPushButton:hover {"
                              "   background-color: #0f2630;"
                              "   color: #64e8ff;"
                              "}"
                              "QPushButton:pressed {"
                              "   background-color: #093946;"
                              "}");

  m_closeButton = new QPushButton("✖ Close", this);
  m_closeButton->setStyleSheet("QPushButton {"
                               "   background-color: #121820;"
                               "   color: #d9e7f5;"
                               "   border: 2px solid #ff5a36;"
                               "   border-radius: 0px;"
                               "   padding: 8px 16px;"
                               "   font-weight: 600;"
                               "}"
                               "QPushButton:hover {"
                               "   background-color: #2a1713;"
                               "   color: #ff9c87;"
                               "}"
                               "QPushButton:pressed {"
                               "   background-color: #3f1510;"
                               "}"
                               "QPushButton:disabled {"
                               "   background-color: #171d24;"
                               "   border-color: #35414e;"
                               "   color: #607182;"
                               "}");

  buttonLayout->addWidget(m_openButton);
  buttonLayout->addWidget(m_closeButton);
  buttonLayout->addStretch();

  mainLayout->addWidget(m_fileLabel);
  mainLayout->addLayout(buttonLayout);

  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void FilePickerWidget::setupConnections() {
  connect(m_openButton, &QPushButton::clicked, this,
          &FilePickerWidget::onOpenClicked);
  connect(m_closeButton, &QPushButton::clicked, this,
          &FilePickerWidget::onCloseClicked);
}

void FilePickerWidget::onOpenClicked() {
  QString filePath = QFileDialog::getOpenFileName(this, "Select File",
                                                  QString(), m_fileFilter);
  // User cancelled the dialog - this is normal, just return silently
  if (filePath.isEmpty()) {
    return;
  }
  QFile file(filePath);
  if (!file.exists()) {
    emit fileSelectionErrorOccurred(
        QString("File not found: %1").arg(filePath));
    return;
  }
  // Defer signal emission to ensure file dialog is fully closed
  // This prevents crashes when VTK operations start while dialog is closing
  // Use QPointer to safely check if widget still exists when lambda executes
  QPointer<FilePickerWidget> self = this;
  QTimer::singleShot(0, this, [self, filePath]() {
    if (self == nullptr) {
      return;
    }
    emit self->fileSelectionChanged(filePath);
  });
}

void FilePickerWidget::onCloseClicked() { emit fileSelectionCleared(); }

void FilePickerWidget::setSelectedFile(const QString &filePath) {
  QFileInfo info(filePath);
  m_fileLabel->setText("📄 " + info.fileName());
  m_fileLabel->setToolTip(filePath);
  m_closeButton->setEnabled(true);
}

void FilePickerWidget::clear() {
  m_fileLabel->setText(m_placeholderText);
  m_fileLabel->setToolTip(QString());
  m_fileLabel->setStyleSheet("QLabel {"
                             "   color: #a7b4c2;"
                             "   background-color: #151d26;"
                             "   border: 1px solid #2a3a4b;"
                             "   border-radius: 0px;"
                             "   padding: 8px;"
                             "   font-family: monospace;"
                             "}");
  m_closeButton->setEnabled(false);
}

void FilePickerWidget::setFileFilter(const QString &filter) {
  m_fileFilter = filter;
}

void FilePickerWidget::setPlaceholderText(const QString &text) {
  m_placeholderText = text;
  m_fileLabel->setText(text);
}

void FilePickerWidget::setLabelStyle(const QString &style) {
  m_fileLabel->setStyleSheet(style);
}

void FilePickerWidget::setButtonStyle(const QString &openStyle,
                                      const QString &closeStyle) {
  if (!openStyle.isEmpty()) {
    m_openButton->setStyleSheet(openStyle);
  }
  if (!closeStyle.isEmpty()) {
    m_closeButton->setStyleSheet(closeStyle);
  }
}