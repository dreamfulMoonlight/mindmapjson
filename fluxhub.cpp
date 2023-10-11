#include "fluxhub.h"
#include <QFile>
#include <QQmlProperty>
#include <QtDebug>

void saveJsonFile(QJsonObject obj)
{
    // 将 QJsonObject 转换为 QJsonDocument
    QJsonDocument jsonDocument(obj);

    // 将 QJsonDocument 写入 JSON 文件
    QFile jsonFile("./data.json");
    if (jsonFile.open(QIODevice::WriteOnly))
    {
        jsonFile.write(jsonDocument.toJson());  // 将 JSON 数据写入文件
        jsonFile.close();
        qDebug() << "JSON file written successfully.";
    }
    else
    {
        qWarning() << "Failed to open JSON file for writing.";
    }
}

FluxHub::FluxHub(QQmlEngine *engine, QObject *parent) : QObject(parent), m_engine(engine) {}

QJsonObject FluxHub::demoJson()
{
    if (!m_root_node.isEmpty())
    {
        return TreeNode::itemToJson(m_root_node);
    }
    else
    {
        QFile inf(":/demo.json");
        if (!inf.open(QIODevice::ReadOnly))
        {
            qWarning() << "Cannot open demo json file!";
            return QJsonObject();
        }
        auto content = inf.readAll();
        inf.close();
        // 将 JSON 数据转换为 QJsonDocument
        QJsonDocument json_doc = QJsonDocument::fromJson(content);

        // 将 QJsonDocument 转换为 QJsonObject
        if (json_doc.isObject())
        {
            QJsonObject json_object = json_doc.object();
            m_root_node = TreeNode(json_object);
            return json_object;
        }
    }
    return QJsonObject();
}

QQuickItem *FluxHub::visualize(QQuickItem *rootItem)
{
    auto component = new QQmlComponent(m_engine, QUrl::fromLocalFile(":/Node.qml"));
    if (component->status() == QQmlComponent::Error)
    {
        qWarning() << "Component create error!" << component->errorString();
        return nullptr;
    }
    m_levelComponents.push_back(component);
    //    m_levelColors.push_back("#1f4e5a");
    //    m_levelColors.push_back("#009c8e");
    //    m_levelColors.push_back("#ffdb6a");
    //    m_levelColors.push_back("#ffa658");
    //    m_levelColors.push_back("#ea5f40");
    m_levelColors.push_back("#205374");
    m_levelColors.push_back("#27a09e");
    m_levelColors.push_back("#30ce88");
    m_levelColors.push_back("#7de393");
    m_levelColors.push_back("#d3f5ce");

    auto json_obj = TreeNode::itemToJson(m_root_node);
    return createObjectNode(rootItem, json_obj, 0);
}

void FluxHub::addNode()
{
    const QString &input_parent_name = m_select_module_name;
    auto new_name = getValidNodeName(m_root_node, "segment");
    m_root_node.addNode(input_parent_name, new_name);
    auto json_obj = TreeNode::itemToJson(m_root_node);
}

void FluxHub::removeNode()
{
    const QString &input_parent_name = m_select_module_name;
    m_root_node.removeNode(input_parent_name);
}

QString FluxHub::selectName() const
{
    return m_select_module_name;
}

void FluxHub::testCode(QJSValue callback)
{
    QJSEngine *engine = qjsEngine(this);
    callback.call(QJSValueList{engine->toScriptValue(5)});
}

void FluxHub::setSelectName(const QString &name)
{
    m_select_module_name = name;
    emit selectNameChanged();
}

QQuickItem *FluxHub::createLevelItem(int level, QQuickItem *parentItem)
{
    auto *component = level >= m_levelComponents.count() ? m_levelComponents.back() : m_levelComponents[level];
    auto item = qobject_cast<QQuickItem *>(component->create());

    if (item == nullptr)
    {
        qWarning() << "Create level item error!" << component->errorString();
        return nullptr;
    }
    QQmlProperty layoutProp(parentItem, "childrenLayout");
    if (layoutProp.type() == QQmlProperty::Invalid)
    {
        item->setParentItem(parentItem);
    }
    else
    {
        auto layout = layoutProp.read().value<QQuickItem *>();
        Q_ASSERT(layout);
        item->setParentItem(layout);
    }

    QQmlProperty colorProp(item, "color");
    colorProp.write(levelColor(level));
    return item;
}

void FluxHub::setNodeText(QQuickItem *item, const QJsonValue &text)
{
    QQmlProperty nodeTextProp(item, "text");
    auto leaf_type = text.type();
    switch (leaf_type)
    {
        case QJsonValue::Bool:
            nodeTextProp.write(QString(text.toBool()));
            break;
        case QJsonValue::Double:
            nodeTextProp.write(QString::number(text.toDouble()));
            break;
        default:
            nodeTextProp.write(text);
            break;
    }
}

