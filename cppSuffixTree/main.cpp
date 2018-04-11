//
//  main.cpp
//  cppSuffixTree
//
//  Created by longbaolin on 2018/4/10.
//  Copyright © 2018年 longbaolin. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <set>
#include <vector>

using namespace std;

/**
 *上下文
 */
class Context {
public:
    vector<string>* texts;
    vector<string>* locations;
    int * count;
    
    Context(int * cou, vector<string> * text, vector<string> * location) {
        count = cou;
        texts = text;
        locations = location;
    }
};

/**
 * 指向text中某一个位置的指针
 * */
class Index {
public:
    int cur = -1;
    Index(){}
    Index(int c) {
        cur = c;
    }
};

/**
 * SuffixTree的节点
 * */
class Node {
public:
    //上下文
    Context * context;
    //所包含的字符串开始的位置
    Index * left = NULL;
    //所包含的字符串结束的位置
    Index * right = NULL;
    //所有的子节点
    vector<Node*> subs = vector<Node*>();
    //SuffixLink
    Node * suffixNode = NULL;
    //文本列表
    set<int> * equi;
    //引用文本
    int ref = -1;
    //唯一标识符
    int flag = 0;
    
    Node(Context * c) {
        context = c;
        flag = *context->count;
        *context->count = *context->count + 1;
    }
    Node(Context * c, Index * l, Index * r) {
        context = c;
        flag = *context->count;
        *context->count = *context->count + 1;
        left = l;
        right = r;
        ref = (int)context->texts->size() - 1;
        equi = new set<int>();
        equi->insert(ref);
    }
    
    string toString() {
        if (flag == 0) {
            return "root";
        }
        string flagStr = to_string(flag);
        //string lefts = to_string(left->cur);
        //string rights = to_string(right->cur);
        //return lefts + rights + flagStr;
        string refs = context->texts->at(ref);
        int leftIndex = left->cur;
        int rightIndex = right->cur + 1;
        return refs.substr(leftIndex, rightIndex - leftIndex) + flagStr;
    }
};

class ActivePoint {
public:
    //活动节点
    Node * active_node;
    int active_edg;
    int active_length;
    
    ActivePoint(Node * node, int edg, int length) {
        active_node = node;
        active_edg = edg;
        active_length = length;
    }
};

class SuffixTree {
public:
    //当前节点数统计
    int count = 0;
    //保存用来建树的文本
    vector<string> texts = vector<string>();
    //相应的文本在代码中所在的位置
    vector<string> locations = vector<string>();
    //后缀树的上下文,必须唯一
    Context context = Context(&count, &texts, &locations);
    //SuffixTree的根结点
    Node root = *(new Node(&context));
    //活动点, 活动点的对象也必须唯一
    ActivePoint activePoint = ActivePoint(&root, -1, -1);
    //在当前build步骤还需要插入多少节点
    int remainder = 0;
    //相似方法结果
    set<set<string>> nameSetSet = set<set<string>>();
    //当前用来建树的文本
    string curText;
    //表示新加入文本是否已存在的flag
    bool spilt = false;
    /**
     * SuffixTree的构造器
     * */
    SuffixTree(){
    }
    
    void addText(string text, string location) {
        //重置上下文状态
        remainder = 0;
        activePoint.active_node = &root;
        activePoint.active_edg = -1;
        activePoint.active_length = -1;
        spilt = false;
        
        string * t = new string(text);
        curText = *t + "$";
        texts.push_back(*t + "$");
        locations.push_back(location);
        
        build();
    }
    
