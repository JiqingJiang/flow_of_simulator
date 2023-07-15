#include "parse.h"

// 将传入的字符串解析为double型数字
double stripString(char *stringIn)
{
    char buf[BufLength], buf2[BufLength];
    int a, b;
    strcpy(buf, stringIn);
    for (a = 0; buf[a] != '='; a++)
        ;
    a++;
    for (b = 0; buf[a] != '\0'; b++, a++)
        buf2[b] = buf[a];
    buf2[b] = '\0';
    return atof(buf2); // 转成浮点数
}
void parseNetList(Netlist &netlist, string &inFileName, string &outFileName)
{
    ifstream inFile;
    ofstream outFile;
    NodeHead &nodeList = netlist.getNodeHead();
    CompHead &compList = netlist.getCompHead();
    ModelHead &modelList = netlist.getModelHead();

    // 分析过程中所需的变量
    char buf[BufLength], buf1[BufLength], buf2[BufLength], nameBuf[NameLength];
    char *bufPtr, *charPtr1, *charPtr2;
    int intBuf1, intBuf2, intBuf3, intBuf4;
    double douBuf1, douBuf2, douBuf3, douBuf4;
    CompType typeBuf;
    Component *compPtr, *compPtr1, *compPtr2;
    Node *nodePtr, *nodePtr1, *nodePtr2;
    Model *modelPtr;
    TranType TtypeBuf;

    // 2. 处理输入文件相关
    if (inFileName.empty())
    {
        cerr << "Please enter the input Spice Netlist: <\"QUIT\" to exit>" << endl;
        cin >> inFileName;
        if (inFileName == "QUIT")
        {
            cerr << "Program Exited Abnormally!" << endl;
            exit(0);
        }
    }
    inFile.open(inFileName, ios::in);
    while (!inFile)
    {
        cerr << inFileName << " is an invalid input file." << endl
             << "Please enter the input Spice Netlist: <\"QUIT\" to exit>" << endl;
        cin >> inFileName;
        if (inFileName == "QUIT")
        {
            cerr << "Program Exited Abnormally!" << endl;
            exit(0);
        }
        inFile.open(inFileName, ios::in);
    }

    // 3. 处理输出文件相关
    if (outFileName.empty())
    {
        outFileName = inFileName + ".out";
    }
    cout << endl
         << "Output saved to file: " << outFileName << endl;

    cout << endl
         << "==================The process of netlist parsing starts===================" << endl;

    // 4. 网表解析  /*对于一个网表的识别，除了应包含点器件连接关系、模型声明语句、结束语句，还应包含控制语句和分析语句*/
    // 4.1 标题行处理
    inFile.getline(buf, BufLength);
    netlist.setTitle(buf);

    // 4.2 扫描点语句相关
    inFile.getline(buf, BufLength);
    cout << endl
         << "===================(1) Dot statements scan has started====================" << endl;
    while (inFile.good())
    {
        if ((buf == NULL) || (*buf == '\0'))
        {
            inFile.getline(buf, BufLength);
            continue;
        }
        strcpy(buf1, buf);
        strcpy(buf2, strtok(buf1, " "));
        /*TODO：对于各种点语句的识别*/
        if (!strcmp(buf2, ".model"))
        {                                    // strtok是一个分解字符串的函数，如果不是模型申明语句，那么跳过
            strcpy(buf2, strtok(NULL, " ")); // 继续分割，赋值到buf2，此时buf2是器件的名称
            charPtr1 = strtok(NULL, " ");    // 继续分割给到charPtr1，此时charPtr1是器件的类型
            if (!strcmp(charPtr1, "PNP"))    // 类型处理
                TtypeBuf = PNP;
            else if (!strcmp(charPtr1, "NPN"))
                TtypeBuf = NPN;
            else if (!strcmp(charPtr1, "NMOS"))
                TtypeBuf = NMOS;
            else if (!strcmp(charPtr1, "PMOS"))
                TtypeBuf = PMOS;

            charPtr1 = strtok(NULL, " "); // 继续分割，此时charPtr1后面的是晶体管的参数值
            double temp1 = NA, temp2 = NA, temp3 = NA, temp4 = NA;
            while (charPtr1 != NULL)
            { // 若有参数
                // 下面处理就是四种参数的值，分别对应就行。stripString是将字符串转成浮点数double的
                if ((charPtr1[0] == 'I') && (charPtr1[1] == 'S') && (charPtr1[2] == '='))
                {
                    temp1 = stripString(charPtr1);
                }
                if ((charPtr1[0] == 'B') && (charPtr1[1] == 'F') && (charPtr1[2] == '='))
                {
                    temp2 = stripString(charPtr1);
                }
                if ((charPtr1[0] == 'B') && (charPtr1[1] == 'R') && (charPtr1[2] == '='))
                {
                    temp3 = stripString(charPtr1);
                }
                if ((charPtr1[0] == 'T') && (charPtr1[1] == 'E') && (charPtr1[4] == '='))
                {
                    temp4 = stripString(charPtr1);
                }
                charPtr1 = strtok(NULL, " ");
            }
            modelPtr = new Model(buf2, TtypeBuf, temp1, temp2, temp3, temp4);
            modelList.addModel(modelPtr);
        }

        if (!strcmp(buf2, ".tran"))
        {
            charPtr1 = strtok(NULL, " ");
            while (charPtr1 != NULL)
            {
                string s(charPtr1);
                if (s.substr(0, 4) == "stop")
                {
                    // cout << stripString(buf2) << endl;
                    // netlist.setTranStop(stripString(buf2));
                    netlist.setTranStop(stripString(charPtr1));
                }
                /*TODO：处理其他的参数情况*/
                charPtr1 = strtok(NULL, " ");
            }
            netlist.setAnalysisType(TRAN);
            // cout << netlist.getAnalysisType();
        }

        // if (!strcmp(strtok(buf1, " "), ".ic")) {
        //     //待补充
        // }

        // if (!strcmp(strtok(buf1, " "), ".nodeset")) {
        //     //待补充
        // }

        // if (!strcmp(strtok(buf1, " "), ".options")) {
        //     //待补充
        // }

        inFile.getline(buf, BufLength);
    }

    inFile.close();
    inFile.clear();
    inFile.open(inFileName, ios::in);

    // 4.3 器件遍历扫描，并入链
    cout << endl
         << "=====================(2) Components scan has started======================" << endl;
    char model_str[9];
    inFile.getline(buf, BufLength);
    inFile.getline(buf, BufLength);
    while (inFile.good())
    {
        if ((buf == NULL) || (*buf == '\0'))
        {
            inFile.getline(buf, BufLength);
            continue;
        }
        if (isalpha(*buf))
        { // 首字母是符号，因为每种组件都有对应的代号名称，如电阻是r，bjt是q，v是电压源
            //  EDIT THIS SECTION IF NEW COMPONENTS ARE ADDED!!!
            //  we could do some rearranging in this section to catch each type in order.
            switch (*buf)
            {
            case 'v':
            case 'V':
                typeBuf = VSource; // 电压源类型
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                douBuf1 = atof(strtok(NULL, " ")); // 该器件的属性值
                compPtr = new Component(typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
                compList.addComp(compPtr);
                break;

            case 'i':
            case 'I':
                typeBuf = ISource; // 电流源类型
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                douBuf1 = atof(strtok(NULL, " "));
                compPtr = new Component(typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
                compList.addComp(compPtr);
                break;

            case 'q':
            case 'Q':
                typeBuf = BJT; // bjt类型
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                intBuf3 = atoi(strtok(NULL, " "));
                compPtr = new Component(typeBuf, NA, NA, intBuf1, intBuf2, intBuf3, NA,
                                        modelList.getModel(strtok(NULL, " ")), nameBuf); // 找到对应的模型，给加进去
                compList.addComp(compPtr);
                break;

            case 'm':
            case 'M':
                typeBuf = MOSFET; // mos类型
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                intBuf3 = atoi(strtok(NULL, " "));
                intBuf4 = atoi(strtok(NULL, " "));
                compPtr = new Component(typeBuf, NA, NA, intBuf1, intBuf2, intBuf3, intBuf4,
                                        modelList.getModel(strtok(NULL, " ")), nameBuf);
                compList.addComp(compPtr);
                break;

            case 'r':
            case 'R':
                typeBuf = Resistor; // 电阻类型
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                douBuf1 = atof(strtok(NULL, " "));
                compPtr = new Component(typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
                compList.addComp(compPtr);
                break;

            case 'd':
            case 'D':
                typeBuf = Diode; // 就是只有这种类型的组件有tempin送值
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                charPtr1 = strtok(NULL, " ");
                while (charPtr1 != NULL)
                {
                    if ((charPtr1[0] == 'I') && (charPtr1[1] == 'S') && (charPtr1[2] == '='))
                    {
                        douBuf1 = stripString(charPtr1);
                    }
                    if ((charPtr1[0] == 'T') && (charPtr1[1] == 'E') && (charPtr1[4] == '='))
                    {
                        douBuf2 = stripString(charPtr1);
                    }
                    charPtr1 = strtok(NULL, " ");
                }
                compPtr = new Component(typeBuf, douBuf1, douBuf2, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
                compList.addComp(compPtr);
                break;

            case 'c':
            case 'C':
                typeBuf = Capacitor;
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                douBuf1 = atof(strtok(NULL, " "));
                compPtr = new Component(typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
                compList.addComp(compPtr);
                break;

            case 'l':
            case 'L':
                typeBuf = Inductor;
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                douBuf1 = atof(strtok(NULL, " "));
                compPtr = new Component(typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
                compList.addComp(compPtr);
                break;
            }
        }
        inFile.getline(buf, BufLength);
    }
    inFile.close();
    inFile.clear();

    cout << endl
         << "=====================(3) The contents of the netlist have been scanned====" << endl
         << endl;

    /************************************************************/
    /** 对于起始节点编号的确定是一个问题。常见的是接地点为0号节点， **/
    /** 也即为第一个节点。但似乎有约定是所连器件数最多的节点作为公  **/
    /** 共节点，即接地节点。                                    **/
    /************************************************************/

    // 4.3 依据器件链表，进行节点遍历扫描，节点成链并与器件建立联系
    cout << endl
         << "=====================(4) The node generation process has started==========" << endl;
    compPtr1 = compList.getComp(0);
    while (compPtr1 != NULL)
    {
        for (int b = 0; b < 3; b++)
        { // TODO：遇到4个端口的器件需注意这里的改变
            // 验证端口b是否被遍历过了 &&  端口所连节点编号存在
            if ((!compPtr1->isCon(b)) && (compPtr1->getConVal(b) != NA))
            {
                intBuf1 = compPtr1->getConVal(b);
                // 【这边应该有一个去重的操作，不然会重复创建节点。这里就不用去重了，因为下面人家也是直接扫描了，组件connect的时候会置SET】
                nodePtr1 = nodeList.addNode();  // 新生成节点，并在每个节点中记录当前已有多少个节点【其实是节点编号】，同样在头结点链表中也有记录
                nodePtr1->setNameNum(intBuf1);  // ~> naming the node as in the netlist file
                compPtr1->connect(b, nodePtr1); // ~> connecting the 'connector' of component to the node
                nodePtr1->connect(b, compPtr1); // ~> connecting the 'connection' of the node to the component

                // now search and connect all other appropriate connectors to this node.
                // error checking should be added to prevent duplicated, or skipped connectors.
                // compPtr2 = compPtr1->getNext();
                compPtr2 = compPtr1->getNext();
                while (compPtr2 != NULL)
                {
                    for (int c = 0; c < 3; c++)
                    {
                        if (compPtr2->getConVal(c) == intBuf1)
                        {
                            compPtr2->connect(c, nodePtr1);
                            nodePtr1->connect(c, compPtr2);
                            break;
                        }
                    }
                    compPtr2 = compPtr2->getNext();
                }
            }
        }
        compPtr1 = compPtr1->getNext();
    }

    // 5. 确认传入的起始节点编号可用、正确；如果没有传入起始节点编号，那么找到连接器件数最多的节点作为接地节点
    if (netlist.getDatum() != NA)
    {
        Boolean check = FALSE;
        nodePtr = nodeList.getNode(0);
        while (nodePtr != NULL)
        {
            if (nodePtr->getNameNum() == netlist.getDatum())
                check = TRUE;
            nodePtr = nodePtr->getNext();
        }
        if (check == FALSE)
        {
            cerr << "Datum value invalid!" << endl
                 << "PROGRAM EXITED ABNORMALLY!" << endl;
            exit(0);
        }
    }
    else
    {
        nodePtr = nodeList.getNode(0);
        nodePtr1 = nodePtr->getNext();
        while (nodePtr1 != NULL)
        {
            if (nodePtr1->getCount() > nodePtr->getCount())
                nodePtr = nodePtr1;
            nodePtr1 = nodePtr1->getNext();
        }
        netlist.setDatum(nodePtr->getNameNum()); // datum是Count数最大的那个
    }

    // 6. 找到节点编号中最大的
    // 注意：不能简单地通过nodeList中的nodeCount就得出编号，因为对于网表描述的不一，有的0接地，有的不存在0
    nodePtr = nodeList.getNode(0); //~> getting the pointer to the first node, pointed by 'headNode'
    int lastnode = nodePtr->getNameNum();
    while (nodePtr != NULL)
    {
        lastnode = (nodePtr->getNameNum() > lastnode) ? nodePtr->getNameNum() : lastnode;
        nodePtr = nodePtr->getNext();
    }
    netlist.setLastnode(lastnode);

    // 7. 输出解析内容，检查是否正确解析
    outFile.open(outFileName + "_parser.txt", ios::out);
    if (!outFile.is_open())
    {
        cerr << "Failed to open " << outFileName + "_parser.txt" << '\n';
        exit(0);
    }
    outFile << "Title: " << netlist.getTitle() << endl;
    outFile << "datum = " << netlist.getDatum() << "        lastnode = " << netlist.getLastnode() << endl;
    nodePtr = nodeList.getNode(0);
    while (nodePtr != NULL)
    {
        outFile << "节点" << nodePtr->getNameNum() << "        所连器件数为：" << nodePtr->getCount() << endl;
        nodePtr->printMessage(outFile);
        nodePtr = nodePtr->getNext();
    }
    outFile.close();
    outFile.clear();

    cout << endl
         << "==================The netlist is parsed completely========================" << endl;

    return;
}

int main()
{
    string path = "/home/jiangjiq/桌面/homework/Parse/Netlist4.txt";
    string output = "Netlist4.out";
    Netlist netlist;
    parseNetList(netlist, path, output);

    return 0;
}