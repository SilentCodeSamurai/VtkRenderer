#include "FilePickerWidget.h"
#include <QFileDialog>
#include <QFile>

FilePickerWidget::FilePickerWidget(QWidget *parent)
    : QWidget(parent), m_fileFilter("All files (*.*)"), m_placeholderText("No file selected")
{
    setupUi();
    setupConnections();
    updateUi();
}

void FilePickerWidget::setupUi()
{
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    // File label
    m_fileLabel = new QLabel(m_placeholderText, this);
    m_fileLabel->setWordWrap(true);
    m_fileLabel->setStyleSheet(
        "QLabel {"
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
    m_openButton->setStyleSheet(
        "QPushButton {"
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
    m_closeButton->setStyleSheet(
        "QPushButton {"
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

void FilePickerWidget::setupConnections()
{
    connect(m_openButton, &QPushButton::clicked,
            this, &FilePickerWidget::onOpenClicked);
    connect(m_closeButton, &QPushButton::clicked,
            this, &FilePickerWidget::onCloseClicked);
}

void FilePickerWidget::onOpenClicked()
{
    openFile();
}

void FilePickerWidget::onCloseClicked()
{
    closeFile();
}

void FilePickerWidget::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select File",
        QString(),
        m_fileFilter);

    if (!filePath.isEmpty())
    {
        setFile(filePath);
    }
}

void FilePickerWidget::closeFile()
{
    if (!m_currentFile.isEmpty())
    {
        clear();
        emit fileClosed();
    }
}

bool FilePickerWidget::setFile(const QString &filePath)
{
    // Validate file exists
    if (!QFile::exists(filePath))
    {
        emit error(QString("File not found: %1").arg(filePath));
        return false;
    }

    QFileInfo info(filePath);

    // Update state
    m_currentFile = filePath;

    // Update UI
    m_fileLabel->setText("📄 " + info.fileName());
    m_fileLabel->setToolTip(filePath);
    m_fileLabel->setStyleSheet(
        "QLabel {"
        "   color: #e6f8ff;"
        "   background-color: #112330;"
        "   border: 1px solid #00bcd4;"
        "   border-radius: 0px;"
        "   padding: 8px;"
        "   font-family: monospace;"
        "   font-weight: 600;"
        "}");

    m_closeButton->setEnabled(true);

    // Emit signals
    emit fileOpened(filePath);
    return true;
}

void FilePickerWidget::clear()
{
    m_currentFile.clear();
    updateUi();
}

void FilePickerWidget::updateUi()
{
    if (m_currentFile.isEmpty())
    {
        m_fileLabel->setText(m_placeholderText);
        m_fileLabel->setToolTip(QString());
        m_fileLabel->setStyleSheet(
            "QLabel {"
            "   color: #a7b4c2;"
            "   background-color: #151d26;"
            "   border: 1px solid #2a3a4b;"
            "   border-radius: 0px;"
            "   padding: 8px;"
            "   font-family: monospace;"
            "}");
        m_closeButton->setEnabled(false);
    }
    else
    {
        QFileInfo info(m_currentFile);
        m_fileLabel->setText("📄 " + info.fileName());
        m_fileLabel->setToolTip(m_currentFile);
        m_closeButton->setEnabled(true);
    }
}

void FilePickerWidget::setFileFilter(const QString &filter)
{
    if (m_fileFilter != filter)
    {
        m_fileFilter = filter;
    }
}

void FilePickerWidget::setPlaceholderText(const QString &text)
{
    m_placeholderText = text;
    if (m_currentFile.isEmpty())
    {
        m_fileLabel->setText(text);
    }
}

void FilePickerWidget::setLabelStyle(const QString &style)
{
    m_fileLabel->setStyleSheet(style);
}

void FilePickerWidget::setButtonStyle(const QString &openStyle, const QString &closeStyle)
{
    if (!openStyle.isEmpty())
    {
        m_openButton->setStyleSheet(openStyle);
    }
    if (!closeStyle.isEmpty())
    {
        m_closeButton->setStyleSheet(closeStyle);
    }
}