    /**
     * 把新文本加入后缀树
     * */
    void build() {
        Index index = Index(0);
        while (index.cur < curText.size()) {
            //cout << "Tree Structure before index++" << endl;
            print();
            
            //需要插入的下一个字符
            char insert = curText.at(index.cur);
            
            //如果一个新添加的字符已经存在，直接往后运行
            //cout << "在Index相应增加的时候查找待插入字符是否存在" << endl;
            if (find(insert)) {
                remainder++;
                index.cur++;
                continue;
            }
            
            //如果这个字符不存在的话👀
            spilt = true;
            if (remainder == 0) { //只需要插入当前字符
                //当remainder为0的时候肯定是在根节点上
                Index* newNodeLeft = new Index(index.cur);
                Node* newNode = new Node(&context, newNodeLeft, &index);
                //cout << "新插入节点" << newNode->toString() << "位置"  << &newNode << endl;
                activePoint.active_node->subs.push_back(newNode);
            } else { //还需要处理之前的步骤留下来的后缀们
                remainder++;
                innerSplit(&index, NULL);
            }
            index.cur++;
        }
        index.cur--;
        resetIndex();
    }
    
    /*
     * 处理剩余的插入后缀
     * */
    void innerSplit(Index* index, Node* prefixNode) {
        //cout << endl;
        //cout << endl;
        string prefixString = "NULL";
        if (prefixNode != NULL) {
            prefixString = prefixNode->toString();
        }
        //cout << "********Inner Split with index: " << index->cur <<  "********* prefixNode: " << prefixString << endl;
        //cout << "Deal With remainder: " << remainder << endl;
        //cout << "Active Point before Insertion: " << endl;
        printActivePoint();
        
        char insert = curText.at(index->cur);
        //cout << "在递归插入的流程中寻找待插入字符是否存在" << endl;
        if (find(insert)) {
            //cout<<"待插入后缀： " <<  insert << " 已找到，暂且退出递归"<< endl;
            return;
        }
        //cout << "Active Point before Insertion: " << endl;
        printActivePoint();
        
        if ( activePoint.active_length == -1) {
            Index* leftIndex = new Index(index->cur);
            Node* insertNode = new Node(&context, leftIndex, index);
            activePoint.active_node->subs.push_back(insertNode);
            prefixNode = NULL;
        } else {
            Node* splitNode = activePoint.active_node->subs.at(activePoint.active_edg);
            if (activePoint.active_length < splitNode->right->cur) {
                //cout << "开始split........." << endl;
                //把原来的一个节点分割成两个节点
                Index* leftIndex = new Index(splitNode->left->cur);
                Index* rightIndex = new Index(splitNode->left->cur + activePoint.active_length);
                Node* newNode = new Node(&context, leftIndex, rightIndex);
                newNode->ref = splitNode->ref;
                
                Index* splitLeft = new Index(splitNode->left->cur + activePoint.active_length + 1);
                splitNode->left = splitLeft;
                
                //cout << "插入用来分裂的新节点：" << newNode->toString() << endl;
                
                if (prefixNode != NULL) {
                    prefixNode->suffixNode = newNode;
                }
                
                newNode->subs.push_back(splitNode);
                
                vector<Node *>::iterator idx;
                for (vector<Node *>::iterator i = activePoint.active_node->subs.begin(); i != activePoint.active_node->subs.end(); i++) {
                    if ((*i)->flag  == splitNode->flag) {
                        idx = i;
                        break;
                    }
                }
                activePoint.active_node->subs.erase(idx);
                activePoint.active_node->subs.push_back(newNode);
                //插入需要插入的新节点
                Index* insertLeft = new Index();
                insertLeft->cur = index->cur;
                Node* insertNode = new Node(&context, insertLeft, index);
                newNode->subs.push_back(insertNode);
                
                prefixNode = newNode;
            }
        }
        
        //减少remainder
        remainder--;
        if (remainder == 0) {
            print();
//            cout << "********递归结束: " << index->cur << "*********"<<endl;
//            cout << "Active Point after Insertion: " << endl;
            printActivePoint();
//            cout<< endl;
//            cout<< endl;
//            cout<< endl;
            return;
        }
        
//        cout << "插入完成，检测sufffixNode: " << activePoint.active_node->suffixNode << endl;
        //节点已经插入完毕，根据规则一和规则三对ActiveNode进行处理
        if (activePoint.active_node == &root) {
//            cout << "活动点是root......" << endl;
            activePoint.active_length--;
            activePoint.active_edg = -1;
//            cout << "..........Find Index: " << index->cur - remainder + 1 << endl;
            char newIndex = curText.at(index->cur - remainder + 1);
            for (int i = 0; i < activePoint.active_node->subs.size(); i++) {
                Node* cur = activePoint.active_node->subs.at(i);
                if (texts.at(cur->ref).at(cur->left->cur) == newIndex) {
                    activePoint.active_edg = i;
                    break;
                }
            }
            dealWithActiveNodeTrans(index);
        } else if (activePoint.active_node->suffixNode == NULL) {
//            cout << "活动点: " << texts.at(activePoint.active_node->ref).at(activePoint.active_node->left->cur) << " suffix为空......" << endl;
            activePoint.active_node = &root;
            activePoint.active_length = index->cur - (index->cur - remainder + 1) - 1;
//            cout << "newIndex's index: " << (index->cur - remainder + 1) << endl;
            char newIndex = curText.at(index->cur - remainder + 1);
            activePoint.active_edg = -1;
            for (int i = 0; i < activePoint.active_node->subs.size(); i++) {
                Node* cur = activePoint.active_node->subs.at(i);
                if (texts.at(cur->ref).at(cur->left->cur) == newIndex) {
                    activePoint.active_edg = i;
                    break;
                }
            }
            dealWithActiveNodeTrans(index);
        } else {
//            cout << "sssssssssss --- follow suffix link" << endl;
            activePoint.active_node = activePoint.active_node->suffixNode;
            int preIndex = index->cur - remainder + 1;
            int insetLength = index->cur - preIndex + 1;
            int impPrefix = insetLength - 1 - (activePoint.active_length + 1);
            int charIndex = preIndex + impPrefix;
            char newIndex = curText.at(charIndex);
            //System.out.println("Char to insert: " + newIndex + " And next char to Insert " + text.charAt(i + 1));
            for (int j = 0; j < activePoint.active_node->subs.size(); j++) {
                Node* cur = activePoint.active_node->subs.at(j);
                if (texts.at(cur->ref).at(cur->left->cur) == newIndex) {
                    activePoint.active_edg = j;
                    break;
                }
            }
            
            int sub = 0;
            while (activePoint.active_edg >= 0) {
//                cout << "在处理边长度不够的情况 length = " << activePoint.active_length << endl;
                
                Node* edg = activePoint.active_node->subs.at(activePoint.active_edg);
                int length = activePoint.active_length;
//                cout << "Edg: " << edg << ": " << texts.at(edg->ref).substr(edg->left->cur, edg->right->cur - edg->left->cur + 1) << endl;
                if (edg->right->cur - edg->left->cur < length) {
//                    cout << "边长度不够的时候有剩余的往前跳" << endl;
                    activePoint.active_node = edg;
                    activePoint.active_length -= edg->right->cur - edg->left->cur + 1;
                    sub += edg->right->cur - edg->left->cur + 1;
                    activePoint.active_edg = -1;
                } else if (edg->right->cur - edg->left->cur == length) {
//                    cout << "边长度恰好的时候往前跳" << endl;
                    activePoint.active_node = edg;
                    activePoint.active_length = -1;
                    activePoint.active_edg = -1;
                    break;
                } else {
//                    cout << "边长足够，不跳" << endl;
                    break;
                }
                
                char find = curText.at(charIndex + sub);
//                cout << "边长度不够，前进一个节点，下一个被查找的字符是：" << find << " remainder是：" << remainder << " index cur是： " << index->cur << endl;
                for (int j = 0; j < activePoint.active_node->subs.size(); j++) {
                    Node* cur = activePoint.active_node->subs.at(j);
                    if (texts.at(cur->ref).at(cur->left->cur) == find) {
                        activePoint.active_edg = j;
                        break;
                    }
                }
                
            }
            
        }
        
//        cout << "Tree Structure after insertion" << endl;
//        print();
//        cout << "Active Point after Insertion: " << endl;
        printActivePoint();
        
//        cout << "********Done Split with index: " << index->cur << "*********"<< endl;
//        cout << endl;
//        cout << endl;
//        cout << endl;
        
        innerSplit(index, prefixNode);
    }
    
    
    /**
     * 处理活动点转移的时候活动边长度不够的问题
     * */
    void dealWithActiveNodeTrans(Index* index) {
        //处理新边长度不够的情况
        int sub = 0;
        while (activePoint.active_edg >= 0 && activePoint.active_length >= 0) {
            //cout << "在处理边长度不够的情况 length = " << activePoint.active_length <<endl;
            Node* edg = activePoint.active_node->subs.at(activePoint.active_edg);
            int length = activePoint.active_length;
            //cout << "Edg: " << edg << ": " << texts.at(edg->ref).substr(edg->left->cur, edg->right->cur - edg->left->cur + 1) << endl;
            if (edg->right->cur - edg->left->cur < length) {
                //cout << "边长度不够的时候有剩余的往前跳" << endl;
                activePoint.active_node = edg;
                activePoint.active_length -= edg->right->cur - edg->left->cur + 1;
                sub += edg->right->cur - edg->left->cur + 1;
                activePoint.active_edg = -1;
            } else if (edg->right->cur - edg->left->cur == length) {
                //cout << "边长度恰好的时候往前跳" << endl;
                activePoint.active_node = edg;
                activePoint.active_length = -1;
                activePoint.active_edg = -1;
                break;
            } else {
                //cout << "边长足够，不跳" << endl;
                break;
            }
            
            char find = curText.at(index->cur - remainder + 1 + sub);
            //cout << "边长度不够，前进一个节点，下一个被查找的字符是：" << find << " remainder是：" << remainder << " index cur是： " << index->cur << endl;
            for (int i = 0; i < activePoint.active_node->subs.size(); i++) {
                Node* cur = activePoint.active_node->subs.at(i);
                if (texts.at(cur->ref).at(cur->left->cur) == find) {
                    activePoint.active_edg = i;
                    break;
                }
            }
            
        }
    }
    
