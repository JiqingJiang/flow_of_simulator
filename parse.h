#include <ctype.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <unordered_map>
using namespace std;

// 常量声明
const double K = 1.38E-23;
const double Q = 1.60E-19;
const double N = 38.78;
const int NameLength = 80, BufLength = 3000, NA = -1;

enum CompType
{ // 器件类型
    MOSFET,
    BJT,
    VSource,
    ISource,
    Inductor,
    Resistor,
    Diode,
    Capacitor
};
enum TranType
{ // tran类型
    NMOS,
    PMOS,
    NPN,
    PNP
};
enum Flag
{ // 标志【用于设置器件端口状态】
    UNSET,
    SET
};
enum Boolean
{ // 逻辑判断
    FALSE,
    TRUE
};
enum EquaType
{ // 方程类型
    Nodal,
    Modified
};
enum AnalysisType
{ // 分析类型
    DC,
    AC,
    TRAN
};

// 七种封装类
class Component;
class ComponentHead;
class Node;
class NodeHead;
class Model;
class ModelHead;
class Netlist;

// 两种信息类
struct Connectors
{               // 器件“端口”类封装信息
    Flag flag;  // “端口”状态
    Node *node; // 本器件该“端口”所连节点实体
    int conNum; // 本器件该“端口”所连节点编号
};
struct Connections
{                      // 连接关系信息封装体
    Connections *next; // 下一个“连接关系”
    Component *comp;   // 指向器件实体
    int conNum;        // 该关系所连节点的“端口”编号
};

class Component
{ // 器件
public:
    Component(CompType typeIn, double valueIn, double tempIn, int con0In,
              int con1In, int con2In, int con3In, Model *modelIn, char *nameIn);
    ~Component();
    CompType getType();
    Component *getNext();
    int getcompNum();
    int getNum();
    int getNodeNum(int conNum);
    double getVal();
    Node *getNode(int conNum);
    char *getName();
    int getConVal(int conNum);

    void setNext(Component *nextIn);
    void setNum(int numIn);
    void connect(int conNum, Node *nodeIn);
    Boolean isCon(int conNum);

    void printMessage(ofstream &outFile, int conNum);
    void genKCLEquation(ofstream &outFile, vector<double> &F_x, vector<double> &X, int datum, int lastnode, int nameNum, int MNAName = NA);
    void genKCLJAC(ofstream &outFile, vector<vector<double>> &JAC, vector<double> &X, int nameNum1, int nameNum2, int datum, int lastnode, int MNAName = NA);
    int genKVLEquation(ofstream &outFile, vector<double> &F_x, vector<double> &X, int datum, int lastnode);
    int genKVLJAC(ofstream &outFile, vector<vector<double>> &JAC, vector<double> &X, int datum, int lastnode);

private:
    Connectors con0, con1, con2, con3; // 器件的4个“端口”
    Component *next;                   // 自身预留接口，以成链
    CompType type;                     // 器件类型
    int compNum;                       // 器件编号（是在本类别中的器件编号）
    double value, temp;                // 器件送值
    Model *model;                      // 器件对应模型
    char name[NameLength];             // 器件名称
};

class Node
{ // 节点
public:
    Node(int num);
    ~Node();
    int getNum();
    int getNameNum();
    int getCount();
    Node *getNext();
    Connections *getConList();

    void setNext(Node *nodeIn);
    void setNameNum(int numIn);
    void connect(int conNumIn, Component *compIn);

    void printMessage(ofstream &outFile);
    void genKCLEquation(ofstream &outFile, vector<double> &F_x, vector<double> &X, int datum, int lastnode, Boolean isMNA);
    void genKCLJAC(ofstream &outFile, vector<vector<double>> &JAC, vector<double> &X, int nameNum2, int datum, int lastnode, Boolean isMNA);

private:
    Node *next;            // 自身预留接口，以成链
    int nodeNum, conCount; // 节点编号【这个是节点生成时的编号，并不是节点在网表中的那个编号（因为网表中的编号可能还不是按自然数顺序来的，若是的话那就差不多等价）】；节点所连组件数【用于筛选datum】
    Connections *conList;  // 节点所相关的连接关系
    int nameNum;           // 节点编号
};

