//
//  main.cpp
//  model2_viterbi
//
//  Created by 王青龙 on 5/23/16.
//  Copyright © 2016 王青龙. All rights reserved.
//

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

void read_index_word(string str,map<string, int>& word2index){//读词和编号，这个的作用是可以把原始的句对转化成index
    ifstream fin(str);
    string line;
    string word;
    int index;

    while (getline(fin,line)) {
        istringstream linestream(line);
        linestream>>index;
        linestream>>word;
        word2index.insert({word,index});
    }
    cout<<word2index.size()<<endl;
}

void read_corpus(string str, string str2,map<string, int>& word2index){
    ifstream fin(str);
    ofstream fout(str2);
    string line;
    string word;
    int index;
    bool firstword=true;

    while(getline(fin, line)){
        firstword=true;
        istringstream linestream(line);
        while(linestream>>word){
            if(firstword){
                firstword=false;
            }else{
                fout<<" ";
            }
            if(word2index.find(word)==word2index.end()){
                fout<<-1;//对于这些未登录词可以暂时用-1表示，设置固定的概率
            }else{
                index=word2index[word];
                fout<<index;
            }
        }
        fout<<endl;
    }
}

void read_t(string str, map<int, map<int, double>>& t){
    ifstream fin(str);
    string line;
    int index_ch;
    int index_en;
    double prop;
    while (getline(fin, line)) {
        istringstream linestream(line);
        linestream>>index_ch;
        if(t.find(index_ch)==t.end()){
            t[index_ch]=map<int, double>();
        }
        linestream>>index_en;
        linestream>>prop;
        t[index_ch].insert({index_en, prop});
    }
    cout<<t.size()<<endl;//包含了一个空
}

void read_a(string str, map<int, map<int, map<int, double>>>& a){//位置对齐概率
    ifstream fin(str);
    string line;
    int len_ch;
    int index_ch;
    int index_en;
    double prop;
    while (getline(fin, line)) {
        istringstream linestream(line);
        linestream>>index_ch;//源语言位置
        linestream>>index_en;//目标语言位置
        linestream>>len_ch;//源语言长度
        linestream>>prop;//跳过这个
        linestream>>prop;//对齐概率
        if(a.find(len_ch)==a.end()){
            a[len_ch]=map<int, map<int, double>>();
        }
        if(a[len_ch].find(index_ch)==a[len_ch].end()){
            a[len_ch][index_ch]=map<int, double>();
        }
        a[len_ch][index_ch].insert({index_en, prop});
    }

    cout<<a.size()<<endl;
}

void read_a_g(string str, map<int, map<int, map<int, double>>>& a){//用Giza的格式
    ifstream fin(str);
    string line;
    int len_ch;
    int index_ch;
    int index_en;
    int index_en_my;
    double prop;
    while (getline(fin, line)) {
        istringstream linestream(line);
        linestream>>index_ch;//源语言位置
        linestream>>index_en;//目标语言位置
        index_en_my=index_en-1;
        linestream>>len_ch;//源语言长度
        linestream>>prop;//跳过这个
        linestream>>prop;//对齐概率
        if (a.find(len_ch)==a.end()) {
            a[len_ch]=map<int, map<int, double>>();
        }
        if(a[len_ch].find(index_en_my)==a[len_ch].end()){
            a[len_ch][index_en_my]=map<int, double>();
        }
        a[len_ch][index_en_my].insert({index_ch, prop});
    }
}

void read_index(string str, vector<vector<int>> &vv){//存储index表示的文件
    ifstream fin(str);
    string line;
    int index;
    int i=0;

    while (getline(fin, line)) {
        istringstream linestream(line);
        vv.push_back(vector<int>());
        while(linestream>>index){
            vv[i].push_back(index);
        }
        ++i;
    }
    cout<<vv.size()<<endl;
}