    bool find(char c) {
        //cout << "检测当前需要插入的字符是否已经被隐式包含了 Find character " << c << endl;
        if (activePoint.active_edg == -1) {
            for (int i = 0; i < activePoint.active_node->subs.size(); i++) {
                Node * curNode = activePoint.active_node->subs.at(i);
                string textToCompare = texts.at(curNode->ref);
                //cout << "TextToCompare: " << textToCompare << "Index: "  << curNode->left->cur << endl;
                if (textToCompare.at(curNode->left->cur) == c) {
                    activePoint.active_edg = i;
                    activePoint.active_length = 0;
                    if (c == '$' && !spilt) {
                        int number = (int)texts.size() - 1;
                        activePoint.active_node->subs.at(activePoint.active_edg)->equi->insert(number);
                    }
                    if (curNode->left->cur + activePoint.active_length == curNode->right->cur) {
                        activePoint.active_node = curNode;
                        activePoint.active_edg = -1;
                        activePoint.active_length -= curNode->right->cur - curNode->left->cur + 1;
                        //cout << "往前跳一个节点 + " << activePoint.active_length << endl;
                    }
                    return true;
                }
            }
        } else {
            Node* curNode = activePoint.active_node->subs.at(activePoint.active_edg);
            if (curNode->left->cur + activePoint.active_length == curNode->right->cur) {
                for (int i = 0; i < curNode->subs.size(); i++) {
                    Node * subNode = curNode->subs.at(i);
                    if (texts.at(subNode->ref).at(subNode->left->cur) == c) {
                        activePoint.active_node = curNode;
                        activePoint.active_edg = i;
                        activePoint.active_length = 0;
                        //cout << "往前跳一个节点" << endl;
                        if (c == '$' && !spilt) {
                            int number = (int)texts.size() - 1;
                            activePoint.active_node->subs.at(activePoint.active_edg)->equi->insert(number);
                        }
                        return true;
                    }
                }
            }else {
                if (texts.at(curNode->ref).at(curNode->left->cur + activePoint.active_length + 1) == c) {
                    activePoint.active_length++;
                    if (curNode->left->cur + activePoint.active_length == curNode->right->cur) {
                        activePoint.active_node = curNode;
                        activePoint.active_edg = -1;
                        activePoint.active_length -= curNode->right->cur - curNode->left->cur + 1;
                        //cout << "往前跳一个节点" << endl;
                        if (c == '$' && !spilt) {
                            int number = (int)texts.size() - 1;
                            activePoint.active_node->equi->insert(number);
                        }
                    }
                    return true;
                }
            }
        }
        //cout << "Character " << c << "  misMatch" << endl;
        return false;
    }
    