class Model
{ // 模型
public:
    Model(char *nameIn, TranType typeIn, double isIn, double bfIn, double brIn, double tempIn);
    ~Model();
    Model *getNext();
    TranType getType();
    char *getName();
    double getIs();
    double getAf();
    double getAr();
    double getBf();
    double getBr();
    double getTemp();
    double getN();
    void setNext(Model *nextIn);

private:
    char name[NameLength];           // 模型名称
    double is, af, ar, temp, br, bf; // 模型参数
    Model *next;                     // 自身预留接口，以便成链
    TranType type;                   // 模型类型
};

class NodeHead
{ // 节点链表结构
public:
    NodeHead();
    ~NodeHead();
    Node *addNode();
    int getCount();
    Node *getNode(int nodeNum);

private:
    Node *nodeList; // 节点链表
    int nodeCount;  // 链上节点总数
};

class CompHead
{ // 器件链表结构
public:
    CompHead();
    ~CompHead();
    void addComp(Component *component);
    int getCount(CompType type);
    Component *getComp(int compNum);

private:
    Component *compList;                                                  // 器件链表
    int iCount, rCount, dCount, cCount, mCount, vSCount, iSCount, bCount; // 储存每种器件的总数目
};

class ModelHead
{ // 模型链表结构
public:
    ModelHead();
    ~ModelHead();
    void addModel(Model *modelIn);
    Model *getModel(char *nameIn);

private:
    Model *modelList; // 模型链表
};

class Netlist
{ // 网表
public:
    Netlist();
    ~Netlist();
    void setTitle(string name);
    void setAnalysisType(AnalysisType type);
    void setISIC(Boolean isIC);
    void setISNodeset(Boolean isNodeset);
    void setISOptions(Boolean isOptions);
    void insertIC(int id, double value);
    void insertNodeset(int id, double value);
    void insertOptions(string param, double value);
    void setTranStop(double stopTime);
    void setLastnode(int id);
    void setDatum(int id);

    ModelHead &getModelHead();
    CompHead &getCompHead();
    NodeHead &getNodeHead();
    string getTitle();
    AnalysisType getAnalysisType();
    Boolean getISIC();
    Boolean getISNodeset();
    Boolean getISOptions();
    unordered_map<int, double> &getICMap();
    unordered_map<int, double> &getNodesetMap();
    unordered_map<string, double> &getOptionsMap();
    double getTranStop();
    int getDatum();
    int getLastnode();

private:
    ModelHead modelList;
    CompHead compList;
    NodeHead nodeList;
    string title;
    AnalysisType analysisType;
    Boolean is_ic;
    Boolean is_nodeset;
    Boolean is_options;
    unordered_map<int, double> ic;
    unordered_map<int, double> nodeset;
    unordered_map<string, double> options;
    double tran_stop;
    int lastnode;
    int datum;
};

/*****************************************************************************************/
/**************************************Component******************************************/
Component::Component(CompType typeIn, double valueIn = NA, double tempIn = NA,
                     int con0In = NA, int con1In = NA, int con2In = NA, int con3In = NA,
                     Model *modelIn = NULL, char *nameIn = NULL)
{

    type = typeIn;
    con0.conNum = con0In;
    con1.conNum = con1In;
    con2.conNum = con2In;
    con3.conNum = con3In;
    con0.flag = UNSET;
    con1.flag = UNSET;
    con2.flag = UNSET;
    con3.flag = UNSET;
    value = valueIn;
    temp = tempIn;
    next = NULL;
    model = modelIn;
    strcpy(name, nameIn);
}

Component::~Component(){};

/*
 * 将节点实体连接到器件的对应端口上
 * @param conNum 要连接到器件的“端口”号
 * @param nodeIn 要连接到器件上的节点实体
 */
void Component::connect(int conNum, Node *nodeIn)
{
    if (conNum == 0)
    {
        con0.node = nodeIn;
        con0.flag = SET;
    }
    if (conNum == 1)
    {
        con1.node = nodeIn;
        con1.flag = SET;
    }
    if (conNum == 2)
    {
        con2.node = nodeIn;
        con2.flag = SET;
    }
    if (conNum == 3)
    {
        con3.node = nodeIn;
        con3.flag = SET;
    }
}