void model2_align(string str, vector<vector<int>>& vv_ch, vector<vector<int>>& vv_en, map<int,map<int,map<int,double>>>& prop_a, map<int, map<int, double>>& prop_t){

    ofstream fout(str);
    bool firstword=true;
    double tmp_max=0.0;
    int tmp_j=0;

    for (int l=0; l<vv_ch.size(); ++l) {
        firstword=true;
        for (int i=0; i<vv_ch[l].size(); ++i) {
            tmp_max=0.0;
            tmp_j=0;
            for (int j=0; j<vv_en[l].size(); ++j) {//在这里a的位置选择从1开始，prop_a的第一维长度表示源语言长度
                //                if (vv_ch[l][i]!=-1&&vv_en[l][j]!=-1) {//貌似在这里，即使两个词都曾经出现过，也有可能在概率表中没有出现过，这是个问题
                if (prop_t.find(vv_ch[l][i])!=prop_t.end()&&prop_t[vv_ch[l][i]].find(vv_en[l][j])!=prop_t[vv_ch[l][i]].end()&&prop_a.find(vv_ch[l].size()+4)!=prop_a.end()&&prop_a[vv_ch[l].size()+4].find(i+1)!=prop_a[vv_ch[l].size()+4].end()&&prop_a[vv_ch[l].size()+4][i+1].find(j+1)!=prop_a[vv_ch[l].size()+4][i+1].end()) {//既保证了在概率表中出现过，又保证了在对齐表中出现过
                    if (tmp_max<prop_t[vv_ch[l][i]][vv_en[l][j]]*prop_a[vv_ch[l].size()+4][i+1][j+1]) {
                        tmp_max=prop_t[vv_ch[l][i]][vv_en[l][j]]*prop_a[vv_ch[l].size()+4][i+1][j+1];
                        tmp_j=j;
                    }
                }else{
                    //这里是在概率表和对齐表中可能没有出现过的情况
                    if (prop_t.find(vv_ch[l][i])!=prop_t.end()&&prop_t[vv_ch[l][i]].find(vv_en[l][j])!=prop_t[vv_ch[l][i]].end()) {
                        if (tmp_max<prop_t[vv_ch[l][i]][vv_en[l][j]]*0.0000001) {
                            tmp_max=prop_t[vv_ch[l][i]][vv_en[l][j]]*0.0000001;
                            tmp_j=j;
                        }
                    }
                    if (prop_a.find(vv_ch[l].size()+4)!=prop_a.end()&&prop_a[vv_ch[l].size()+4].find(i+1)!=prop_a[vv_ch[l].size()+4].end()&&prop_a[vv_ch[l].size()+4][i+1].find(j+1)!=prop_a[vv_ch[l].size()+4][i+1].end()) {
                        if (tmp_max<0.0000001*prop_a[vv_ch[l].size()+4][i+1][j+1]) {
                            tmp_max=0.0000001*prop_a[vv_ch[l].size()+4][i+1][j+1];
                            tmp_j=j;
                        }
                    }
                }

            }
            //在这里已经选定了最大的，并且是用tmp_max存储的
            if (prop_t[0].find(tmp_j)!=prop_t[0].end()&&prop_a[vv_ch[l].size()+4][0].find(tmp_j)!=prop_a[vv_ch[l].size()+4][0].end()) {//选定的目标语言在概率表中有对空的概率
                if (tmp_max<prop_t[0][vv_en[l][tmp_j]]*prop_a[vv_ch[l].size()+4][0][tmp_j]) {


                }else{
                    if (firstword) {
                        firstword=false;
                    }else{
                        fout<<" ";
                    }
                    fout<<i<<"-"<<tmp_j;
                }
            }else{//选定的目标语言在概率表中没有对空的概率,这里或许不需要再一次进行分类了，可以都输出
                if (tmp_max<0.004*prop_a[vv_ch[l].size()+4][0][tmp_j]) {//
                    //选定的目标语言对空的概率更大，不输出或者找概率第二大的
                }else{
                    if (firstword) {
                        firstword=false;
                    }else{
                        fout<<" ";
                    }
                    fout<<i<<"-"<<tmp_j;
                }
            }
        }
        fout<<endl;
    }
}

void model2_g_align(string str, vector<vector<int>>& vv_ch, vector<vector<int>>& vv_en, map<int,map<int,map<int,double>>>& prop_a, map<int, map<int, double>>& prop_t){
    ofstream fout(str);
    bool firstword=true;
    double tmp_max=0.0;
    int tmp_i=0;
    auto lc=vv_ch[0].size();

    for (int l=0; l<vv_ch.size(); ++l) {
        firstword=true;
        lc=vv_ch[l].size()-1;//每一行对应a的第一维长度
        vector<int> viterbi_alignment(vv_en[l].size());//用来存储每个目标语言对应的源语言序号
        for (int j=0; j<vv_en[l].size(); ++j) {
            tmp_max=0.0;
            tmp_i=0;
            for (int i=0; i<vv_ch[l].size(); ++i) {
                if (tmp_max<prop_t[vv_ch[l][i]][vv_en[l][j]]*prop_a[lc][j][i]) {
                    tmp_max=prop_t[vv_ch[l][i]][vv_en[l][j]]*prop_a[lc][j][i];
                    tmp_i=i;
                }
            }
            viterbi_alignment[j]=tmp_i;
        }

        for (int j=0; j!=vv_en[l].size(); ++j) {
            if (viterbi_alignment[j]) {
                if (firstword) {
                    firstword=false;
                }else{
                    fout<<" ";
                }
                fout<<viterbi_alignment[j]-1<<"-"<<j;
            }
        }
        fout<<endl;
    }
    fout.close();
}

int main(int argc, const char * argv[]) {
    // insert code here...
    string dir_name = "/Users/wangql/Desktop/model2_viterbi";

    map<string, int> word2index_ch, word2index_en;//存储词和编号
    read_index_word(dir_name + "/corpus.ch.vcb", word2index_ch);
    read_index_word(dir_name + "/corpus.en.vcb", word2index_en);

    map<int, map<int, double>> t;//词翻译概率也读进来了
    read_t(dir_name + "/model2_4.t", t);

    map<int, map<int, map<int, double>>> a;//位置对齐概率
    //    read_a(dir_name + "/s2t64.ahmm.4", a);
    read_a_g(dir_name + "/model2_4.a", a);

    read_corpus(dir_name + "/corpus.ch", dir_name + "/corpus.ch.index", word2index_ch);//输出中文index
    read_corpus(dir_name + "/corpus.en", dir_name + "/corpus.en.index", word2index_en);//输出英文index

    vector<vector<int>> vv_ch, vv_en;
    read_index(dir_name + "/corpus.ch.index", vv_ch);
    read_index(dir_name + "/corpus.en.index", vv_en);


    //    model2_align(dir_name + "/model2.align", vv_ch, vv_en, a, t);//model2_align
    model2_g_align(dir_name + "/model2_g.align", vv_ch, vv_en, a, t);

    return 0;
}