    /**
     *在一次构造完成后将所有叶子节点的随text的index自增长指针都固定下来
     */
    void resetIndex() {
        vector<Node *> list = vector<Node *>();
        for (Node * sub : root.subs) {
            list.push_back(sub);
        }
        vector<Node *> temp = vector<Node *>();
        
        while (true) {
            
            for (Node * n : list) {
                for (Node * sub : n->subs) {
                    temp.push_back(sub);
                }
                if (n->subs.size() == 0) {
                    n->right = new Index(n->right->cur);
                }
            }
            if (temp.size() == 0) {
                break;
            }
            list = vector<Node*>(temp);
            temp.clear();
        }
        
    }
    
    /**
     * 格式化打印出整个后缀树
     * 层次遍历，按照层次打印
     */
    void print() {
        //cout << "---------Tree Structure---------" << root.subs.size() <<endl;
        vector<Node *> list = vector<Node *>();
        for (Node * sub : root.subs) {
            list.push_back(sub);
        }
        vector<Node *> temp = vector<Node *>();
        
        int line = 1;
        while (true) {
            
            for (Node * n : list) {
                
                string suff = "NULL";
                if (n -> suffixNode != NULL) {
                    suff = n->suffixNode->toString();
                }
                //cout << n->toString()  << " " << suff << "(" << line << ")    ";
                for (Node * sub : n->subs) {
                    temp.push_back(sub);
                    //cout << "|-" << sub->toString();
                }
                //cout << "    ";
            }
            
            //cout << endl;
            if (temp.size() == 0) {
                break;
            }
            list = vector<Node*>(temp);
            temp.clear();
            line++;
        }
    }
    