// 获取器件类型
CompType Component::getType()
{
    return type;
}

int Component::getNum()
{
    return compNum;
}

// 获取器件编号
int Component::getcompNum()
{
    return compNum;
}

// 获取器件链表中的下一项
Component *Component::getNext()
{
    return next;
}

// 获得器件的值
double Component::getVal()
{
    return value;
}

// 添加组件到链上，并完成链接关系
void Component::setNext(Component *nextIn)
{
    next = nextIn;
}

// 设置器件编号
void Component::setNum(int numIn)
{
    compNum = numIn;
}

// 获得本器件某端口所连节点编号
int Component::getConVal(int conNum)
{
    int rtVal;
    if (conNum == 0)
        rtVal = con0.conNum;
    if (conNum == 1)
        rtVal = con1.conNum;
    if (conNum == 2)
        rtVal = con2.conNum;
    if (conNum == 3)
        rtVal = con3.conNum;
    return rtVal;
}

// 判断端口是否可用
Boolean Component::isCon(int conNum)
{
    Boolean rtVal;
    if (conNum == 0)
        rtVal = (con0.flag == SET) ? TRUE : FALSE;
    if (conNum == 1)
        rtVal = (con1.flag == SET) ? TRUE : FALSE;
    if (conNum == 2)
        rtVal = (con2.flag == SET) ? TRUE : FALSE;
    if (conNum == 3)
        rtVal = (con3.flag == SET) ? TRUE : FALSE;
    return rtVal;
}

// 从端口获取节点实体
Node *Component::getNode(int conNum)
{
    switch (conNum)
    {
    case 0:
        return con0.node;
    case 1:
        return con1.node;
    case 2:
        return con2.node;
    case 3:
        return con3.node;
    }
    return NULL;
}

// 从端口获取节点数目【这个没啥用，不用关注】
int Component::getNodeNum(int conNum)
{
    switch (conNum)
    {
    case 0:
        return con0.node->getNum();
    case 1:
        return con1.node->getNum();
    case 2:
        return con2.node->getNum();
    case 3:
        return con3.node->getNum();
    }
    return NA;
}

// 获取器件名称
char *Component::getName()
{
    return name;
}

// 打印器件的信息
void Component::printMessage(ofstream &outFile, int conNum)
{
    switch (type)
    {
    case BJT:
        outFile << "      编号：" << getcompNum() << "    类型："
                << "BJT"
                << " 连接端口：" << conNum << "    名称：" << name;
        outFile << "        value: IS = " << model->getIs() << "  AF = " << model->getAf() << "  AR = " << model->getAr() << "  N = " << model->getN() << endl;
        break;
    case VSource:
        outFile << "      编号：" << getcompNum() << "    类型："
                << "VSource"
                << " 连接端口：" << conNum << "    名称：" << name;
        outFile << "        value: " << value << endl;
        break;
    case Resistor:
        outFile << "      编号：" << getcompNum() << "    类型："
                << "Resistor"
                << " 连接端口：" << conNum << "    名称：" << name;
        outFile << "        value: " << value << endl;
        break;
    case ISource:
        outFile << "      编号：" << getcompNum() << "    类型："
                << "ISource"
                << " 连接端口：" << conNum << "    名称：" << name;
        outFile << "        value: " << value << endl;
        break;
    case Capacitor:
        outFile << "      编号：" << getcompNum() << "    类型："
                << "Capacitor"
                << " 连接端口：" << conNum << "    名称：" << name;
        outFile << "        value: " << value << endl;
        break;
    }
}
/*****************************************************************************************/
/*****************************************Node********************************************/

Node::Node(int Num)
{
    next = NULL;
    nodeNum = Num;
    conCount = 0;
    conList = NULL;
    nameNum = NA;
}

Node::~Node(){};

// 获取节点在链上的编号【其实用于判断节点号一样的时候，用这个namenum也行】
int Node::getNum()
{
    return nodeNum;
}

// 设置节点的编号
void Node::setNameNum(int numIn)
{
    nameNum = numIn;
}

// 获取节点在网表中命名的编号
int Node::getNameNum()
{
    return nameNum;
}

