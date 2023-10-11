#ifndef FLUXHUB_H
#define FLUXHUB_H

#include <QColor>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQuickItem>

class TreeNode
{
public:
    TreeNode(QString name = "");
    ;
    TreeNode(QJsonValue value);
    const QString &getNodeName() const
    {
        return m_node_name;
    }
    const QList<TreeNode> &getChildren() const
    {
        return m_children_node;
    }
    bool isEmpty();
    bool addNode(const QString &parent_name, const QString &input_name);
    void removeNode(const QString &input_name);
    static QString getNodeName(const QJsonValue &value);
    static QJsonObject itemToJson(TreeNode node);

private:
    void init(QJsonValue &value);
    QString m_node_name{""};
    QList<TreeNode> m_children_node;
};
class FluxHub : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(QJsonObject demoJson READ demoJson NOTIFY demoJSONChanged)
    Q_PROPERTY(QString select_name READ selectName WRITE setSelectName NOTIFY selectNameChanged)
    explicit FluxHub(QQmlEngine *engine, QObject *parent = nullptr);

    QJsonObject demoJson();

    Q_INVOKABLE QQuickItem *visualize(QQuickItem *rootItem);
    Q_INVOKABLE void addNode();
    Q_INVOKABLE void removeNode();
    Q_INVOKABLE QString selectName() const;
    Q_INVOKABLE void testCode(QJSValue callback);
    void setSelectName(const QString &name);
signals:
    void selectNameChanged();
    void demoJSONChanged();
public slots:
    void destroyNode(QQuickItem *rootNode)
    {
        delete rootNode;
    }

private:
    QQuickItem *createObjectNode(QQuickItem *parent_item, const QJsonObject &jsonObj, int level);
    QQuickItem *createLeafNode(QQuickItem *current_item, const QJsonValue &jsonVal, int level);
    void createArrayNode(QQuickItem *current_item, const QJsonArray &json_array, int level);
    QString getValidNodeName(TreeNode &node, const QString &input_name);
    bool checkNodeNameValid(const TreeNode &node, const QString &input_name);
    QQuickItem *createLevelItem(int level, QQuickItem *parentItem);

    void setNodeText(QQuickItem *item, const QJsonValue &text);
    inline QString levelColor(int level)
    {
        return level >= m_levelColors.size() ? m_levelColors.back() : m_levelColors[level];
    }

    static QString getNewSuffixName(const QString &name);

private:
    QQmlEngine *m_engine = nullptr;
    QString m_select_module_name{""};
    TreeNode m_root_node;
    QList<QQmlComponent *> m_levelComponents;
    QList<QString> m_levelColors;
};

#endif  // FLUXHUB_H