QString FluxHub::getNewSuffixName(const QString &name)
{
    auto suffix_index = name.lastIndexOf('_');
    if (suffix_index > 0)
    {
        auto index = name.right(name.length() - suffix_index - 1).toInt();
        return name.left(suffix_index) + "_" + QString::number(index + 1);
    }
    else
    {
        return name + "_0";
    }
}

QQuickItem *FluxHub::createObjectNode(QQuickItem *parent_item, const QJsonObject &json_obj, int level)
{
    auto object_node = createLevelItem(level, parent_item);
    setNodeText(object_node, json_obj["name"]);

    auto children_node_json = json_obj["children"].toArray();
    for (int i = 0; i < children_node_json.size(); i++)
    {
        auto child = children_node_json[i];
        auto child_type = children_node_json[i].type();

        QQuickItem *rt = nullptr;
        if (child_type == QJsonValue::Object)
        {
            createObjectNode(object_node, child.toObject(), level + 1);
        }
        else if (child_type == QJsonValue::Array)
        {
            createArrayNode(object_node, child.toArray(), level + 1);
        }
        else
        {
            rt = createLeafNode(object_node, child, level + 1);
        }
    }
    return object_node;
}

void FluxHub::createArrayNode(QQuickItem *parent_item, const QJsonArray &json_array, int level)
{
    for (int i = 0; i != json_array.size(); ++i)
    {
        auto element = json_array[i];
        auto type = element.type();
        QQuickItem *rt = nullptr;
        if (type == QJsonValue::Object)
        {
            createObjectNode(parent_item, element.toObject(), level + 1);
        }
        else
        {
            createLeafNode(parent_item, element, level + 1);
        }
        if (rt == nullptr)
        {
            qWarning() << "Create array element error!"
                       << "" << type;
            return;
        }
    }
}

QString FluxHub::getValidNodeName(TreeNode &node, const QString &input_name)
{
    if (!checkNodeNameValid(m_root_node, input_name))
    {
        auto new_input_name = FluxHub::getNewSuffixName(input_name);
        return getValidNodeName(m_root_node, new_input_name);
    }
    else
    {
        return input_name;
    }
}

bool FluxHub::checkNodeNameValid(const TreeNode &node, const QString &input_name)
{
    if (node.getNodeName() == input_name)
    {
        return false;
    }
    else
    {
        const auto &children = node.getChildren();
        for (int i = 0; i < children.size(); i++)
        {
            if (!checkNodeNameValid(children[i], input_name))
            {
                return false;
            }
        }
    }
    return true;
}

QQuickItem *FluxHub::createLeafNode(QQuickItem *current_item, const QJsonValue &jsonVal, int level)
{
    QQuickItem *leafNode = nullptr;
    leafNode = createLevelItem(level, current_item);
    setNodeText(leafNode, jsonVal);
    return leafNode;
}

TreeNode::TreeNode(QString name) : m_node_name(name) {}

TreeNode::TreeNode(QJsonValue value)
{
    init(value);
}

bool TreeNode::isEmpty()
{
    if (m_node_name == "" && m_children_node.size() == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool TreeNode::addNode(const QString &parent_name, const QString &input_name)
{
    if (m_node_name == parent_name)
    {
        m_children_node.append(TreeNode(input_name));
        return true;
    }

    for (auto iter = m_children_node.begin(); iter != m_children_node.end(); iter++)
    {
        if (iter->addNode(parent_name, input_name))
        {
            return true;
        }
    }
    return false;
}

void TreeNode::removeNode(const QString &input_name)
{
    for (auto iter = m_children_node.begin(); iter != m_children_node.end();)
    {
        if (iter->getNodeName() == input_name && iter->getChildren().size() == 0)
        {
            iter = m_children_node.erase(iter);
        }
        else
        {
            iter->removeNode(input_name);
            iter++;
        }
    }
}

QString TreeNode::getNodeName(const QJsonValue &value)
{
    auto leaf_type = value.type();
    switch (leaf_type)
    {
        case QJsonValue::Bool:
            return QString(value.toBool());
        case QJsonValue::Double:
            return QString::number(value.toDouble());
        default:
            return value.toString();
    }
}

QJsonObject TreeNode::itemToJson(TreeNode node)
{
    QJsonObject ret;
    ret.insert("name", node.getNodeName());
    QJsonArray children_json;
    const auto &children = node.getChildren();
    for (int i = 0; i < children.size(); i++)
    {
        children_json.append(itemToJson(children[i]));
    }
    ret.insert("children", children_json);
    return ret;
}

void TreeNode::init(QJsonValue &value)
{
    auto node_type = value.type();
    switch (node_type)
    {
        case QJsonValue::Object:
        {
            m_node_name = TreeNode::getNodeName(value["name"]);
            auto children_json = value["children"].toArray();
            for (int i = 0; i < children_json.size(); i++)
            {
                auto child = children_json[i];
                m_children_node.append(TreeNode(child));
            }
            break;
        }
        default:
            m_node_name = TreeNode::getNodeName(value);
    }
}