// 获取本节点连接的器件数目
int Node::getCount()
{
    return conCount;
}

// 获取本节点相关的连接关系
Connections *Node::getConList()
{
    return conList;
}

// 将组件连接到节点上
void Node::connect(int conNumIn, Component *compIn)
{
    Connections *conPtr;
    conCount++;
    if (conList == NULL)
    { // 如果当前节点的连接组件链表为空
        conList = new Connections;
        conList->next = NULL;
        conList->conNum = conNumIn; // 这个连接关系的端口是conNumIn号端口
        conList->comp = compIn;     // 指向了传过来的组件
    }
    else
    {
        conPtr = conList;
        while (conPtr->next != NULL)
            conPtr = conPtr->next;
        conPtr->next = new Connections;
        conPtr = conPtr->next;
        conPtr->next = NULL;
        conPtr->conNum = conNumIn;
        conPtr->comp = compIn;
    }
}

// 获取下一个节点指针
Node *Node::getNext()
{
    return next;
}

// 添加节点到链上，并完善连接关系
void Node::setNext(Node *nodeIn)
{
    next = nodeIn;
}

// 打印节点信息
void Node::printMessage(ofstream &outFile)
{
    Connections *conList = getConList();
    while (conList != NULL)
    {
        conList->comp->printMessage(outFile, conList->conNum);
        conList = conList->next;
    }
}
/*****************************************************************************************/
/**************************************Model******************************************/
Model::Model(char *nameIn, TranType typeIn, double isIn, double bfIn, double brIn, double tempIn)
{
    // 由于网表参数的正负值问题，可能会出现结果的错误。
    // 本次设计是ishen为负值
    strcpy(name, nameIn);
    type = typeIn;
    is = isIn;
    br = brIn;
    bf = bfIn;
    af = bfIn / (bfIn + 1);
    ar = brIn / (brIn + 1);
    temp = tempIn;
    next = NULL;
}

Model::~Model() {}

// 获取model类型
TranType Model::getType()
{
    return type;
}

// 获取模型名称
char *Model::getName()
{
    return name;
}

// 获取模型is参数值
double Model::getIs()
{
    return is;
}

// 获取模型bf参数值
double Model::getBf()
{
    return bf;
}

// 获取模型br参数值
double Model::getBr()
{
    return br;
}

// 获取模型af参数值
double Model::getAf()
{
    return af;
}

// 获取模型ar参数值
double Model::getAr()
{
    return ar;
}

// 获取模型temp参数值
double Model::getTemp()
{
    return temp;
}

// 获取N值【注意这里设定了一个恒定的N值，如果temp为默认值NA的话，那么采用38.78】
double Model::getN()
{
    return temp == NA ? 38.78 : (Q / (K * getTemp()));
}

// 添加下一项
void Model::setNext(Model *nextIn)
{
    next = nextIn;
}

// 获取下一项指针
Model *Model::getNext()
{
    return next;
}

/*****************************************************************************************/
/**************************************CompHead*******************************************/
CompHead::CompHead()
{
    compList = NULL;
    mCount = 0;
    bCount = 0;
    iCount = 0;
    rCount = 0;
    dCount = 0;
    cCount = 0;
    vSCount = 0;
    iSCount = 0;
}

CompHead::~CompHead(){};