    /*
     * 打印当前的ActivePoint
     * **/
    void printActivePoint() {
//        cout << "@@@@@@@@@ Current activePoint structure @@@@@@@@@" << endl;
//        cout << "ActiveNode: " << activePoint.active_node->toString() << "   " << endl;
//        cout << "ActiveLength: " << activePoint.active_length << endl;
        if (activePoint.active_edg == -1) {
            //cout << "ActiveEdg: " << activePoint.active_edg<< endl;
        } else {
            Node* edg = activePoint.active_node->subs.at(activePoint.active_edg);
            //cout << "ActiveEdg: " <<  texts.at(edg->ref).substr(edg->left->cur, edg->right->cur - edg->left->cur + 1);
        }
        if (activePoint.active_node->suffixNode != NULL) {
            //cout << "Has Suffix:  " << activePoint.active_node->suffixNode->toString() << endl;
        }
        //cout<<"@@@@@@@@@ end activePoint structure @@@@@@@@@"<<endl;
    }
    
    /**
     * 获取等价类
     * */
    set<set<string>> getEquivalenceClass(){
        vector<Node *> list = vector<Node *>(root.subs);
        vector<Node *> temp = vector<Node *>();
        int index = 0;
        
        while (true) {
            for (Node* n : list) {
                //cout << "节点的equi大小" << n->equi->size() << endl;
                for (Node* sub : n->subs) {
                    temp.push_back(sub);
                }
                if (index > 0 && n->equi->size() > 1) {
                    //说明是叶子节点，输出等价类
                    set<string> nameSet = set<string>();
                    
                    for (int p : *n->equi) {
                        nameSet.insert(locations.at(p));
                    }
                    if (nameSet.size() <= 1) {
                        continue;
                    }
                    nameSetSet.insert(nameSet);
                }
            }
            if (temp.size() == 0) break;
            list = temp;
            temp = vector<Node *>();
            index++;
        }
        //cout << "得到了等价类" << nameSetSet.size() << endl;
        return nameSetSet;
    }
};

