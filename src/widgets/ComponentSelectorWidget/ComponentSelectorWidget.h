#ifndef COMPONENTSELECTORWIDGET_H
#define COMPONENTSELECTORWIDGET_H

#include <QWidget>
#include <QString>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVector>

// Structure to represent an array and its components
struct ArrayInfo {
    QString name;
    int componentCount;
    QVector<QString> componentNames;
    
    ArrayInfo(const QString& n = QString(), int compCount = 0) 
        : name(n), componentCount(compCount) {
        // Generate default component names
        for (int i = 0; i < componentCount; ++i) {
            if (componentCount == 1) {
                componentNames.append("Value");
            } else if (componentCount == 3) {
                componentNames.append(QString("Component %1").arg(QChar('X' + i)));
            } else {
                componentNames.append(QString("Component %1").arg(i));
            }
        }
    }
};

class ComponentSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ComponentSelectorWidget(QWidget *parent = nullptr);
    
    // Public API - Getters
    bool hasArrays() const { return !m_arrays.isEmpty(); }
    int currentArrayIndex() const { return m_arrayCombo->currentIndex(); }
    int currentComponentIndex() const { 
        int idx = m_componentCombo->currentIndex();
        return (idx >= 0) ? m_componentCombo->itemData(idx).toInt() : -1;
    }
    QString currentArrayName() const;
    QString currentComponentName() const;
    QVector<ArrayInfo> arrays() const { return m_arrays; }
    int arrayCount() const { return m_arrays.size(); }
    int componentCount() const { return m_currentComponentCount; }
    
    // Public API - Setters (with signals)
    void setArrays(const QVector<ArrayInfo>& arrays);
    void addArray(const ArrayInfo& array);
    void addArray(const QString& name, int componentCount);
    void clearArrays();
    void setCurrentArray(int index);
    void setCurrentComponent(int componentIndex);
    void setArrayLabel(const QString& text);
    void setComponentLabel(const QString& text);
    void setEnabled(bool enabled);
    
    // Configuration methods (fluent interface)
    ComponentSelectorWidget* withArrays(const QVector<ArrayInfo>& arrays);
    ComponentSelectorWidget* withArray(const QString& name, int components);
    ComponentSelectorWidget* withLabels(const QString& arrayLabel, const QString& componentLabel);
    ComponentSelectorWidget* withStyle(const QString& style);

public slots:
    void selectArray(int index);
    void selectComponent(int componentIndex);
    void selectMagnitude();  // Special case for magnitude (-1)
    void clear();

signals:
    // Core selection signal
    void selectionChanged(int arrayIndex, int componentIndex);

private slots:
    void onArrayIndexChanged(int index);
    void onComponentIndexChanged(int index);
    void updateComponentCombo(int arrayIndex);
    void updateEnableState();

private:
    void setupUi();
    void setupConnections();

private:
    // UI Elements
    QGroupBox* m_groupBox;
    QLabel* m_arrayLabel;
    QComboBox* m_arrayCombo;
    QLabel* m_componentLabel;
    QComboBox* m_componentCombo;
    QHBoxLayout* m_componentLayout;
    
    // Data
    QVector<ArrayInfo> m_arrays;
    int m_currentComponentCount;
    
    // Configuration
    QString m_arrayLabelText;
    QString m_componentLabelText;
    bool m_autoSelectFirst;
};

#endif // COMPONENTSELECTORWIDGET_H