// 添加器件到链表
void CompHead::addComp(Component *component)
{
    Component *compPtr;
    switch (component->getType())
    {
    case ISource:
        iSCount++;
        if (compList == NULL)
        {
            compList = component;
            component->setNum(iSCount);
        }
        else
        {
            compPtr = compList;
            while (compPtr->getNext() != NULL)
                compPtr = compPtr->getNext();
            compPtr->setNext(component);
            compPtr = compPtr->getNext();
            compPtr->setNum(iSCount);
        }
        break;
    case VSource:
        vSCount++;
        if (compList == NULL)
        {
            compList = component;
            component->setNum(vSCount);
        }
        else
        {
            compPtr = compList;
            while (compPtr->getNext() != NULL)
                compPtr = compPtr->getNext();
            compPtr->setNext(component);
            compPtr = compPtr->getNext();
            compPtr->setNum(vSCount);
        }
        break;
    case Resistor:
        rCount++;
        if (compList == NULL)
        {
            compList = component;
            component->setNum(rCount);
        }
        else
        {
            compPtr = compList;
            while (compPtr->getNext() != NULL)
                compPtr = compPtr->getNext();
            compPtr->setNext(component);
            compPtr = compPtr->getNext();
            compPtr->setNum(rCount);
        }
        break;
    case MOSFET:
        mCount++;
        if (compList == NULL)
        {
            compList = component;
            component->setNum(mCount);
        }
        else
        {
            compPtr = compList;
            while (compPtr->getNext() != NULL)
                compPtr = compPtr->getNext();
            compPtr->setNext(component);
            compPtr = compPtr->getNext();
            compPtr->setNum(mCount);
        }
        break;
    case BJT:
        bCount++;
        if (compList == NULL)
        {
            compList = component;
            component->setNum(bCount);
        }
        else
        {
            compPtr = compList;
            while (compPtr->getNext() != NULL)
                compPtr = compPtr->getNext();
            compPtr->setNext(component);
            compPtr = compPtr->getNext();
            compPtr->setNum(bCount);
        }
        break;
    case Diode:
        dCount++;
        if (compList == NULL)
        {
            compList = component;
            component->setNum(dCount);
        }
        else
        {
            compPtr = compList;
            while (compPtr->getNext() != NULL)
                compPtr = compPtr->getNext();
            compPtr->setNext(component);
            compPtr = compPtr->getNext();
            compPtr->setNum(dCount);
        }
        break;
    case Capacitor:
        cCount++;
        if (compList == NULL)
        {
            compList = component;
            component->setNum(cCount);
        }
        else
        {
            compPtr = compList;
            while (compPtr->getNext() != NULL)
                compPtr = compPtr->getNext();
            compPtr->setNext(component);
            compPtr = compPtr->getNext();
            compPtr->setNum(cCount);
        }
        break;
    case Inductor:
        iCount++;
        if (compList == NULL)
        {
            compList = component;
            component->setNum(iCount);
        }
        else
        {
            compPtr = compList;
            while (compPtr->getNext() != NULL)
                compPtr = compPtr->getNext();
            compPtr->setNext(component);
            compPtr = compPtr->getNext();
            compPtr->setNum(iCount);
        }
        break;
    }
}

// 获取传入类型的器件的总数
int CompHead::getCount(CompType type)
{
    switch (type)
    {
    case ISource:
        return iSCount;
    case VSource:
        return vSCount;
    case Resistor:
        return rCount;
    case Diode:
        return dCount;
    case MOSFET:
        return mCount;
    case BJT:
        return bCount;
    case Capacitor:
        return cCount;
    case Inductor:
        return iCount;
    }
    return NA;
}

// 依据器件编号（链上位置），获取器件实体
Component *CompHead::getComp(int compNum)
{
    Component *compPtr = compList;
    for (int a = 0; a < compNum; a++)
        compPtr = compPtr->getNext();
    return compPtr;
}

/*****************************************************************************************/
/**************************************NodeHead*******************************************/
NodeHead::NodeHead()
{
    nodeList = NULL;
    nodeCount = 0;
}

NodeHead::~NodeHead(){};

// 添加节点到链上
Node *NodeHead::addNode()
{
    Node *nodePtr;
    nodeCount++;
    if (nodeList == NULL)
    {
        nodeList = new Node(nodeCount);
        return nodeList;
    }
    else
    {
        nodePtr = nodeList;
        while (nodePtr->getNext() != NULL)
            nodePtr = nodePtr->getNext();
        nodePtr->setNext(new Node(nodeCount));
        return nodePtr->getNext();
    }
}

// 获取节点链表上的节点总数
int NodeHead::getCount()
{
    return nodeCount;
}

// 根据节点编号获取节点实体
Node *NodeHead::getNode(int nodeNum)
{
    Node *nodePtr = nodeList;
    // need check that nodeNum does not exceed node count
    // 因为在链上节点是按照顺序来的，逐个依序编号的
    for (int a = 0; a < nodeNum; a++)
        nodePtr = nodePtr->getNext();
    return nodePtr;
}