class CloneDetector {
public:
    //结果集
    set<set<string>> equiSet = set<set<string>>();

    void detect(vector<string> tests, vector<string> locations) {
        
        //正向查找
        SuffixTree suffixTree = SuffixTree();
        for (int i = 0; i < (int)tests.size(); i++) {
            //cout << "Add text : " << tests[i] << endl;
            suffixTree.addText(tests.at(i), locations.at(i));
            //suffixTree.print();
        }
        for (set<string> s : suffixTree.getEquivalenceClass()) {
            equiSet.insert(s);
        }
        
        //反向查找
        SuffixTree suffixTreeR = SuffixTree();
        for (int i = (int)tests.size() - 1; i >= 0; i--) {
            //cout << "Add text : " << tests[i] << endl;
            suffixTreeR.addText(tests.at(i), locations.at(i));
            //suffixTree.print();
        }
        for (set<string> s : suffixTreeR.getEquivalenceClass()) {
            equiSet.insert(s);
        }
        
        //结果集合并
        equiMerge();
        
        cout << "开始输出得到的等价类: " << equiSet.size() << endl;
        for (set<string> s : equiSet) {
            for (string str : s) {
                cout << str;
            }
            cout << endl;
        }
        
        
//        for (int i = (int)tests.size() - 1; i >= 0; i--) {
//            cout << "Add text : " << tests[i] << endl;
//            suffixTree.addText(tests.at(i), locations.at(i));
//            suffixTree.print();
//        }
//        suffixTree.getEquivalenceClass();
//        
//        
//        suffixTree.printBidirectional();
    }
    
    /**
     * 对等价类进行去重和合并
     * */
    void equiMerge() {
        vector<set<string>> temp = vector<set<string>>();
        for (set<string> s: equiSet) {
            temp.push_back(s);
        }
        equiSet = set<set<string>>();
        bool* marked = new bool[(int)temp.size()];
        for (int i = 0; i < (int)temp.size(); i++) {
            marked[i] = false;
        }
        
        for (int i = 0; i < temp.size(); i++) {
            if (marked[i]) {
                continue;
            }
            marked[i] = true;
            set<string> cur = temp.at(i);
            for (int j = i + 1; j < temp.size(); j++) {
                if (marked[j]) {
                    continue;
                }
                bool needMerge = false;
                for (string str : temp.at(j)) {
                    if (cur.find(str) != cur.end()) {
                        needMerge = true;
                    }
                }
                if (!needMerge) {
                    continue;
                }
                marked[j] = true;
                for (string str: temp.at(j)) {
                    cur.insert(str);
                }
            }
            equiSet.insert(cur);
        }
    }
    
};



int main(int argc, const char * argv[]) {
    // insert code here...
    SuffixTree s = SuffixTree();
   
    vector<string> texts = {"CompoundStmt", "CompoundStmtDeclStmtDeclStmtReturnStmtIntegerLiteralIntegerLiteralBinaryOperatorImplicitCastExprImplicitCastExprImplicitCastExprDeclRefExprDeclRefExprDeclRefExpr", "CompoundStmtDeclStmtDeclStmtReturnStmtIntegerLiteralIntegerLiteralBinaryOperatorImplicitCastExprImplicitCastExprImplicitCastExprDeclRefExprDeclRefExprDeclRefExpr",     "aaa",     "aaa",     "aaabaaa",  "aasasasaa",   "aaaabbbbaaaabbbbbbbb"};
    vector<string> locations = {"method0", "method1",   "method2", "method3", "method4", "method5",  "method6",     "method7"};
    CloneDetector detector = CloneDetector();
    detector.detect(texts, locations);
    return 0;
}