/*****************************************************************************************/
/**************************************ModelHead******************************************/
// 模型链表初始化
ModelHead::ModelHead()
{
    modelList = NULL;
}
ModelHead::~ModelHead() {}

// 添加模型到链上
void ModelHead::addModel(Model *modelIn)
{
    Model *modelPtr;
    if (modelList == NULL)
    {
        modelList = modelIn;
    }
    else
    {
        modelPtr = modelList;
        while (modelPtr->getNext() != NULL)
        {
            modelPtr = modelPtr->getNext();
        }
        modelPtr->setNext(modelIn);
    }
}

// 依据传入名称获取模型实体
Model *ModelHead::getModel(char *nameIn)
{
    Model *modelPtr = modelList;
    while (strcmp(modelPtr->getName(), nameIn))
    {
        modelPtr = modelPtr->getNext();
    }
    return modelPtr;
}

/*****************************************************************************************/
/***************************************Netlist*******************************************/
Netlist::Netlist()
{
    title = "nothing";
    analysisType = DC;
    is_ic = FALSE;
    is_nodeset = FALSE;
    is_options = FALSE;
    tran_stop = NA;
    datum = NA;
    lastnode = NA;
}

// Netlist的析构函数
Netlist::~Netlist() {}

// 设置网表的主题
void Netlist::setTitle(string name)
{
    title = name;
}

// 设置网表分析的类型
void Netlist::setAnalysisType(AnalysisType type)
{
    analysisType = type;
}

// 标记是否有.ic控制条件
void Netlist::setISIC(Boolean isIC)
{
    is_ic = isIC;
}

// 标记是否有.nodeset条件
void Netlist::setISNodeset(Boolean isNodeset)
{
    is_nodeset = isNodeset;
}

// 标记是否有options语句
void Netlist::setISOptions(Boolean isOptions)
{
    is_options = isOptions;
}

// 插入.ic控制条件的条目
void Netlist::insertIC(int id, double value)
{
    ic.insert({id, value});
}

// 插入.nodeset控制条件的条目
void Netlist::insertNodeset(int id, double value)
{
    nodeset.insert({id, value});
}

// 插入.options控制条件的条目
void Netlist::insertOptions(string param, double value)
{
    options.insert({param, value});
}

// 设定瞬态分析中的终止时间
void Netlist::setTranStop(double stopTime)
{
    tran_stop = stopTime;
}

// 设定网表中最大的节点编号
void Netlist::setLastnode(int id)
{
    lastnode = id;
}

// 设定网表中接地节点的编号
void Netlist::setDatum(int id)
{
    datum = id;
}

// 获取网表中的模型链表引用
ModelHead &Netlist::getModelHead()
{
    return modelList;
}

// 获取网表中的器件链表引用
CompHead &Netlist::getCompHead()
{
    return compList;
}

// 获取网表中的节点链表的引用
NodeHead &Netlist::getNodeHead()
{
    return nodeList;
}

// 获取网表的title
string Netlist::getTitle()
{
    return title;
}

// 获取网表分析类型
AnalysisType Netlist::getAnalysisType()
{
    return analysisType;
}

// 获取标记，是否有.ic控制条件
Boolean Netlist::getISIC()
{
    return is_ic;
}

// 获取标记，是否有.nodeset控制条件
Boolean Netlist::getISNodeset()
{
    return is_nodeset;
}

// 获取标记，是否有.options控制条件
Boolean Netlist::getISOptions()
{
    return is_options;
}

// 获取.ic控制条件条目集合的引用
unordered_map<int, double> &Netlist::getICMap()
{
    return ic;
}

// 获取.nodeset控制条件条目集合的引用
unordered_map<int, double> &Netlist::getNodesetMap()
{
    return nodeset;
}

// 获取.options控制条件条目集合的引用
unordered_map<string, double> &Netlist::getOptionsMap()
{
    return options;
}

// 获取瞬态分析中的终止时间
double Netlist::getTranStop()
{
    return tran_stop;
}

// 获取网表中接地节点的编号
int Netlist::getDatum()
{
    return datum;
}

// 获取网表中最大的节点编号
int Netlist::getLastnode()
{
    return lastnode;
}
